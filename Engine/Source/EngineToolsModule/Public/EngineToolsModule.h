#pragma once
#include "Module.h"

namespace LE
{
class EngineToolsModule : public ModuleBase
{
public:
	void RegisterServices() override;
	void ShutdownServices() override;
	void RegisterReflection() override;

};

REGISTER_MODULE(EngineToolsModule, "EngineToolsModule")
}
