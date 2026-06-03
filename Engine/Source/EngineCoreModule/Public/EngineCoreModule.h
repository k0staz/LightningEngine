#pragma once
#include "Module.h"

namespace LE
{
/**
 * @brief Engine Core Module handling core reflection types and services registration
 */
class EngineCoreModule : public ModuleBase 
{
public:
	void RegisterServices() override;
	void ShutdownServices() override;
	void RegisterReflection() override;

};

REGISTER_MODULE(EngineCoreModule, "EngineCoreModule")
}
