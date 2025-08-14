#include "SceneRendering/SceneRenderer.h"

#include "RendererModule.h"
#include "StaticStateResource.h"
#include "MeshPassCommandBuilders/MeshPassCommandBuilder.h"

namespace LE::Renderer
{
SceneRender::SceneRender(SceneView View, const RenderScene* SceneToRender)
	: View(View)
	  , Scene(SceneToRender)
{
}

void SceneRender::Render()
{
	BeginInitViews();

	// Render Passes go here
	// TODO: For now we just call them once after another, but later here we will be building render graph
	RenderBasePass();

}

void SceneRender::BeginInitViews()
{
	View.InitResourcesRHI();
}

void SceneRender::RenderThreadEnd(SceneRender* Renderer)
{
	delete Renderer;
}

void SceneRender::RenderBasePass()
{
	MessPassCommandBuilder commandBuilder;
	commandBuilder.PassType = RenderPassType::Base;
	SetupBasePassState(commandBuilder.PassRenderState);

	const Map<EcsEntity, RenderObjectProxy*>& proxies = Scene->GetProxyMap();
	for (auto& it : proxies)
	{
		MeshGroup meshGroup;
		it.second->GetMeshGroup(meshGroup);

		commandBuilder.BuildMeshDrawCommands(meshGroup, &View);
	}

	for (auto& drawCommand : commandBuilder.DrawList)
	{
		if (MeshDrawCommand::SubmitDrawBegin(drawCommand, RenderCommandList::Get()))
		{
			MeshDrawCommand::SubmitDrawEnd(drawCommand, RenderCommandList::Get());
		}
	}
}

void SceneRender::SetupBasePassState(MeshPassRenderState& RenderState)
{
	RenderState.SetDepthStencilState(StaticDepthStencilState<>::GetRHI());
}

static void RenderSceneView_RenderThread(RenderCommandList& CmdList, SceneRender* Renderer)
{
	Renderer->Render();

	SceneRender::RenderThreadEnd(Renderer);
}

void RendererModule::BeginRendering(const SceneView& View)
{
	// Create Scene Renderers
	SceneRender* sceneRender = new SceneRender(View, &Scene);

	// Enqueue Rendering
	RenderCommandList::Get().EnqueueLambdaCommand([sceneRender](RenderCommandList& CmdList)
	{
		RenderSceneView_RenderThread(CmdList, sceneRender);
	});
}
}
