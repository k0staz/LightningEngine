#pragma once
#include "Module.h"

namespace LE
{
class CoreECSModule : public ModuleBase
{
public:
	void RegisterServices() override;
	void ShutdownServices() override;
	void RegisterReflection() override;

};

REGISTER_MODULE(CoreECSModule, "CoreECSModule")
}
