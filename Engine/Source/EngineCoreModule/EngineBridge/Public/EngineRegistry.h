#pragma once
#include "IGameEngine.h"

namespace LE
{
	void RegisterEngine(IGameEngine* Engine);
	IGameEngine* GetGameEngine();
}
