#pragma once
#include "Application/SystemWindow.h"
#include "SceneRendering/RenderScene.h"
#include "SceneRendering/SceneView.h"

namespace LE::Renderer
{
struct ViewportInfo : RenderResource
{
	ViewportInfo()
		:
		ViewportRHI(nullptr)
		, SystemWindowHandle(nullptr)
	{
	}

	RefCountingPtr<RHI::RHIViewport> ViewportRHI;
	void* SystemWindowHandle;
};

class RendererModule
{
public:
	RenderScene& GetRenderScene();

	RefCountingPtr<RHI::RHIViewport> GetViewport(const RefCountingPtr<SystemWindow> Window);

	void CreateViewport(const RefCountingPtr<SystemWindow> Window);
	void DeleteViewport(const RefCountingPtr<SystemWindow> Window);

	void BeginRendering(const SceneView& View);

private:
	Map<const SystemWindow*, ViewportInfo*> WindowToViewportInfo; // If we ever create separate UI renderer it needs to be moved there
	RenderScene Scene;
};
}
