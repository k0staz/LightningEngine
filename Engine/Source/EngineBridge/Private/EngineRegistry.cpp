#include "EngineRegistry.h"

static LE::IGameEngine* gGameEngine = nullptr;
namespace LE
{
void RegisterEngine(IGameEngine* Engine)
{
	gGameEngine = Engine;
}

IGameEngine* GetGameEngine()
{
	return gGameEngine;
}
}
