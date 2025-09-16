#include "GameEngine.h"

#include <thread>

#include "D3D11DynamicRHI.h"
#include "EngineGlobals.h"
#include "GameViewport.h"
#include "WindowsWindow.h"
#include "Application/SystemWindow.h"
#include "EventCore/EventManager.h"
#include "Multithreading/JobScheduler.h"
#include "Time/Clock.h"

namespace LE
{
GameEngine gGameEngine;

void GameEngine::Init()
{
	RegisterEngine(this);
	D3D11::UseD3D11RHIModule();
	InitMaterials();

	GameWorld = new World;
	GameWorld->Init();

	InitJobScheduler();

	RHI::InitRHI();

	MakeWindow();

	Viewport = new GameViewport;
	Viewport->Viewport = new Renderer::Viewport(Viewport, GetRendererModule()->GetViewport(Window));
	const WindowDescription& description = Window->GetDescription();
	Viewport->Viewport->SetSizeXY(description.DesiredWidth, description.DesiredHeight);
}

void GameEngine::Shutdown()
{
	JobScheduler* scheduler = JobScheduler::Get();
	scheduler->Shutdown();

	GameWorld->Shutdown();
	delete GameWorld;
	delete Viewport;

	if (Window)
	{
		RendererModule.DeleteViewport(Window);
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

	gEventManager.DispatchEvents();

	Clock::StartFrame();

	JobScheduler* scheduler = JobScheduler::Get();
	scheduler->StartFrame();
	scheduler->WaitForAll(); // TODO: Use function do jobs or smth like this, so GT would do something as well

	const Clock::TimePoint frameEnd = Clock::Now();
	LE_INFO("Frame Finished, took {}ms", Clock::GetMsBetween(frameBeginning, frameEnd));

	DrawViewport();
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

void GameEngine::DrawViewport()
{
	Viewport->Viewport->Draw();
	Renderer::RenderCommandList::Get().Execute();
	Renderer::RenderCommandList::Get().Clear();
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
	JobScheduler* scheduler = JobScheduler::Get();
	scheduler->Init(static_cast<int>(std::thread::hardware_concurrency()) - 2);
}
}
