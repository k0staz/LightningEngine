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

    for (uint32 i = 0; i < static_cast<uint32>(RHI::RHISamplerType::Count); ++i)
    {
        if (GlobalSamplers[i])
        {
            LE_ASSERT_DESC(false, "Global sampler is not null")
        }

        GlobalSamplers[i] = device->CreateSampler(static_cast<RHI::RHISamplerType>(i));
    }

    RHI::RHIDescriptorSetLayoutDesc globalDescLayout;
    globalDescLayout.Bindings.reserve(2);
    RHI::RHIDescriptorSetLayoutBindingDesc& globalTextureBinding = globalDescLayout.Bindings.emplace_back();
    globalTextureBinding.Binding = RHI::GLOBAL_TEXTURES_BINDING;
    globalTextureBinding.DescriptorType = RHI::RHIDescriptorType::SampledImage;
    globalTextureBinding.DescriptorCount = RHI::TEXTURE_SLOT_COUNT;
    globalTextureBinding.ShaderStage = RHI::RHIShaderStage::Fragment;

    RHI::RHIDescriptorSetLayoutBindingDesc& globalSamplerBinding = globalDescLayout.Bindings.emplace_back();
    globalSamplerBinding.Binding = RHI::GLOBAL_SAMPLERS_BINDING;
    globalSamplerBinding.DescriptorType = RHI::RHIDescriptorType::Sampler;
    globalSamplerBinding.DescriptorCount = static_cast<uint32>(RHI::RHISamplerType::Count);
    globalSamplerBinding.ShaderStage = RHI::RHIShaderStage::Fragment;
    globalSamplerBinding.ImmutableSamplers = {GlobalSamplers.begin(), GlobalSamplers.end()};

    globalDescLayout.BindingFlags.resize(2);
    globalDescLayout.BindingFlags[0] = RHI::RHIDescriptorBindingFlags::PartiallyBound | RHI::RHIDescriptorBindingFlags::UpdateAfterBind;
    globalDescLayout.BindingFlags[1] = RHI::RHIDescriptorBindingFlags::None;

    globalDescLayout.Flags = RHI::RHIDescriptorSetLayoutCreateFlags::UpdateAfterBindPool;

    GlobalDescriptorSetLayout = device->CreateDescriptorSetLayout(globalDescLayout);

    RHI::RHIDescriptorSetPoolDesc globalDescPoolDesc;
    globalDescPoolDesc.Flags = RHI::RHIPoolCreateFlags::UpdateAfterBind;
    globalDescPoolDesc.MaxSets = 1;
    globalDescPoolDesc.PoolSizes.reserve(2);
    auto& texturePoolSize = globalDescPoolDesc.PoolSizes.emplace_back();
    texturePoolSize.DescriptorType = RHI::RHIDescriptorType::SampledImage;
    texturePoolSize.DescriptorCount = RHI::TEXTURE_SLOT_COUNT;
    auto& samplerPoolSize = globalDescPoolDesc.PoolSizes.emplace_back();
    samplerPoolSize.DescriptorType = RHI::RHIDescriptorType::Sampler;
    samplerPoolSize.DescriptorCount = static_cast<uint32>(RHI::RHISamplerType::Count);

    GlobalDescriptorSetPool = device->CreateDescriptorSetPool(globalDescPoolDesc);

    RHI::RHIDescriptorSetDesc globalTexDesc;
    globalTexDesc.Pool = GlobalDescriptorSetPool;
    globalTexDesc.Layout = GlobalDescriptorSetLayout;

    GlobalDescriptorSet = device->CreateDescriptorSet(globalTexDesc);
}

void RenderResourceManager::Shutdown()
{
    RHI::RHIDevice* device = RHI::RHIDevice::Get();

    for (auto& it : ResourceStorages)
    {
        for (auto& resourceStateIt : it.second->GetAllRenderResourceStates())
        {
            ReleaseResource(it.first, resourceStateIt.ResourceId);
        }
    }

    GlobalMeshBuffer->RemoveBufferChannel(RHI::RHIGlobalBufferChannelType::Index);
    GlobalMeshBuffer->RemoveBufferChannel(RHI::RHIGlobalBufferChannelType::Position);
    GlobalMeshBuffer->RemoveBufferChannel(RHI::RHIGlobalBufferChannelType::Normal);
    GlobalMeshBuffer->RemoveBufferChannel(RHI::RHIGlobalBufferChannelType::TexCoord);

    device->DestroyBuffer(GlobalMeshBuffer);

    device->DestroyDescriptorSetLayout(GlobalDescriptorSetLayout);
    device->DestroyDescriptorSetPool(GlobalDescriptorSetPool);

    for (auto& sampler : GlobalSamplers)
    {
        if (sampler)
        {
            device->DestroySampler(sampler);
            sampler = nullptr;
        }
    }

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

    ResourceStorages.clear();
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

    OnBeginFrameProcessPendingDelete();
}

void RenderResourceManager::EnqueuePendingBarriers(RefCountingPtr<RHI::RHICommandList> CommandList)
{
    const uint64 currentTransferValue = RHI::RHIDevice::Get()->GetCurrentTransferTimelineValue();
    RHI::RHIDependencyDesc dependencyDesc;
    bool hasBarriers = false;
    {
        std::lock_guard barrierLock(PendingBarriersMutex);
        if (PendingBarriers.empty())
        {
            return;
        }

        while (!PendingBarriers.empty() && PendingBarriers.top().TimelineValue < currentTransferValue)
        {
            dependencyDesc.ImageMemoryBarriers.insert(dependencyDesc.ImageMemoryBarriers.end(), PendingBarriers.top().ImageBarriers.begin(),
                                                      PendingBarriers.top().ImageBarriers.end());
            PendingBarriers.pop();
            hasBarriers = true;
        }
    }

    if (hasBarriers)
    {
        CommandList->PipelineBarrier(dependencyDesc);
    }
}

void RenderResourceManager::DispatchBatchLoadTasks()
{
    std::vector<PendingStaticMeshLoad> thisFrameBatchStaticMesh;
    {
        std::lock_guard lock(StaticMeshLoadsMutex);
        thisFrameBatchStaticMesh.swap(PendingStaticMeshLoads);
    }

    std::vector<PendingTextureLoad> thisFrameBatchTexture;
    {
        std::lock_guard lock(TextureLoadsMutex);
        thisFrameBatchTexture.swap(PendingTextureLoads);
    }

    if (thisFrameBatchStaticMesh.empty() && thisFrameBatchTexture.empty())
    {
        ThisFrameBatchContext.WasStarted = false;
        return;
    }

    uint64 transferValue = RHI::RHIDevice::Get()->GetCurrentTransferTimelineValue();
    for (auto& pendingLoad : thisFrameBatchStaticMesh)
    {
        pendingLoad.ResourceHandle.Get().SetBatchTransferValue(transferValue);
    }

    for (auto& pendingLoad : thisFrameBatchTexture)
    {
        pendingLoad.ResourceHandle.Get().SetBatchTransferValue(transferValue);
    }

    const uint8 taskMeshCount = thisFrameBatchStaticMesh.size() > 1 ? 2 : 1;
    const uint8 taskTextureCount = thisFrameBatchTexture.size() > 1 ? 2 : 1;
    std::vector<RefCountingPtr<AsyncTaskNodeBase>> thisFrameTasks;
    thisFrameTasks.resize(taskMeshCount + taskTextureCount);

    JobScheduler& jobScheduler = GetServiceRegistry().GetService<JobScheduler>();

    const uint32 totalMeshRequests = static_cast<uint32>(thisFrameBatchStaticMesh.size());
    const uint32 totalTextureRequests = static_cast<uint32>(thisFrameBatchTexture.size());
    const uint32 requestsMeshPerTask = totalMeshRequests / taskMeshCount;
    const uint32 requestsTexturePerTask = totalTextureRequests / taskTextureCount;

    uint32 processedRequests = 0;
    for (size_t i = 0; i < taskMeshCount; ++i)
    {
        std::vector<PendingStaticMeshLoad> taskPayload;

        auto copyBegin = thisFrameBatchStaticMesh.begin() + processedRequests;

        uint32 requestsNum = i == (taskMeshCount - 1) ? totalMeshRequests - processedRequests : requestsMeshPerTask;
        processedRequests += requestsNum;

        auto copyEnd = copyBegin + requestsNum;
        taskPayload.insert(taskPayload.end(), copyBegin, copyEnd);

        thisFrameTasks[i] = MultithreadingUtils::MakeTask(
            "RecordTransferCommandsTask",
            &jobScheduler,
            &RenderResourceManager::RecordTransferCommandsStaticMeshTask,
            this,
            std::move(taskPayload));
    }

    processedRequests = 0;
    for (size_t i = taskMeshCount; i < taskMeshCount + taskTextureCount; ++i)
    {
        std::vector<PendingTextureLoad> taskPayload;

        auto copyBegin = thisFrameBatchTexture.begin() + processedRequests;

        uint32 requestsNum = i == ((taskMeshCount + taskTextureCount) - 1)
                                 ? totalTextureRequests - processedRequests
                                 : requestsTexturePerTask;
        processedRequests += requestsNum;

        auto copyEnd = copyBegin + requestsNum;
        taskPayload.insert(taskPayload.end(), copyBegin, copyEnd);

        thisFrameTasks[i] = MultithreadingUtils::MakeTask(
            "RecordTransferCommandsTask",
            &jobScheduler,
            &RenderResourceManager::RecordTransferCommandsTextureTask,
            this,
            std::move(taskPayload));
    }

    ThisFrameBatchContext.RemainingTaskCount = taskMeshCount + taskTextureCount;
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

    // We must do it before the submit command list, because it increments transfer timeline value
    const uint64 transferValue = RHI::RHIDevice::Get()->GetCurrentTransferTimelineValue();
    {
        std::lock_guard barrierLock(PendingBarriersMutex);
        ThisFrameBatchContext.PendingBarriers.TimelineValue = transferValue;
        PendingBarriers.emplace(ThisFrameBatchContext.PendingBarriers);
    }


    ThisFrameBatchContext.CompletionSignal.acquire();
    const uint64 frameIdx = Renderer::GetCurrentRenderFrame() % DEFAULT_FRAMES_IN_FLIGHT;
    LE_INFO("FinalizeBatchLoad. FrameIdx: {}", frameIdx);
    RHI::RHIDevice::Get()->SubmitCommandList(RHI::RHICommandListType::Transfer, ThisFrameBatchContext.FinalizedList);
    ThisFrameBatchContext.WasStarted = false;
    ThisFrameBatchContext.FinalizedList.clear();

    ThisFrameBatchContext.PendingBarriers.ImageBarriers.clear();
    ThisFrameBatchContext.PendingBarriers.TimelineValue = 0;
}

RenderResourceHandle<const StaticMeshRenderResource> RenderResourceManager::RequestStaticMesh(AssetHandle<StaticMeshAsset> Asset)
{
    if (!Asset.IsValid())
    {
        return {};
    }

    auto& staticMeshStorage = GetCreateResourceStorage<StaticMeshRenderResource>();
    const auto resourceState = staticMeshStorage.GetCreateRenderResourceState(Asset->GetStableId());
    auto& resource = static_cast<StaticMeshRenderResource&>(staticMeshStorage.GetResource(resourceState.ResourceId));
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
        std::lock_guard lock(StaticMeshLoadsMutex);
        PendingStaticMeshLoads.emplace_back(std::move(resource), Asset);
    }

    return resource;
}

RenderResourceHandle<const TextureRenderResource> RenderResourceManager::RequestTexture(AssetHandle<TextureAsset> Asset)
{
    if (!Asset.IsValid())
    {
        return {};
    }

    auto& textureStorage = GetCreateResourceStorage<TextureRenderResource>();
    const auto resourceState = textureStorage.GetCreateRenderResourceState(Asset->GetStableId());
    auto& resource = static_cast<TextureRenderResource&>(textureStorage.GetResource(resourceState.ResourceId));
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
        std::lock_guard lock(TextureLoadsMutex);
        PendingTextureLoads.emplace_back(std::move(resource), Asset);
    }

    return resource;
}

bool RenderResourceManager::HasResource(RenderResourceTypeId TypeId, RenderResourceId ResourceId) const
{
    auto resourceStorage = GetResourceStorage(TypeId);
    if (resourceStorage)
    {
        return resourceStorage->Has(ResourceId);
    }

    return false;
}

RefCountingPtr<RHI::RHIDescriptorSetLayout> RenderResourceManager::GetGlobalDescriptorSetLayout() const
{
    return GlobalDescriptorSetLayout;
}

RefCountingPtr<RHI::RHIDescriptorSet> RenderResourceManager::GetGlobalDescriptorSet() const
{
    return GlobalDescriptorSet;
}

RenderResource& RenderResourceManager::GetRenderResource(RenderResourceTypeId TypeId, RenderResourceId ResourceId) const
{
    return GetResourceStorage(TypeId)->GetResource(ResourceId);
}

void RenderResourceManager::RecordPendingResourceDelete(RenderResourceTypeId TypeId, RenderResourceId ResourceId)
{
    const uint64 frameIdx = Renderer::GetCurrentRenderFrame() % DEFAULT_FRAMES_IN_FLIGHT;
    {
        std::lock_guard lock(PendingResourceDeletesMutex);
        PendingResourceDeletes[frameIdx].emplace_back(TypeId, ResourceId);
    }
}

void RenderResourceManager::ReleaseResource(RenderResourceTypeId TypeId, RenderResourceId ResourceId)
{
    if (TypeId == RenderResourceTypeIdGetter<StaticMeshRenderResource>::Value)
    {
        ReleaseStaticMesh(ResourceId);
    }
    else if (TypeId == RenderResourceTypeIdGetter<TextureRenderResource>::Value)
    {
        ReleaseTexture(ResourceId);
    }
    else
    {
        LE_ASSERT_DESC(false, "Unknown resource type");
    }
}

RenderResourceStorageBase* RenderResourceManager::GetResourceStorage(RenderResourceTypeId TypeId) const
{
    auto it = ResourceStorages.find(TypeId);
    if (it != ResourceStorages.end())
    {
        return it->second.get();
    }

    return nullptr;
}

void RenderResourceManager::ReleaseStaticMesh(RenderResourceId ResourceId)
{
    auto& staticMeshStorage = GetCreateResourceStorage<StaticMeshRenderResource>();

    {
        auto& staticMeshRenderResource = static_cast<StaticMeshRenderResource&>(staticMeshStorage.GetResource(ResourceId));

        StaticMeshChannels.IndexChannel->FreeSubAllocation(staticMeshRenderResource.IndexBuffer);
        StaticMeshChannels.PositionChannel->FreeSubAllocation(staticMeshRenderResource.PositionBuffer);
        StaticMeshChannels.NormalsChannel->FreeSubAllocation(staticMeshRenderResource.NormalBuffer);
        StaticMeshChannels.TexCoordChannel->FreeSubAllocation(staticMeshRenderResource.TexCoordsBuffer);
    }

    staticMeshStorage.ReleaseRenderResource(ResourceId);
}

void RenderResourceManager::ReleaseTexture(RenderResourceId ResourceId)
{
    auto& textureStorage = GetCreateResourceStorage<TextureRenderResource>();
    RHI::RHIDevice* device = RHI::RHIDevice::Get();
    {
        std::lock_guard lock(TextureBindingSlots.SlotMutex);
        auto& textureRenderResource = static_cast<TextureRenderResource&>(textureStorage.GetResource(ResourceId));
        TextureBindingSlots.FreeSlots.emplace(textureRenderResource.BindingSlot);
        device->DestroyImageView(textureRenderResource.TextureView);
        device->DestroyImage(textureRenderResource.Texture);
    }

    textureStorage.ReleaseRenderResource(ResourceId);
}

void RenderResourceManager::OnBeginFrameProcessPendingDelete()
{
    const uint64 frameIdx = Renderer::GetCurrentRenderFrame() % DEFAULT_FRAMES_IN_FLIGHT;
    std::vector<PendingResourceDelete> pendingDeletes;
    {
        std::lock_guard lock(PendingResourceDeletesMutex);
        pendingDeletes.swap(PendingResourceDeletes[frameIdx]);
    }

    if (pendingDeletes.empty())
    {
        return;
    }

    for (const auto& pendingDelete : pendingDeletes)
    {
        ReleaseResource(pendingDelete.TypeId, pendingDelete.ResourceId);
    }
}

uint64 RenderResourceManager::GetFreeTextureBindingSlot()
{
    std::lock_guard lock(TextureBindingSlots.SlotMutex);
    if (TextureBindingSlots.FreeSlots.empty())
    {
        return TextureBindingSlots.HeadSlotIndex++;
    }

    const uint64 freedSlot = TextureBindingSlots.FreeSlots.top();
    TextureBindingSlots.FreeSlots.pop();

    return freedSlot;
}

void RenderResourceManager::RecordTransferCommandsStaticMeshTask(std::vector<PendingStaticMeshLoad> PendingStaticMeshLoad)
{
    ZoneScopedNC("RenderResourceManager::RecordTransferCommandsTask StaticMesh", tracy::Color::Purple);

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
    commandList->CopyToGlobalBuffer(GlobalMeshBuffer, stageBuffer, uploadDescriptions);
    commandList->EndRecording();

    OnTransferTaskEnd(commandList);
}

void RenderResourceManager::RecordTransferCommandsTextureTask(std::vector<PendingTextureLoad> PendingTextureLoad)
{
    ZoneScopedNC("RenderResourceManager::RecordTransferCommandsTask Texture", tracy::Color::Purple);

    RHI::RHIDevice* device = RHI::RHIDevice::Get();
    RefCountingPtr<RHI::RHICommandList> commandList = device->CreateCommandList(RHI::RHICommandListType::Transfer);

    uint64 totalSize = 0;
    std::vector<RHI::RHIBufferImageCopyDesc> uploadDescriptions;
    std::vector<ktxTexture2*> ktxTextures;
    ktxTextures.reserve(PendingTextureLoad.size());
    uploadDescriptions.reserve(PendingTextureLoad.size());
    for (const auto& pendingTextureLoad : PendingTextureLoad)
    {
        auto& asset = pendingTextureLoad.Asset;
        auto& resource = pendingTextureLoad.ResourceHandle.Get();
        ktxTexture2* texture = asset->KtxTexture;
        if (!texture)
        {
            LE_ERROR("Invalid ktx texture");
            continue;
        }
        ktxTextures.emplace_back(texture);

        // Create Image
        RHI::RHIImageDesc imageDesc;
        imageDesc.Width = texture->baseWidth;
        imageDesc.Height = texture->baseHeight;
        imageDesc.Depth = 1;
        imageDesc.MipLevels = texture->numLevels;
        imageDesc.ArraySize = 1;
        imageDesc.Format = RHI::MapFromVkFormat(texture->vkFormat);
        imageDesc.Usage = RHI::RHIImageUsageFlag::TransferDst | RHI::RHIImageUsageFlag::Sampled;
        resource.Texture = device->CreateImage(imageDesc);

        RHI::RHIBufferImageCopyDesc& uploadDescription = uploadDescriptions.emplace_back();
        uploadDescription.Image = resource.Texture;
        uploadDescription.Regions.resize(imageDesc.MipLevels);
        for (auto i = 0; i < imageDesc.MipLevels; ++i)
        {
            RHI::RHIBufferImageCopyDesc::CopyRegion& region = uploadDescription.Regions[i];
            ktx_size_t mipOffset = 0;
            ktxTexture2_GetImageOffset(texture, i, 0, 0, &mipOffset);
            region.SourceBufferOffset = totalSize + mipOffset;
            region.SubresourceLayers.Aspect = RHI::RHIImageAspectFlags::Color;
            region.SubresourceLayers.MipLevel = i;
            region.SubresourceLayers.NumArraySlices = 1;
            region.Extent = {imageDesc.Width >> i, imageDesc.Height >> i, 1};
        }

        RHI::RHIImageViewDesc viewDesc;
        viewDesc.Image = resource.Texture;
        viewDesc.ViewType = RHI::RHIImageViewType::View2D;
        viewDesc.Format = imageDesc.Format;
        RHI::RHISubresourceRange& subRange = viewDesc.SubresourceRange;
        subRange.Aspect = RHI::RHIImageAspectFlags::Color;
        subRange.NumMipLevels = imageDesc.MipLevels;
        subRange.NumArraySlices = 1;
        resource.TextureView = device->CreateImageView(viewDesc);

        totalSize += static_cast<uint64>(asset->KtxTexture->dataSize);

        // Write to descriptor set
        RHI::RHIUpdateDescriptorSetDesc setDesc;
        RHI::RHIDescriptorImageInfoDesc& infoDesc = setDesc.ImageInfos.emplace_back();
        infoDesc.View = resource.TextureView;
        infoDesc.Layout = RHI::RHIImageLayout::ShaderReadOnly;

        setDesc.Set = GlobalDescriptorSet;
        setDesc.Binding = RHI::GLOBAL_TEXTURES_BINDING;
        setDesc.ArrayElement = GetFreeTextureBindingSlot();
        setDesc.DescriptorType = RHI::RHIDescriptorType::SampledImage;

        device->UpdateDescriptorSet(setDesc);
        resource.BindingSlot = setDesc.ArrayElement;
    }

    RHI::RHIBufferDescription stageBufferDescription;
    stageBufferDescription.UsageType = RHI::RHIBufferUsageType::UploadStaging;
    stageBufferDescription.Size = totalSize;
    RefCountingPtr<RHI::RHILinearBuffer> stageBuffer = device->CreateBuffer(stageBufferDescription);
    FrameTransferContexts[GetCurrentRenderFrame() % DEFAULT_FRAMES_IN_FLIGHT][Thread::GetWorkerTaskThreadIndex()].StageBuffer = stageBuffer;

    RHI::RHIDependencyDesc barrierDependencyDesc;
    RHI::RHIDependencyDesc barrierReadDependencyDesc;
    barrierDependencyDesc.ImageMemoryBarriers.reserve(uploadDescriptions.size());
    barrierReadDependencyDesc.ImageMemoryBarriers.reserve(uploadDescriptions.size());

    std::vector<RHI::RHIImageMemoryBarrierDesc> barrierTransferDescs;
    barrierTransferDescs.reserve(uploadDescriptions.size());
    const uint32 transferQueueIndex = device->GetTransferQueueFamilyIndex();
    const uint32 graphicsQueueIndex = device->GetGraphicsQueueFamilyIndex();
    for (size_t i = 0; i < uploadDescriptions.size(); ++i)
    {
        const RHI::RHIBufferImageCopyDesc& uploadDescription = uploadDescriptions[i];
        ktxTexture2* texture = ktxTextures[i];

        RHI::RHIImageMemoryBarrierDesc& barrierTransferDesc = barrierDependencyDesc.ImageMemoryBarriers.emplace_back();
        barrierTransferDesc.SrcStageFlags = RHI::RHIPipelineStageFlags::None;
        barrierTransferDesc.SrcAccessFlags = RHI::RHIAccessFlags::None;
        barrierTransferDesc.DstStageFlags = RHI::RHIPipelineStageFlags::Transfer;
        barrierTransferDesc.DstAccessFlags = RHI::RHIAccessFlags::TransferWrite;
        barrierTransferDesc.OldLayout = RHI::RHIImageLayout::None;
        barrierTransferDesc.NewLayout = RHI::RHIImageLayout::TransferDst;
        barrierTransferDesc.Image = uploadDescription.Image;
        barrierTransferDesc.SubresourceRange.Aspect = RHI::RHIImageAspectFlags::Color;
        barrierTransferDesc.SubresourceRange.NumMipLevels = texture->numLevels;
        barrierTransferDesc.SubresourceRange.NumArraySlices = 1;

        RHI::RHIImageMemoryBarrierDesc& barrierReadDesc = barrierReadDependencyDesc.ImageMemoryBarriers.emplace_back();
        barrierReadDesc.SrcStageFlags = barrierTransferDesc.DstStageFlags;
        barrierReadDesc.SrcAccessFlags = barrierTransferDesc.DstAccessFlags;
        barrierReadDesc.DstStageFlags = RHI::RHIPipelineStageFlags::None;
        barrierReadDesc.DstAccessFlags = RHI::RHIAccessFlags::None;
        barrierReadDesc.OldLayout = barrierTransferDesc.NewLayout;
        barrierReadDesc.NewLayout = RHI::RHIImageLayout::ShaderReadOnly;
        barrierReadDesc.SrcQueueFamilyIndex = transferQueueIndex;
        barrierReadDesc.DstQueueFamilyIndex = graphicsQueueIndex;
        barrierReadDesc.Image = barrierTransferDesc.Image;
        barrierReadDesc.SubresourceRange = barrierTransferDesc.SubresourceRange;

        stageBuffer->Write(texture->pData, texture->dataSize);

        RHI::RHIImageMemoryBarrierDesc& pendingBarrier = barrierTransferDescs.emplace_back();
        pendingBarrier.SrcStageFlags = barrierReadDesc.DstStageFlags;
        pendingBarrier.SrcAccessFlags = barrierReadDesc.DstAccessFlags;
        pendingBarrier.DstStageFlags = RHI::RHIPipelineStageFlags::FragmentShader;
        pendingBarrier.DstAccessFlags = RHI::RHIAccessFlags::ShaderRead;
        pendingBarrier.OldLayout = barrierReadDesc.NewLayout;
        pendingBarrier.NewLayout = RHI::RHIImageLayout::ShaderReadOnly;
        pendingBarrier.SrcQueueFamilyIndex = transferQueueIndex;
        pendingBarrier.DstQueueFamilyIndex = graphicsQueueIndex;
        pendingBarrier.Image = barrierReadDesc.Image;
        pendingBarrier.SubresourceRange = barrierReadDesc.SubresourceRange;
    }

    {
        std::lock_guard lock(ThisFrameBatchContext.ContextMutex);
        ThisFrameBatchContext.PendingBarriers.ImageBarriers.insert(ThisFrameBatchContext.PendingBarriers.ImageBarriers.end(),
                                                                   barrierTransferDescs.begin(), barrierTransferDescs.end());
    }

    commandList->BeginRecording();

    commandList->PipelineBarrier(barrierDependencyDesc);

    for (const auto& uploadDesc : uploadDescriptions)
    {
        commandList->CopyBufferToImage(stageBuffer, uploadDesc);
    }

    commandList->PipelineBarrier(barrierReadDependencyDesc);

    commandList->EndRecording();

    OnTransferTaskEnd(commandList);
}

void RenderResourceManager::OnTransferTaskEnd(RefCountingPtr<RHI::RHICommandList> CommandList)
{
    bool isLastTask = false;
    {
        std::lock_guard lock(ThisFrameBatchContext.ContextMutex);
        ThisFrameBatchContext.FinalizedList.emplace_back(std::move(CommandList));
        --ThisFrameBatchContext.RemainingTaskCount;
        isLastTask = ThisFrameBatchContext.RemainingTaskCount == 0;
    }

    if (isLastTask)
    {
        ThisFrameBatchContext.CompletionSignal.release();
    }
}
}
