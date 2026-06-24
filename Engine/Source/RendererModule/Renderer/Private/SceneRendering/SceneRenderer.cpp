#include "SceneRendering/SceneRenderer.h"

#include "RHIDevice.h"
#include "ShaderCompiler.h"
#include "ShaderVariationRegistry.h"
#include "PipelineObjects/PipelineObjectManager.h"
#include "RenderContributors/RenderContributorManager.h"
#include "RenderContributors/GlobalContributors/GlobalContributor.h"
#include "RenderDynamicDataManager/RenderDynamicDataManager.h"
#include "ShaderPass/ShaderPass.h"
#include "Templates/Alignment.h"
#include "tracy/Tracy.hpp"

namespace LE::RHI
{
class RHIDevice;
}

namespace LE::Renderer
{
class RenderDynamicDataManager;

void SceneRender::Render()
{
    LE_ASSERT_DESC(Scene, "Scene is null");

    ExtractPipelineBatches();
    WriteContributorsFrameData();

    ExecuteTestPass();
}

void SceneRender::ExtractPipelineBatches()
{
    ZoneScopedN("Extract Pipeline Batches");
    std::vector<RenderProxyState*> enabledProxies;
    Scene->GetEnabledRenderProxies(enabledProxies);

    RenderContributorManager& contributorManager = GetServiceRegistry().GetService<RenderContributorManager>();
    for (auto& proxyState : enabledProxies)
    {
        PermutationVariationKey permutationVariationKey;

        RenderContributor& meshContributor = contributorManager.GetRenderContributor(
            proxyState->MeshVariationTypeId, proxyState->MeshVariationInstanceId);
        meshContributor.AddProxyToThisFrameContribution(proxyState->EntityId);

        permutationVariationKey.MeshContributorTypeId = proxyState->MeshVariationTypeId;
        permutationVariationKey.MeshInstanceId = proxyState->MeshVariationInstanceId;
        if (!BatchStorage.Has(permutationVariationKey))
        {
            PipelineBatchStorage::PipelineBatchData batchData;
            batchData.InstanceCount = 1;
            batchData.MeshInstanceId = proxyState->MeshVariationInstanceId;
            BatchStorage.AddEntrance(permutationVariationKey, batchData);
        }
        else
        {
            BatchStorage.IncrementInstanceCount(permutationVariationKey);
        }
    }
}

void SceneRender::WriteContributorsFrameData()
{
    ZoneScopedN("Write Contributors Frame Data");
    RenderContributorManager& contributorManager = GetServiceRegistry().GetService<RenderContributorManager>();
    Renderer::RenderDynamicDataManager& dynamicDataManager = GetServiceRegistry().GetService<Renderer::RenderDynamicDataManager>();
    contributorManager.WriteContributorsFrameData(dynamicDataManager.GetCurrentFrameRingBuffer());
}

void SceneRender::ExecuteTestPass()
{
    ZoneScopedN("Execute Test Pass");
    RHI::RHIDevice* device = RHI::RHIDevice::Get();
    RefCountingPtr<RHI::RHICommandList> commandList = device->CreateCommandList(RHI::RHICommandListType::Graphics);

    uint32 swapchainImageIndex;
    if (!device->GetNextSwapchainImageIndex(Window, swapchainImageIndex))
    {
        LE_ASSERT_DESC(false, "Failed to get swapchain image index");
        return;
    }

    commandList->BeginRecording();

    RHI::RHIDependencyDesc barrierDependencyDesc;
    RHI::RHIImageMemoryBarrierDesc& swapchainBarrier = barrierDependencyDesc.ImageMemoryBarriers.emplace_back();
    swapchainBarrier.SrcStageFlags = RHI::RHIPipelineStageFlags::ColorTarget;
    swapchainBarrier.SrcAccessFlags = RHI::RHIAccessFlags::None;
    swapchainBarrier.DstStageFlags = RHI::RHIPipelineStageFlags::ColorTarget;
    swapchainBarrier.DstAccessFlags = RHI::RHIAccessFlags::ColorRead | RHI::RHIAccessFlags::ColorWrite;
    swapchainBarrier.OldLayout = RHI::RHIImageLayout::None;
    swapchainBarrier.NewLayout = RHI::RHIImageLayout::Attachment;
    swapchainBarrier.Image = Window->GetSwapchainImage(swapchainImageIndex);
    swapchainBarrier.SubresourceRange.Aspect = RHI::RHIImageAspectFlags::Color;
    swapchainBarrier.SubresourceRange.NumMipLevels = 1;
    swapchainBarrier.SubresourceRange.NumArraySlices = 1;

    RHI::RHIImageMemoryBarrierDesc& depthBarrier = barrierDependencyDesc.ImageMemoryBarriers.emplace_back();
    depthBarrier.SrcStageFlags = RHI::RHIPipelineStageFlags::DepthTarget;
    depthBarrier.SrcAccessFlags = RHI::RHIAccessFlags::DepthWrite;
    depthBarrier.DstStageFlags = RHI::RHIPipelineStageFlags::DepthTarget;
    depthBarrier.DstAccessFlags = RHI::RHIAccessFlags::DepthWrite;
    depthBarrier.OldLayout = RHI::RHIImageLayout::None;
    depthBarrier.NewLayout = RHI::RHIImageLayout::Attachment;
    depthBarrier.Image = Window->GetDepthImage();
    depthBarrier.SubresourceRange.Aspect = RHI::RHIImageAspectFlags::Depth | RHI::RHIImageAspectFlags::Stencil;
    depthBarrier.SubresourceRange.NumMipLevels = 1;
    depthBarrier.SubresourceRange.NumArraySlices = 1;

    commandList->PipelineBarrier(barrierDependencyDesc);

    RHI::RHIRenderingAttachmentDesc colorAttachmentDesc = {};
    colorAttachmentDesc.Attachment = Window->GetSwapchainImageView(swapchainImageIndex);
    colorAttachmentDesc.LoadOp = RHI::RHILoadOp::Clear;
    colorAttachmentDesc.StoreOp = RHI::RHIStoreOp::Store;
    colorAttachmentDesc.ClearValue.ClearColor = LinearColor::Black();

    RHI::RHIRenderingAttachmentDesc depthAttachmentDesc = {};
    depthAttachmentDesc.Attachment = Window->GetDepthImageView();
    depthAttachmentDesc.LoadOp = RHI::RHILoadOp::Clear;
    depthAttachmentDesc.StoreOp = RHI::RHIStoreOp::Ignore;
    depthAttachmentDesc.ClearValue.DepthStencil.ClearDepth = 1.0f;
    depthAttachmentDesc.ClearValue.DepthStencil.ClearStencil = 0;

    RHI::RHIRenderingDesc renderingDesc = {};
    renderingDesc.RectangleExtent = {Window->GetWidth(), Window->GetHeight()};
    renderingDesc.LayerCount = 1,
        renderingDesc.ColorAttachments = &colorAttachmentDesc;
    renderingDesc.ColorAttachmentCount = 1;
    renderingDesc.DepthAttachment = &depthAttachmentDesc;

    commandList->BeginRendering(renderingDesc);

    RHI::RHIViewportDesc viewport;
    viewport.Width = Window->GetWidth();
    viewport.Height = Window->GetHeight();
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    commandList->SetViewport(viewport);
    commandList->SetScissor(Vector2U(Window->GetWidth(), Window->GetHeight()));

    PipelineObjectManager& pipelineObjectManager = GetServiceRegistry().GetService<PipelineObjectManager>();
    size_t typeCount = BatchStorage.GetPermutationTypesCount();

    RenderContributorManager& contributorManager = GetServiceRegistry().GetService<RenderContributorManager>();
    RenderContributor& globalContributor = contributorManager.GetRenderContributor(
                RenderContributorTypeIdGetter<GlobalContributor>::Value, GlobalFrameDataContributorId);
    const uint64 alignment = RHI::GlobalStorageAlignment;
    for (size_t typeIndex = 0; typeIndex < typeCount; ++typeIndex)
    {
        const PermutationVariationKey batchKey = BatchStorage.GetPermutationVariationKey(typeIndex);
        PipelineObjectManager::PipelinePermutationKey permutationKey;
        permutationKey.ShaderTypeId = ShaderPassGetter<TestShaderPass>::Value;
        permutationKey.GlobalContributorTypeId = RenderContributorTypeIdGetter<GlobalContributor>::Value;
        permutationKey.MeshContributorTypeId = batchKey.MeshContributorTypeId;

        RefCountingPtr<RHI::RHIPipelineObject> pipelineObject = pipelineObjectManager.GetPipelineObject(permutationKey);
        if (!pipelineObject)
        {
            continue;
        }

        commandList->BindPipeline(pipelineObject);
        for (const auto& batchVariation : std::ranges::subrange(BatchStorage.GetPermutationVariationBegin(typeIndex),
                                                                BatchStorage.GetPermutationVariationEnd(typeIndex)))
        {
            RenderContributor& meshContributor = contributorManager.GetRenderContributor(
                batchKey.MeshContributorTypeId, batchVariation.MeshInstanceId);
            if (!meshContributor.IsReady())
            {
                continue;
            }

            ShaderPassGetter<TestShaderPass>::PassConstants testPassConstants = {};
            testPassConstants.GlobalFrameDataGpuAddress = globalContributor.GetThisFrameDataGPUAddress();
            testPassConstants.MeshFrameDataGpuAddress = meshContributor.GetThisFrameDataGPUAddress();
            const uint32 indexCount = meshContributor.GetIndexCount();

            RHI::RHIPushConstantsDesc pushConstantsDesc = {};
            pushConstantsDesc.PipelineLayout = pipelineObject->GetPipelineLayout();
            pushConstantsDesc.ShaderStage = RHI::RHIShaderStage::Vertex;
            TStructAligned<ShaderPassGetter<TestShaderPass>::PassConstants> alignedPushConstants = testPassConstants;
            pushConstantsDesc.Size = Align(sizeof(alignedPushConstants), alignment);
            pushConstantsDesc.Data = &alignedPushConstants;

            commandList->PushConstants(pushConstantsDesc);
            commandList->Draw(indexCount, batchVariation.InstanceCount);
        }
    }
    commandList->EndRendering();

    RHI::RHIDependencyDesc presentDependencyDesc;
    RHI::RHIImageMemoryBarrierDesc& presentBarrier = presentDependencyDesc.ImageMemoryBarriers.emplace_back();
    presentBarrier.SrcStageFlags = RHI::RHIPipelineStageFlags::ColorTarget;
    presentBarrier.SrcAccessFlags = RHI::RHIAccessFlags::ColorWrite;
    presentBarrier.DstStageFlags = RHI::RHIPipelineStageFlags::ColorTarget;
    presentBarrier.DstAccessFlags = RHI::RHIAccessFlags::None;
    presentBarrier.OldLayout = RHI::RHIImageLayout::Attachment;
    presentBarrier.NewLayout = RHI::RHIImageLayout::Present;
    presentBarrier.Image = Window->GetSwapchainImage(swapchainImageIndex);
    presentBarrier.SubresourceRange.Aspect = RHI::RHIImageAspectFlags::Color;
    presentBarrier.SubresourceRange.NumMipLevels = 1;
    presentBarrier.SubresourceRange.NumArraySlices = 1;
    commandList->PipelineBarrier(presentDependencyDesc);

    commandList->EndRecording();
    device->SubmitCommandList(RHI::RHICommandListType::Graphics, {commandList}, swapchainImageIndex);
    device->Present(Window, swapchainImageIndex);
}
}
