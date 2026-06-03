#include "RendererModule.h"

#include <tracy/Tracy.hpp>

#include "SceneRendering/SceneRenderer.h"
#include "Multithreading/JobScheduler.h"
#include "AssetMasterFile.gen.h"
#include "ECSMasterFile.gen.h"
#include "ShaderMasterRegistryFile.gen.h"
#include "ECS/Ecs.h"

namespace LE
{
void RendererModule::RegisterServices()
{}

void RendererModule::RegisterReflection()
{
	AutoRegistration::RendererModule::RegisterAllAssetTypes(GetServiceRegistry().GetService<AssetStorageFactory>());
	AutoRegistration::RendererModule::RegisterAllSystems(GetECSModule().GetSystemRegistry());
	AutoRegistration::RendererModule::RegisterAllMaterialShader();
}

void RendererModule::ShutdownServices() {}

Renderer::RenderScene& RendererModule::GetRenderScene()
{
	return Scene;
}

RefCountingPtr<Renderer::Viewport> RendererModule::GetViewport(const RefCountingPtr<const SystemWindow> Window)
{
	if (!WindowToViewportInfo.contains(Window))
	{
		CreateViewport(Window);
	}

	return WindowToViewportInfo[Window];
}

void RendererModule::CreateViewport(const RefCountingPtr<const SystemWindow> Window)
{
	if (WindowToViewportInfo.contains(Window))
	{
		return;
	}

	const WindowDescription& description = Window->GetDescription();
	RefCountingPtr<RHI::RHIViewport> rhiViewport = RHI::RHICreateViewport(Window->GetSystemWindowHandle(), static_cast<uint32>(description.DesiredWidth),
														  static_cast<uint32>(description.DesiredHeight), false);
	WindowToViewportInfo[Window] = new Renderer::Viewport(rhiViewport);
}

void RendererModule::DeleteViewport(const RefCountingPtr<const SystemWindow> Window)
{
	if (WindowToViewportInfo.contains(Window))
	{
		WindowToViewportInfo.erase(Window);
	}
}

void RendererModule::BeginRendering(const RefCountingPtr<const SystemWindow> Window, const Renderer::SceneViewInfo& ViewInfo)
{
	RefCountingPtr<Renderer::Viewport> viewport = GetViewport(Window);
	
	Renderer::SceneView newSceneView(ViewInfo);
	newSceneView.SetupViewMatrices(viewport);
	
	// Create Scene Renderers
	Renderer::SceneRender* sceneRender = new Renderer::SceneRender(newSceneView, &Scene);

	viewport->EnqueueBeginRenderFrame();
	// Enqueue Rendering
	Renderer::RenderCommandList::Get().EnqueueLambdaCommand([sceneRender](Renderer::RenderCommandList& CmdList)
	{
		sceneRender->Render();

		delete sceneRender;
	});
	viewport->EnqueueEndRenderFrame();
}

void RendererModule::DrawFrame()
{
	Delegate<void(float)> renderDelegate;
	renderDelegate.Attach<&RendererModule::DrawFrameInternal>(this);
	Renderer::RenderCommandList::Get().FinalizeFrame();
	
	JobScheduler& scheduler = GetServiceRegistry().GetService<JobScheduler>();
	scheduler.StartFrameRender(renderDelegate);
}

void RendererModule::DrawFrameInternal(float)
{
	ZoneScopedN("Draw Frame");
	JobScheduler& scheduler = GetServiceRegistry().GetService<JobScheduler>();
	scheduler.IncrementRenderThreadCount();
	Renderer::RenderCommandList::Get().Render_ExecuteFrame();
	FrameMark;
}
}
