#include "GameEngine.h"

#include <thread>
#include <windows.h>

#include "CoreECSModule.h"
#include "EngineCoreModule.h"
#include "EngineGlobals.h"
#include "EngineToolsModule.h"
#include "RenderCommandList.h"
#include "RendererECSCoreModule.h"
#include "SystemManager.h"
#include "AssetManager/AssetManager.h"
#include "common/TracySystem.hpp"
#include "EventCore/EventManager.h"
#include "Multithreading/JobScheduler.h"
#include "Time/Clock.h"
#include "tracy/Tracy.hpp"

namespace LE
{
GameEngine gGameEngine;

#define NUM_WORKER_THREADS 4
#define NUM_RENDER_THREADS 1

void GameEngine::Init()
{
	tracy::SetThreadName("Main thread");
	RegisterEngine(this);

	RegisterModuleRegistry(&ModuleReg);
	RegisterModules();
	
	RendererModule& rendererModule = GetModuleRegistry().GetModule<RendererModule>();
	rendererModule.InitializeWithVulkanDevice();
	
	RegisterServiceRegistry(&ServiceReg);
	ModuleReg.RegisterServices();
	
	GameWorld = new World;
	GameWorld->Init();
	
	ModuleReg.RegisterReflection();
	
	GameWorld->InitTestData();

	rendererModule.InitializeGlobalFrameData();
	
	MakeWindow();

	InitJobScheduler();
	
	JobScheduler& scheduler = ServiceReg.GetService<JobScheduler>();
	scheduler.ConstructUpdateGraph();
}

void GameEngine::Shutdown()
{
	ServiceReg.GetService<JobScheduler>().Shutdown();
	RHI::RHIDevice* device = RHI::RHIDevice::Get();
	device->WaitIdle();
	if (MainWindow)
	{
		GetServiceRegistry().GetService<SystemManager>().DestroyWindow(MainWindow);
	}
	MainWindow.Release();

	GameWorld->Shutdown();
	delete GameWorld;

	ServiceReg.ShutDown();

	ModuleReg.Shutdown();
}

void GameEngine::Update(bool& IsDone)
{
	const Clock::TimePoint frameBeginning = Clock::Now();
	
	SystemManager& systemManager = GetServiceRegistry().GetService<SystemManager>();
	IsDone = systemManager.PoolEvents();
	if (IsDone)
	{
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
	rendererModule.AddFrame(MainWindow, GameWorld->GetPrimaryViewInfo());
	rendererModule.ScheduleDrawFrame();
}

void GameEngine::MakeWindow()
{
	MainWindow = GetServiceRegistry().GetService<SystemManager>().CreateLEWindow("LE Engine", 1280, 720);
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

void GameEngine::InitJobScheduler()
{
	ServiceReg.RegisterService<JobScheduler>();
	const int availableThreadCount = std::thread::hardware_concurrency();
	const int capCount = Min(availableThreadCount, static_cast<int>(Constants<int8>::CMax));
	static constexpr int reservedThreads = DEFAULT_TASK_WORKER_THREADS + NUM_RENDER_THREADS + 1;
	LE_ASSERT_DESC(capCount > reservedThreads, "Should be at least {} threads", reservedThreads)

	const int8 workerThreadCount = static_cast<int8>(Min(NUM_WORKER_THREADS, (capCount - reservedThreads)));

	JobScheduler& scheduler = ServiceReg.GetService<JobScheduler>();
	scheduler.StartThreads(workerThreadCount, DEFAULT_TASK_WORKER_THREADS);

	Renderer::RenderCommandList::Get().Initialize(workerThreadCount + DEFAULT_TASK_WORKER_THREADS);
	scheduler.StartRenderThread();
}
}
