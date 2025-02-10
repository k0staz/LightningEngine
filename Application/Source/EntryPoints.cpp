#pragma once

#include "Scene/SceneManager.h"

#if PLATFORM_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

int MainImpl()
{
	Log::Initialize();

	LE::gSceneManager.LoadCurrentScene();

	while (true)
	{
		LE::gSceneManager.Update();
		LE::gSceneManager.PostUpdate();
	}

	return 0;
}

#if PLATFORM_WINDOWS
int WINAPI WinMain(_In_ HINSTANCE /*hInInstance*/, _In_opt_ HINSTANCE /*hPrevInstance*/, _In_ LPSTR /*launchArgs*/, _In_ int /*nCmdShow*/)
{
	return MainImpl();
}
#else
int main(int argc, char* argv[])
{
	return MainImpl();
}
#endif