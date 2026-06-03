#include "GameEngine.h"

#include <thread>

#include "CoreECSModule.h"
#include "D3D11DynamicRHI.h"
#include "EngineCoreModule.h"
#include "EngineGlobals.h"
#include "EngineToolsModule.h"
#include "FBXImporter.h"
#include "RendererECSCoreModule.h"
#include "WindowsWindow.h"
#include "Application/SystemWindow.h"
#include "AssetManager/AssetManager.h"
#include "common/TracySystem.hpp"
#include "EventCore/EventManager.h"
#include "Multithreading/JobScheduler.h"
#include "Time/Clock.h"
#include "tracy/Tracy.hpp"

namespace LE
{
GameEngine gGameEngine;

#define NUM_TASK_THREADS 1
#define NUM_WORKER_THREADS 4
#define NUM_RENDER_THREADS 1

void GameEngine::Init()
{
	tracy::SetThreadName("Main thread");
	RegisterEngine(this);

	RegisterModuleRegistry(&ModuleReg);
	RegisterModules();

	RegisterServiceRegistry(&ServiceReg);
	ModuleReg.RegisterServices();
	
	GameWorld = new World;
	GameWorld->Init();
	
	ModuleReg.RegisterReflection();
	
	GameWorld->InitTestData();
	
	D3D11::UseD3D11RHIModule();
	InitMaterials();
	
	RHI::InitRHI();

	MakeWindow();
	RefCountingPtr<Renderer::Viewport> viewport = GetModuleRegistry().GetModule<RendererModule>().GetViewport(Window);

	InitJobScheduler();
	
	JobScheduler& scheduler = ServiceReg.GetService<JobScheduler>();
	scheduler.ConstructUpdateGraph();
}

void GameEngine::Shutdown()
{
	ServiceReg.ShutDown();

	GameWorld->Shutdown();
	delete GameWorld;

	if (Window)
	{
		GetModuleRegistry().GetModule<RendererModule>().DeleteViewport(Window);
		delete Window;
	}

	RHI::DeleteRHI();
}

void GameEngine::Update(bool& IsDone)
{
	const Clock::TimePoint frameBeginning = Clock::Now();
	// TODO: This needs to be abstracted at some point
	MSG msg;
	ZeroMemory(&msg, sizeof(MSG));
	if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	if (msg.message == WM_QUIT)
	{
		IsDone = true;
		return;
	}
	
	GetServiceRegistry().GetService<EventManager>().DispatchEvents();

	Clock::StartFrame();

	JobScheduler& scheduler = ServiceReg.GetService<JobScheduler>();
	scheduler.StartFrame();
	scheduler.HelpWorkerThreads();

	scheduler.WaitForAll();

	const Clock::TimePoint frameEnd = Clock::Now();
	LE_INFO("Frame Finished, took {}ms", Clock::GetMsBetween(frameBeginning, frameEnd));

	GetServiceRegistry().GetService<AssetManager>().OnFrameEnd();
	FrameMarkNamed("Game Frame");
	
	RendererModule& rendererModule = GetModuleRegistry().GetModule<RendererModule>();
	rendererModule.BeginRendering(Window, GameWorld->GetPrimaryViewInfo());
	rendererModule.DrawFrame();
}

void GameEngine::MakeWindow()
{
	WindowDescription description;

	description.DesiredWidth = 800;
	description.DesiredHeight = 600;
	description.DesiredScreenPositionX = (GetSystemMetrics(SM_CXSCREEN) - description.DesiredWidth) / 2;
	description.DesiredScreenPositionY = (GetSystemMetrics(SM_CYSCREEN) - description.DesiredHeight) / 2;

	Windows::WindowsWindow* newWindow = new Windows::WindowsWindow;
	newWindow->Init(description, GetModuleHandle(nullptr));

	Window = newWindow;

	Window->Show();
	Window->PushToFront();
	Window->SetInFocus();
}

void GameEngine::RegisterModules()
{
	ModuleRegistry& moduleRegistry = GetModuleRegistry();
	moduleRegistry.RegisterModule<EngineCoreModule>();
	moduleRegistry.RegisterModule<CoreECSModule>();
	moduleRegistry.RegisterModule<RendererModule>();
	moduleRegistry.RegisterModule<RendererEcsCoreModule>();
	moduleRegistry.RegisterModule<EngineToolsModule>();
}

void GameEngine::InitMaterials()
{
	LE_INFO("-------------------------Starting Shader Registration-------------------------");
	Renderer::ShaderMetaTypeRegistration::RegisterAll();
	LE_INFO("-------------------------Shader Registration is Finished-------------------------");

	const Renderer::Material::MaterialRegistry& materialRegistry = Renderer::Material::GetMaterialRegistry();
	LE_INFO("Found {} Materials: ", materialRegistry.size());
	for (const auto& it : materialRegistry)
	{
		LE_INFO("{}", it.first.c_str());
	}
}

void GameEngine::InitJobScheduler()
{
	ServiceReg.RegisterService<JobScheduler>();
	const int availableThreadCount = std::thread::hardware_concurrency();
	const int capCount = Min(availableThreadCount, static_cast<int>(Constants<int8>::CMax));
	static constexpr int reservedThreads = NUM_TASK_THREADS + NUM_RENDER_THREADS + 1;
	LE_ASSERT_DESC(capCount > reservedThreads, "Should be at least {} threads", reservedThreads)

	const int8 workerThreadCount = static_cast<int8>(Min(NUM_WORKER_THREADS, (capCount - reservedThreads)));

	JobScheduler& scheduler = ServiceReg.GetService<JobScheduler>();
	scheduler.StartThreads(workerThreadCount, NUM_TASK_THREADS);

	Renderer::RenderCommandList::Get().Initialize(workerThreadCount + NUM_TASK_THREADS);
	scheduler.StartRenderThread();
}
}
