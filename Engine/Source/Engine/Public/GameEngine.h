#pragma once
#include "IGameEngine.h"
#include "ModuleRegistry.h"
#include "RendererModule.h"
#include "LEWindow.h"
#include "World.h"

namespace LE
{
class SystemWindow;
}

namespace LE
{
class GameEngine : public IGameEngine
{
public:
	void Init();
	void Shutdown();

	void Update(bool& IsDone);
	
	IWorld* GetWorld() override { return GameWorld; }
private:
	void MakeWindow();

	void RegisterModules();

	void InitJobScheduler();

public:
	RefCountingPtr<LEWindow> MainWindow;

	World* GameWorld;
	ServiceRegistry ServiceReg;
	ModuleRegistry ModuleReg;
};

extern GameEngine gGameEngine;
}
