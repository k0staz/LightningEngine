#pragma once
#include "IGameEngine.h"
#include "ModuleRegistry.h"
#include "RendererModule.h"
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
	// This will need to be moved to a separate Platform specific application class
	void MakeWindow();

	void RegisterModules();
	
	void InitMaterials();

	void InitJobScheduler();

public:
	RefCountingPtr<SystemWindow> Window;

	World* GameWorld;
	ServiceRegistry ServiceReg;
	ModuleRegistry ModuleReg;
};

extern GameEngine gGameEngine;
}
