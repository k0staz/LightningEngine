#pragma once

#include "GameEngine.h"

#include "Time/Clock.h"

#if PLATFORM_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

#ifdef DEBUG
#include "tracy/Tracy.hpp"

const int TRACY_CALLSTACK_DEPTH = 16;

void* operator new(std::size_t count) {
	auto ptr = std::malloc(count);
	if (!ptr) throw std::bad_alloc();

	TracyAllocS(ptr, count, TRACY_CALLSTACK_DEPTH);
	return ptr;
}

void operator delete(void* ptr) noexcept {
	if (ptr) {
		TracyFreeS(ptr, TRACY_CALLSTACK_DEPTH);
		std::free(ptr);
	}
}

void* operator new[](std::size_t count) {
	auto ptr = std::malloc(count);
	if (!ptr) throw std::bad_alloc();

	TracyAllocS(ptr, count, TRACY_CALLSTACK_DEPTH);
	return ptr;
}

void operator delete[](void* ptr) noexcept {
	if (ptr) {
		TracyFreeS(ptr, TRACY_CALLSTACK_DEPTH);
		std::free(ptr);
	}
}
#endif

int MainImpl()
{
	Log::Initialize();

	LE::gGameEngine.Init();

	LE::gGlobalClock.Start();

	bool isDone = false;
	while (!isDone)
	{
		LE::gGameEngine.Update(isDone);
	}

	LE::gGameEngine.Shutdown();

	return 0;
}

#if PLATFORM_WINDOWS
int WINAPI WinMain(_In_ HINSTANCE /*hInInstance*/, _In_opt_ HINSTANCE /*hPrevInstance*/, _In_ LPSTR /*launchArgs*/, _In_ int /*nCmdShow*/)
{
	//_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	//_CrtSetBreakAlloc(2747);
	//_CrtSetBreakAlloc(2746);
	//_CrtSetBreakAlloc(28168);

	return MainImpl();
}
#else
int main(int argc, char* argv[])
{
	return MainImpl();
}
#endif
