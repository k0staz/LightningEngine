#pragma once
#include "RenderScene.h"
#include "SceneView.h"
#include "MeshPassCommandBuilders/MeshPassCommandBuilder.h"


namespace LE::Renderer
{
class SceneRender
{
public:
	SceneRender(SceneView View, const RenderScene* SceneToRender);

	void Render();

	void BeginInitViews();

	static void RenderThreadEnd(SceneRender* Renderer);

	void RenderBasePass();
	void SetupBasePassState(MeshPassRenderState& RenderState);

private:
	SceneView View;
	const RenderScene* Scene;
};
}
