#pragma once
#include "IWorld.h"

namespace LE
{

class IGameEngine
{
public:
	virtual IWorld* GetWorld() = 0;
};
}
