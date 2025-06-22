#include "RendererModule.h"

#include "SceneRendering/SceneRenderer.h"

namespace LE::Renderer
{
RenderScene& RendererModule::GetRenderScene()
{
	return Scene;
}

RefCountingPtr<RHI::RHIViewport> RendererModule::GetViewport(const RefCountingPtr<SystemWindow> Window)
{
	if (!WindowToViewportInfo.contains(Window))
	{
		CreateViewport(Window);
	}

	ViewportInfo* viewportInfo = WindowToViewportInfo[Window];

	return viewportInfo->ViewportRHI;
}

void RendererModule::CreateViewport(const RefCountingPtr<SystemWindow> Window)
{
	if (WindowToViewportInfo.contains(Window))
	{
		return;
	}

	const WindowDescription& description = Window->GetDescription();

	ViewportInfo* newInfo = new ViewportInfo();
	newInfo->SystemWindowHandle = Window->GetSystemWindowHandle();
	newInfo->ViewportRHI = RHI::RHICreateViewport(newInfo->SystemWindowHandle, static_cast<uint32>(description.DesiredWidth),
	                                              static_cast<uint32>(description.DesiredHeight), false);

	WindowToViewportInfo[Window] = newInfo;
}

void RendererModule::DeleteViewport(const RefCountingPtr<SystemWindow> Window)
{
	if (WindowToViewportInfo.contains(Window))
	{
		delete WindowToViewportInfo[Window];
		WindowToViewportInfo.erase(Window);
	}
}
}
