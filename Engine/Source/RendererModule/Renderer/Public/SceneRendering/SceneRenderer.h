#pragma once
#include "PipelineBatchStorage.h"
#include "RenderScene.h"
#include "RenderGraph/RenderGraph.h"
#include "Templates/NonCopyable.h"

namespace LE::Renderer
{
class SceneRender : NonCopyable
{
public:
    SceneRender(RenderScene* InScene, RenderContributorId GlobalInstanceId, RefCountingPtr<RHI::RHIWindow> TargetWindow) : Scene(InScene),
        Window(TargetWindow), GlobalFrameDataContributorId(GlobalInstanceId)
    {
    }

    void Render();

private:
    void ExtractPipelineBatches();
    void WriteContributorsFrameData();
    void AddTestPass(RenderGraph& RGraph, RGTexture Color, RGTexture Depth);

private:
    RenderScene* Scene;
    RefCountingPtr<RHI::RHIWindow> Window;
    RenderContributorId GlobalFrameDataContributorId = NullId{};

    PipelineBatchStorage BatchStorage;
};
}
