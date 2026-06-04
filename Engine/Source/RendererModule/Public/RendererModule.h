#pragma once
#include "Module.h"
#include "Application/SystemWindow.h"
#include "SceneRendering/RenderScene.h"
#include "Viewport.h"
#include "SceneRendering/SceneView.h"

namespace LE
{
class RendererModule : public ModuleBase
{
public:
	void RegisterServices() override;
	void RegisterReflection() override;
	void ShutdownServices() override;

	Renderer::RenderScene& GetRenderScene();

	RefCountingPtr<Renderer::Viewport> GetViewport(const RefCountingPtr<const SystemWindow> Window);

	void CreateViewport(const RefCountingPtr<const SystemWindow> Window);
	void DeleteViewport(const RefCountingPtr<const SystemWindow> Window);

	void BeginRendering(const RefCountingPtr<const SystemWindow> Window, const Renderer::SceneViewInfo& ViewInfo);
	void DrawFrame();

protected:
	void DrawFrameInternal(float);
	
private:
	Map<RefCountingPtr<const SystemWindow>, RefCountingPtr<Renderer::Viewport>> WindowToViewportInfo;
	Renderer::RenderScene Scene;
};

REGISTER_MODULE(RendererModule, "RendererModule")
}
