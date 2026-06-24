#include "SystemManager.h"

#include "Core.h"
#include "ModuleRegistry.h"
#include "RendererModule.h"
#include "SDL3/SDL_init.h"

namespace LE
{
void SystemManager::Initialize()
{
	if (!SDL_Init(SDL_INIT_VIDEO))
	{
		LE_ASSERT_DESC(false, "Failed to initialize SDL")
	}
}

void SystemManager::Shutdown()
{
	SDL_QuitSubSystem(SDL_INIT_VIDEO);
	SDL_Quit();
}

RefCountingPtr<LEWindow> SystemManager::CreateLEWindow(const std::string& WindowName, int32_t Width, int32_t Height)
{
	SDL_Window* WindowHandle = SDL_CreateWindow(WindowName.c_str(), Width, Height, SDL_WINDOW_VULKAN);
	LE_ASSERT_DESC(WindowHandle != nullptr, "Failed to create SDL window")
	RefCountingPtr<LEWindow> newWindow = new LEWindow(WindowHandle, WindowName, Width, Height);

	GetModuleRegistry().GetModule<RendererModule>().CreateRHIWindow(newWindow);
	return newWindow;
}

void SystemManager::DestroyWindow(RefCountingPtr<LEWindow> Window)
{
	GetModuleRegistry().GetModule<RendererModule>().DeleteRHIWindow(Window);
	SDL_DestroyWindow(static_cast<SDL_Window*>(Window->GetWindowNativeHandle()));
	Window->Reset();
}

bool SystemManager::PoolEvents()
{
	bool quit = false;
	for (SDL_Event event; SDL_PollEvent(&event);)
	{
		if (event.type == SDL_EVENT_QUIT)
		{
			quit = true;
			break;
		}
	}
	return quit;
}
}
