#pragma once
#include "RendererModule.h"
#include "World.h"

namespace LE
{
class SystemWindow;
}

namespace LE
{
class GameViewport;
}

namespace LE
{
class GameEngine
{
public:
	void Init();
	void Shutdown();

	void Update(bool& IsDone);

private:
	// This will need to be moved to a separate Platform specific application class
	void MakeWindow();

	void DrawViewport();

	void InitMaterials();

public:
	RefCountingPtr<SystemWindow> Window;

	World* GameWorld;
	GameViewport* Viewport;
	Renderer::RendererModule RendererModule;
};

extern GameEngine gGameEngine;
}
