#include "GameViewport.h"

#include "EngineGlobals.h"
#include "SceneRendering/SceneView.h"

namespace LE
{
void GameViewport::Draw()
{
	const Renderer::SceneViewInfo& viewInfo = GetWorld()->GetPrimaryViewInfo();
	Renderer::SceneView newSceneView(viewInfo);

	newSceneView.SetupViewMatrices(Viewport);

	GetRendererModule()->BeginRendering(newSceneView);
}
}
