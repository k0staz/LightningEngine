#include "RenderResourceManager/RenderResourceManager.h"

#include <tracy/Tracy.hpp>

#include "RenderCore.h"
#include "RenderDefines.h"
#include "RHIDevice.h"
#include "Multithreading/JobScheduler.h"
#include "Templates/Alignment.h"

namespace LE::Renderer
{
void RenderResourceManager::Initialize()
{
    // Initialize Global Mesh Buffer
    RHI::RHIDevice* device = RHI::RHIDevice::Get();
    RHI::RHIBufferDescription globalMeshBufferDescription;
    globalMeshBufferDescription.UsageType = RHI::RHIBufferUsageType::MeshGlobal;
    globalMeshBufferDescription.Size = GlobalMeshGlobalBufferSize;
    GlobalMeshBuffer = device->CreateBuffer(globalMeshBufferDescription);

    // Initialize Channel
    StaticMeshChannels.IndexChannel = GlobalMeshBuffer->GetCreateBufferChannel(RHI::RHIGlobalBufferChannelType::Index,
                                                                               GlobalMeshIndicesSize);
    StaticMeshChannels.PositionChannel = GlobalMeshBuffer->GetCreateBufferChannel(RHI::RHIGlobalBufferChannelType::Position,
                                                                                  GlobalMeshPositionsSize);
    StaticMeshChannels.NormalsChannel = GlobalMeshBuffer->GetCreateBufferChannel(RHI::RHIGlobalBufferChannelType::Normal,
                                                                                 GlobalMeshNormalsSize);
    StaticMeshChannels.TexCoordChannel = GlobalMeshBuffer->GetCreateBufferChannel(RHI::RHIGlobalBufferChannelType::TexCoord,
                                                                                  GlobalMeshTexCoordsSize);
}

void RenderResourceManager::Shutdown()
{
    RHI::RHIDevice* device = RHI::RHIDevice::Get();

    for (auto& it : StaticMeshStorage.GetAllRenderResourceStates())
    {
        ReleaseStaticMesh(it.ResourceId);
    }

    GlobalMeshBuffer->RemoveBufferChannel(RHI::RHIGlobalBufferChannelType::Index);
    GlobalMeshBuffer->RemoveBufferChannel(RHI::RHIGlobalBufferChannelType::Position);
    GlobalMeshBuffer->RemoveBufferChannel(RHI::RHIGlobalBufferChannelType::Normal);
    GlobalMeshBuffer->RemoveBufferChannel(RHI::RHIGlobalBufferChannelType::TexCoord);

    device->DestroyBuffer(GlobalMeshBuffer);

    for (auto& i : FrameTransferContexts)
    {
        for (auto& [StageBuffer] : i)
        {
            if (StageBuffer)
            {
                device->DestroyBuffer(StageBuffer);
                StageBuffer = nullptr;
            }
        }
    }

    StaticMeshStorage.Clear();
}

void RenderResourceManager::OnBeginFrame()
{
    if (!Thread::IsRenderThread())
    {
        LE_ASSERT_DESC(false, "On Begin Frame should be called from render thread")
        return;
    }

    RHI::RHIDevice* device = RHI::RHIDevice::Get();
    const uint64 frameIdx = Renderer::GetCurrentRenderFrame() % DEFAULT_FRAMES_IN_FLIGHT;
    for (uint32 i = 0; i < DEFAULT_TASK_WORKER_THREADS; ++i)
    {
        RefCountingPtr<RHI::RHILinearBuffer>& stageBuffer = FrameTransferContexts[frameIdx][i].StageBuffer;
        if (!stageBuffer)
        {
            continue;
        }

        device->DestroyBuffer(stageBuffer);
        stageBuffer = nullptr;
    }
}

void RenderResourceManager::DispatchBatchLoadTasks()
{
    std::vector<PendingStaticMeshLoad> thisFrameBatch;
    {
        std::unique_lock lock(StaticMeshLoadsMutex);
        thisFrameBatch.swap(PendingStaticMeshLoads);
    }

    if (thisFrameBatch.empty())
    {
        ThisFrameBatchContext.WasStarted = false;
        return;
    }

    uint64 transferValue = RHI::RHIDevice::Get()->GetCurrentTransferTimelineValue();
    for (auto& pendingLoad : thisFrameBatch)
    {
        pendingLoad.ResourceHandle.Get().SetBatchTransferValue(transferValue);
    }

    const uint8 taskCount = thisFrameBatch.size() > 1 ? 2 : 1;
    std::vector<RefCountingPtr<AsyncTaskNodeBase>> thisFrameTasks;
    thisFrameTasks.resize(taskCount);

    JobScheduler& jobScheduler = GetServiceRegistry().GetService<JobScheduler>();

    const uint32 totalRequests = static_cast<uint32>(thisFrameBatch.size());
    const uint32 requestsPerTask = totalRequests / taskCount;
    uint32 remainder = totalRequests % taskCount;

    auto prevEnd = thisFrameBatch.begin();
    for (auto& task : thisFrameTasks)
    {
        std::vector<PendingStaticMeshLoad> taskPayload;

        uint32 currentChunkSize = requestsPerTask + (remainder > 0 ? 1 : 0);
        if (remainder > 0) { remainder--; }

        auto thisTaskEnd = prevEnd + currentChunkSize;
        if (thisTaskEnd > thisFrameBatch.end())
        {
            thisTaskEnd = thisFrameBatch.end();
        }

        taskPayload.insert(taskPayload.end(), prevEnd, thisTaskEnd);
        prevEnd = thisTaskEnd;

        task = MultithreadingUtils::MakeTask(
            "RecordTransferCommandsTask",
            &jobScheduler,
            &RenderResourceManager::RecordTransferCommandsTask,
            this,
            std::move(taskPayload));
    }

    ThisFrameBatchContext.RemainingTaskCount = taskCount;
    ThisFrameBatchContext.WasStarted = true;

    const uint64 frameIdx = Renderer::GetCurrentRenderFrame() % DEFAULT_FRAMES_IN_FLIGHT;
    LE_INFO("Dispatched batch commands. FrameIdx: {}", frameIdx);

    for (auto& task : thisFrameTasks)
    {
        task->Finalize();
    }
}

void RenderResourceManager::FinalizeBatchLoad()
{
    if (!ThisFrameBatchContext.WasStarted)
    {
        return;
    }

    ThisFrameBatchContext.CompletionSignal.acquire();
    const uint64 frameIdx = Renderer::GetCurrentRenderFrame() % DEFAULT_FRAMES_IN_FLIGHT;
    LE_INFO("FinalizeBatchLoad. FrameIdx: {}", frameIdx);
    RHI::RHIDevice::Get()->SubmitCommandList(RHI::RHICommandListType::Transfer, ThisFrameBatchContext.FinalizedList);
    ThisFrameBatchContext.WasStarted = false;
    ThisFrameBatchContext.FinalizedList.clear();
}

RenderResourceHandle<const StaticMeshRenderResource> RenderResourceManager::RequestStaticMesh(AssetHandle<StaticMeshAsset> Asset)
{
    if (!Asset.IsValid())
    {
        return {};
    }

    const StaticMeshRenderResourceStorage::ResourceState resourceState = StaticMeshStorage.GetCreateRenderResourceState(
        Asset->GetStableId());
    StaticMeshRenderResource& resource = StaticMeshStorage.GetRenderResource(resourceState.ResourceId);
    if (resource.GetState() != RenderResourceState::Unloaded)
    {
        return resource;
    }

    if (!resource.TrySetLoadingState())
    {
        return resource;
    }

    // Schedule loading, at the end of the frame batch request load will be dispatched
    {
        std::unique_lock lock(StaticMeshLoadsMutex);
        PendingStaticMeshLoads.emplace_back(std::move(resource), Asset);
    }

    return resource;
}

RenderContributorId RenderResourceManager::GetRenderContributor(
    RenderResourceHandle<const StaticMeshRenderResource> StaticMeshResource) const
{
    auto state = StaticMeshStorage.GetRenderResourceState(StaticMeshResource.GetResourceId());
    return state.ContributorInstanceId;
}

RenderResource& RenderResourceManager::GetRenderResource(RenderResourceId ResourceId) const
{
    return StaticMeshStorage.GetRenderResource(ResourceId);
}

bool RenderResourceManager::HasResource(RenderResourceId ResourceId) const
{
    return StaticMeshStorage.Has(ResourceId);
}

void RenderResourceManager::ReleaseStaticMesh(RenderResourceHandle<const StaticMeshRenderResource> StaticMeshResource)
{
    ReleaseStaticMesh(StaticMeshResource.GetResourceId());
}

void RenderResourceManager::ReleaseStaticMesh(RenderResourceId ResourceId)
{
    {
        StaticMeshRenderResource& staticMeshRenderResource = StaticMeshStorage.GetRenderResource(ResourceId);
        StaticMeshChannels.IndexChannel->FreeSubAllocation(staticMeshRenderResource.IndexBuffer);
        StaticMeshChannels.PositionChannel->FreeSubAllocation(staticMeshRenderResource.PositionBuffer);
        StaticMeshChannels.NormalsChannel->FreeSubAllocation(staticMeshRenderResource.NormalBuffer);
        StaticMeshChannels.TexCoordChannel->FreeSubAllocation(staticMeshRenderResource.TexCoordsBuffer);
    }


    StaticMeshStorage.ReleaseRenderResource(ResourceId);
}

void RenderResourceManager::RecordTransferCommandsTask(std::vector<PendingStaticMeshLoad> PendingStaticMeshLoad)
{
    ZoneScopedNC("RenderResourceManager::RecordTransferCommandsTask", tracy::Color::Purple);

    RHI::RHIDevice* device = RHI::RHIDevice::Get();
    RefCountingPtr<RHI::RHICommandList> commandList = device->CreateCommandList(RHI::RHICommandListType::Transfer);

    // Prepare CPU data for transfer
    uint64 totalIndices = 0;
    uint64 totalPositions = 0;
    uint64 totalNormals = 0;
    uint64 totalUVs = 0;

    for (const auto& pendingLoad : PendingStaticMeshLoad)
    {
        const auto& asset = pendingLoad.Asset.GetAssetRef();
        totalIndices += asset.Indices.size();
        totalPositions += asset.Positions.size();
        totalNormals += asset.Normals.size();
        totalUVs += asset.UVs.size();
    }

    std::vector<uint32> batchIndices;
    std::vector<Vector3F> batchPositions;
    std::vector<Vector3F> batchNormals;
    std::vector<Vector2F> batchTexCoords;

    batchIndices.reserve(totalIndices);
    batchPositions.reserve(totalPositions);
    batchNormals.reserve(totalNormals);
    batchTexCoords.reserve(totalUVs);

    struct MeshStagingInfo
    {
        uint64 IndicesOffset;
        uint64 PositionsOffset;
        uint64 NormalsOffset;
        uint64 TexCoordsOffset;
    };

    const uint64 alignment = RHI::GlobalStorageAlignment;
    uint64 currentIndicesStageOffset = 0;
    uint64 currentPositionsStageOffset = 0;
    uint64 currentNormalsStageOffset = 0;
    uint64 currentTexCoordsStageOffset = 0;

    std::vector<MeshStagingInfo> stagingInfo;
    stagingInfo.reserve(PendingStaticMeshLoad.size());
    for (const auto& pendingLoad : PendingStaticMeshLoad)
    {
        const StaticMeshAsset& staticMeshAsset = pendingLoad.Asset.GetAssetRef();

        batchIndices.insert(batchIndices.end(), staticMeshAsset.Indices.begin(), staticMeshAsset.Indices.end());
        batchPositions.insert(batchPositions.end(), staticMeshAsset.Positions.begin(), staticMeshAsset.Positions.end());
        batchNormals.insert(batchNormals.end(), staticMeshAsset.Normals.begin(), staticMeshAsset.Normals.end());
        batchTexCoords.insert(batchTexCoords.end(), staticMeshAsset.UVs.begin(), staticMeshAsset.UVs.end());

        const uint64 indicesSize = staticMeshAsset.Indices.size() * sizeof(uint32);
        const uint64 positionsSize = staticMeshAsset.Positions.size() * sizeof(Vector3F);
        const uint64 normalsSize = staticMeshAsset.Normals.size() * sizeof(Vector3F);
        const uint64 texCoordsSize = staticMeshAsset.UVs.size() * sizeof(Vector2F);

        StaticMeshRenderResource& resource = pendingLoad.ResourceHandle.Get();
        resource.IndexCount = staticMeshAsset.Indices.size();
        resource.IndexBuffer = StaticMeshChannels.IndexChannel->CreateSubAllocation(indicesSize);
        resource.PositionBuffer = StaticMeshChannels.PositionChannel->CreateSubAllocation(positionsSize);
        resource.NormalBuffer = StaticMeshChannels.NormalsChannel->CreateSubAllocation(normalsSize);
        resource.TexCoordsBuffer = StaticMeshChannels.TexCoordChannel->CreateSubAllocation(texCoordsSize);

        MeshStagingInfo info{};
        info.IndicesOffset = currentIndicesStageOffset;
        info.PositionsOffset = currentPositionsStageOffset;
        info.NormalsOffset = currentNormalsStageOffset;
        info.TexCoordsOffset = currentTexCoordsStageOffset;
        stagingInfo.push_back(info);

        currentIndicesStageOffset += indicesSize;
        currentPositionsStageOffset += positionsSize;
        currentNormalsStageOffset += normalsSize;
        currentTexCoordsStageOffset += texCoordsSize;
    }

    const uint64 indicesBaseOffset = 0;
    const uint64 positionsBaseOffset = indicesBaseOffset + currentIndicesStageOffset;
    const uint64 normalsBaseOffset = positionsBaseOffset + currentPositionsStageOffset;
    const uint64 textureCoordsBaseOffset = normalsBaseOffset + currentNormalsStageOffset;

    RHI::RHIBufferDescription stageBufferDescription;
    stageBufferDescription.UsageType = RHI::RHIBufferUsageType::UploadStaging;
    stageBufferDescription.Size = textureCoordsBaseOffset + currentTexCoordsStageOffset;
    RefCountingPtr<RHI::RHILinearBuffer> stageBuffer = device->CreateBuffer(stageBufferDescription);

    // Save stage buffer, so we could release it once transfer is finished
    FrameTransferContexts[GetCurrentRenderFrame() % DEFAULT_FRAMES_IN_FLIGHT][Thread::GetWorkerTaskThreadIndex()].StageBuffer = stageBuffer;

    // Write to stage buffer
    stageBuffer->Write(batchIndices.data(), currentIndicesStageOffset);
    stageBuffer->Write(batchPositions.data(), currentPositionsStageOffset);
    stageBuffer->Write(batchNormals.data(), currentNormalsStageOffset);
    stageBuffer->Write(batchTexCoords.data(), currentTexCoordsStageOffset);

    // Record commands
    std::vector<RHI::RHIGlobalBufferUploadDesc> uploadDescriptions;
    uploadDescriptions.reserve(PendingStaticMeshLoad.size() * 4);
    for (uint32 meshIdx = 0; meshIdx < PendingStaticMeshLoad.size(); ++meshIdx)
    {
        StaticMeshRenderResource& resource = PendingStaticMeshLoad[meshIdx].ResourceHandle.Get();
        const MeshStagingInfo& meshStagingInfo = stagingInfo[meshIdx];

        // Indices
        {
            RHI::RHIGlobalBufferUploadDesc& uploadDesc = uploadDescriptions.emplace_back();

            uploadDesc.Size = resource.IndexBuffer->GetSize();
            uploadDesc.GlobalBufferOffset = resource.IndexBuffer->GetOffset();
            uploadDesc.StageBufferOffset = indicesBaseOffset + meshStagingInfo.IndicesOffset;
        }

        // Positions
        {
            RHI::RHIGlobalBufferUploadDesc& uploadDesc = uploadDescriptions.emplace_back();

            uploadDesc.Size = resource.PositionBuffer->GetSize();
            uploadDesc.GlobalBufferOffset = resource.PositionBuffer->GetOffset();
            uploadDesc.StageBufferOffset = positionsBaseOffset + meshStagingInfo.PositionsOffset;
        }

        // Normals
        {
            RHI::RHIGlobalBufferUploadDesc& uploadDesc = uploadDescriptions.emplace_back();

            uploadDesc.Size = resource.NormalBuffer->GetSize();
            uploadDesc.GlobalBufferOffset = resource.NormalBuffer->GetOffset();
            uploadDesc.StageBufferOffset = normalsBaseOffset + meshStagingInfo.NormalsOffset;
        }

        // Texture Coordinates
        {
            RHI::RHIGlobalBufferUploadDesc& uploadDesc = uploadDescriptions.emplace_back();

            uploadDesc.Size = resource.TexCoordsBuffer->GetSize();
            uploadDesc.GlobalBufferOffset = resource.TexCoordsBuffer->GetOffset();
            uploadDesc.StageBufferOffset = textureCoordsBaseOffset + meshStagingInfo.TexCoordsOffset;
        }
    }

    commandList->BeginRecording();
    device->CopyToGlobalBuffer(commandList, GlobalMeshBuffer, stageBuffer, uploadDescriptions);
    commandList->EndRecording();

    bool isLastTask = false;
    {
        std::unique_lock lock(ThisFrameBatchContext.ContextMutex);
        ThisFrameBatchContext.FinalizedList.emplace_back(std::move(commandList));
        --ThisFrameBatchContext.RemainingTaskCount;
        isLastTask = ThisFrameBatchContext.RemainingTaskCount == 0;
    }

    if (isLastTask)
    {
        ThisFrameBatchContext.CompletionSignal.release();
    }
}
}
