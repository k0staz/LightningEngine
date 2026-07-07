#include "SceneRendering/SceneRenderer.h"

#include "RHIDevice.h"
#include "ShaderCompiler.h"
#include "ShaderVariationRegistry.h"
#include "PipelineObjects/PipelineObjectManager.h"
#include "RenderContributors/RenderContributorManager.h"
#include "RenderContributors/GlobalContributors/GlobalContributor.h"
#include "RenderDynamicDataManager/RenderDynamicDataManager.h"
#include "RenderResourceManager/RenderResourceManager.h"
#include "ShaderPass/ShaderPass.h"
#include "Templates/Alignment.h"
#include "RenderGraph/RenderGraph.h"
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

    auto& renderGraph = GetServiceRegistry().GetService<RenderGraph>();

    RGTexture color = renderGraph.ImportSwapchainImage(Window);
    RGTexture depth = renderGraph.ImportDepthImage(Window);

    AddTestPass(renderGraph, color, depth);

    renderGraph.Compile();
    renderGraph.Execute();
    renderGraph.Present(Window);
    renderGraph.Reset();
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
        permutationVariationKey.MeshContributorTypeId = proxyState->MeshVariationTypeId;
        permutationVariationKey.MeshInstanceId = proxyState->MeshVariationInstanceId;
        permutationVariationKey.MaterialContributorTypeId = proxyState->MaterialVariationTypeId;
        permutationVariationKey.MaterialInstanceId = proxyState->MaterialVariationInstanceId;

        RenderContributor& meshContributor = contributorManager.GetRenderContributor(
            proxyState->MeshVariationTypeId, proxyState->MeshVariationInstanceId);
        meshContributor.AddProxyToThisFrameContribution(proxyState->EntityId, permutationVariationKey);

        if (!BatchStorage.Has(permutationVariationKey))
        {
            PipelineBatchStorage::PipelineBatchData batchData;
            batchData.InstanceCount = 1;
            batchData.MeshInstanceId = proxyState->MeshVariationInstanceId;
            batchData.MaterialInstanceId = proxyState->MaterialVariationInstanceId;
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

void SceneRender::AddTestPass(RenderGraph& RGraph, RGTexture Color, RGTexture Depth)
{
    RGraph.AddPass("TestPass",
        [&](RGBuilder& builder)
        {
            builder.SetColorAttachment(0, Color, RHI::RHILoadOp::Clear, RHI::RHIStoreOp::Store, LinearColor::Black());
            builder.SetDepthAttachment(Depth, RHI::RHILoadOp::Clear, RHI::RHIStoreOp::Ignore, 1.0f, 0);
        },
        [=, this](RHI::RHICommandList& Cmd, RenderGraph& Graph)
        {
            auto& pipelineObjectManager = GetServiceRegistry().GetService<PipelineObjectManager>();
            auto& contributorManager = GetServiceRegistry().GetService<RenderContributorManager>();
            auto& globalContributor = contributorManager.GetRenderContributor(RenderContributorTypeIdGetter<GlobalContributor>::Value,
                GlobalFrameDataContributorId);
            auto& renderResourceManager = GetServiceRegistry().GetService<RenderResourceManager>();
            RefCountingPtr<RHI::RHIDescriptorSet> globalDescriptorSet = renderResourceManager.GetGlobalDescriptorSet();

            size_t typeCount = BatchStorage.GetPermutationTypesCount();
            const uint64 alignment = RHI::GlobalStorageAlignment;
            for (size_t typeIndex = 0; typeIndex < typeCount; ++typeIndex)
            {

                const PermutationVariationKey batchKey = BatchStorage.GetPermutationVariationKey(typeIndex);
                PipelineObjectManager::PipelinePermutationKey permutationKey;
                permutationKey.ShaderTypeId = ShaderPassGetter<TestShaderPass>::Value;
                permutationKey.GlobalContributorTypeId = RenderContributorTypeIdGetter<GlobalContributor>::Value;
                permutationKey.MeshContributorTypeId = batchKey.MeshContributorTypeId;
                permutationKey.MaterialContributorTypeId = batchKey.MaterialContributorTypeId;

                RefCountingPtr<RHI::RHIPipelineObject> pipelineObject = pipelineObjectManager.GetPipelineObject(permutationKey);
                if (!pipelineObject)
                {
                    continue;
                }

                Cmd.BindPipeline(pipelineObject);
                Cmd.BindDescriptorSets(pipelineObject->GetPipelineLayout(), {globalDescriptorSet});

                for (const auto& batchVariation : std::ranges::subrange(BatchStorage.GetPermutationVariationBegin(typeIndex),
                                                                        BatchStorage.GetPermutationVariationEnd(typeIndex)))
                {
                    RenderContributor& meshContributor = contributorManager.GetRenderContributor(
                        batchKey.MeshContributorTypeId, batchVariation.MeshInstanceId);
                    if (!meshContributor.IsReady())
                    {
                        continue;
                    }
                    RenderContributor& materialContributor = contributorManager.GetRenderContributor(
                        batchKey.MaterialContributorTypeId, batchVariation.MaterialInstanceId);
                    if (!materialContributor.IsReady())
                    {
                        continue;
                    }

                    ShaderPassGetter<TestShaderPass>::PassConstants testPassConstants = {};
                    testPassConstants.GlobalFrameDataGpuAddress = globalContributor.GetThisFrameDataGPUAddress(batchKey);
                    testPassConstants.MeshFrameDataGpuAddress = meshContributor.GetThisFrameDataGPUAddress(batchKey);
                    testPassConstants.MaterialFrameDataGpuAddress = materialContributor.GetThisFrameDataGPUAddress(batchKey);
                    const uint32 indexCount = meshContributor.GetIndexCount();

                    RHI::RHIPushConstantsDesc pushConstantsDesc = {};
                    pushConstantsDesc.PipelineLayout = pipelineObject->GetPipelineLayout();
                    pushConstantsDesc.ShaderStage = RHI::RHIShaderStage::Vertex;
                    TStructAligned<ShaderPassGetter<TestShaderPass>::PassConstants> alignedPushConstants = testPassConstants;
                    pushConstantsDesc.Size = Align(sizeof(alignedPushConstants), alignment);
                    pushConstantsDesc.Data = &alignedPushConstants;

                    Cmd.PushConstants(pushConstantsDesc);
                    Cmd.Draw(indexCount, batchVariation.InstanceCount);
                }
            }
        }
    );
}
}
