#pragma once
#include "EngineRegistry.h"

namespace LE
{
inline IWorld* GetWorld()
{
	return GetGameEngine()->GetWorld();
}
}
