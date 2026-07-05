#include "RendererModule.h"

#include <tracy/Tracy.hpp>

#include "SceneRendering/SceneRenderer.h"
#include "Multithreading/JobScheduler.h"
#include "AssetMasterFile.gen.h"
#include "ECSMasterFile.gen.h"
#include "RenderCommandList.h"
#include "RenderCore.h"
#include "ShaderCompiler.h"
#include "ShaderMasterRegistryFile.gen.h"
#include "ShaderVariationRegistry.h"
#include "VulkanDevice.h"
#include "ECS/Ecs.h"
#include "PipelineObjects/PipelineObjectManager.h"
#include "RenderResourceManager/RenderResourceManager.h"
#include "RenderContributors/RenderContributorManager.h"
#include "RenderContributors/GlobalContributors/GlobalContributor.h"
#include "RenderDynamicDataManager/RenderDynamicDataManager.h"
#include "RenderGraph/RenderGraph.h"

namespace LE
{
void RendererModule::RegisterServices()
{
	GetServiceRegistry().RegisterService<RHI::ShaderCompiler>();
	GetServiceRegistry().RegisterService<Renderer::RenderResourceManager>();
	GetServiceRegistry().RegisterService<Renderer::RenderContributorManager>();
	GetServiceRegistry().RegisterService<Renderer::RenderDynamicDataManager>();
	GetServiceRegistry().RegisterService<Renderer::PipelineObjectManager>();
	GetServiceRegistry().RegisterService<Renderer::ShaderVariationRegistry>();
	GetServiceRegistry().RegisterService<Renderer::RenderGraph>();
}

void RendererModule::RegisterReflection()
{
	AutoRegistration::RendererModule::RegisterAllAssetTypes(GetServiceRegistry().GetService<AssetStorageFactory>());
	AutoRegistration::RendererModule::RegisterAllSystems(GetECSModule().GetSystemRegistry());
	AutoRegistration::RendererModule::RegisterAllShaderVariations(GetServiceRegistry().GetService<Renderer::ShaderVariationRegistry>());
}

void RendererModule::ShutdownServices()
{
	GetServiceRegistry().UnregisterService<RHI::ShaderCompiler>();
	GetServiceRegistry().UnregisterService<Renderer::RenderResourceManager>();
	GetServiceRegistry().UnregisterService<Renderer::RenderContributorManager>();
	GetServiceRegistry().UnregisterService<Renderer::RenderDynamicDataManager>();
	GetServiceRegistry().UnregisterService<Renderer::PipelineObjectManager>();
	GetServiceRegistry().UnregisterService<Renderer::ShaderVariationRegistry>();
	GetServiceRegistry().UnregisterService<Renderer::RenderGraph>();
}

void RendererModule::InitializeWithVulkanDevice()
{
	Device = std::make_unique<RHI::Vulkan::VulkanDevice>();
	Device->Initialize();
	RHI::RHIDevice::Register(Device.get());
}

void RendererModule::InitializeGlobalFrameData()
{
	Renderer::RenderContributorManager& renderContributorManager = GetServiceRegistry().GetService<Renderer::RenderContributorManager>();
	GlobalFrameDataContributorId = renderContributorManager.CreateRenderContributor<Renderer::GlobalContributor>().GetInstanceId();
}

Renderer::RenderScene& RendererModule::GetRenderScene()
{
	return Scene;
}

void RendererModule::CreateRHIWindow(RefCountingPtr<const LEWindow> Window)
{
	if (RhiWindowMap.contains(Window))
	{
		return;
	}
	
	RHI::RHIWindowDesc description;
	description.NativeWindowHandle = Window->GetWindowNativeHandle();
	description.Height = Window->GetHeight();
	description.Width = Window->GetWidth();
	RhiWindowMap[Window] = Device->CreateWindow(description);
}

void RendererModule::DeleteRHIWindow(RefCountingPtr<const LEWindow> Window)
{
	if (RhiWindowMap.contains(Window))
	{
		Device->DestroyWindow(RhiWindowMap.at(Window));
		RhiWindowMap.erase(Window);
	}
}

RefCountingPtr<RHI::RHIWindow> RendererModule::GetRhiWindow(RefCountingPtr<const LEWindow> Window)
{
	if (RhiWindowMap.contains(Window))
	{
		return RhiWindowMap.at(Window);
	}
	return nullptr;
}

void RendererModule::AddFrame(RefCountingPtr<const LEWindow> Window, const Renderer::SceneViewInfo& ViewInfo)
{
	Renderer::SceneViewInfo sceneViewInfo = ViewInfo;
	sceneViewInfo.RhiWindow = GetRhiWindow(Window);

	Renderer::SceneView newSceneView(std::move(sceneViewInfo));
	newSceneView.SetupViewMatrices();
	EnqueueFrameSceneView(newSceneView);
}

void RendererModule::ScheduleDrawFrame()
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
	Renderer::IncrementCurrentRenderFrame();

	JobScheduler& scheduler = GetServiceRegistry().GetService<JobScheduler>();
	scheduler.IncrementRenderThreadCount();

	Renderer::RenderResourceManager& resourceManager = GetServiceRegistry().GetService<Renderer::RenderResourceManager>();
	Renderer::RenderDynamicDataManager& dynamicDataManager = GetServiceRegistry().GetService<Renderer::RenderDynamicDataManager>();

	// Here we will wait for anything not finished in the earliest frame
	Device->BeginFrame();
	// Clean up from the earliest frame
	resourceManager.OnBeginFrame();
	dynamicDataManager.OnBeginFrame();

	// Dispatch Batch Load Tasks
	resourceManager.DispatchBatchLoadTasks();

	Renderer::RenderCommandList::Get().ExecuteRenderCommands();

	// Update global contributor
	Renderer::GlobalContributor& globalContributor = GetServiceRegistry().GetService<Renderer::RenderContributorManager>().GetRenderContributor<
		Renderer::GlobalContributor>(GlobalFrameDataContributorId);
	Renderer::GlobalContributor::GlobalFrameDynamicData& frameData = globalContributor.GetDynamicData();
	Renderer::SceneView thisFrameView = GetFrameSceneView();

	frameData.ViewToClip = thisFrameView.ThisFrameViewMatrices.ViewToClip;
	frameData.WorldToView = thisFrameView.ThisFrameViewMatrices.WorldToView;

	// Start Drawing frame
	Renderer::SceneRender renderer(&Scene, GlobalFrameDataContributorId, thisFrameView.ViewInfo.RhiWindow);
	renderer.Render();
	
	// Finalize Load Tasks
	resourceManager.FinalizeBatchLoad();

	FrameMark;
}

void RendererModule::EnqueueFrameSceneView(Renderer::SceneView View)
{
	std::unique_lock lock(SceneViewMutex);
	FrameSceneViews.push_back(std::move(View));
}

Renderer::SceneView RendererModule::GetFrameSceneView()
{
	std::unique_lock lock(SceneViewMutex);
	Renderer::SceneView thisFrameView = FrameSceneViews.back();
	FrameSceneViews.pop_back();
	return std::move(thisFrameView);
}

void RendererModule::Shutdown()
{
	ModuleBase::Shutdown();
	Device->Shutdown();
}
}
