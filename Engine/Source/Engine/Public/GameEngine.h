#pragma once
#include "IGameEngine.h"
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
class GameEngine : public IGameEngine
{
public:
	void Init();
	void Shutdown();

	void Update(bool& IsDone);

	// IGameEngine
	Renderer::RendererModule* GetRendererModule() override { return &RendererModule; }
	IWorld* GetWorld() override { return GameWorld; }

	void DrawFrame(const float);

private:
	// This will need to be moved to a separate Platform specific application class
	void MakeWindow();

	void DrawViewport();

	void InitMaterials();

	void InitJobScheduler();

public:
	RefCountingPtr<SystemWindow> Window;

	World* GameWorld;
	GameViewport* Viewport;
	Renderer::RendererModule RendererModule;
};

extern GameEngine gGameEngine;
}
