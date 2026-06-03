#include "ModuleRegistry.h"

#include <ranges>

namespace LE
{

static ModuleRegistry* gModuleRegistry = nullptr;

void ModuleRegistry::RegisterServices() const
{
	for (auto& val : Modules | std::views::values)
	{
		val->RegisterServices();
	}
}

void ModuleRegistry::RegisterReflection() const
{
	for (auto& val : Modules | std::views::values)
	{
		val->RegisterReflection();
	}
}

void ModuleRegistry::ShutdownServices() const
{
	for (auto& val : Modules | std::views::values)
	{
		val->ShutdownServices();
	}
}

ModuleRegistry& GetModuleRegistry()
{
	if (!gModuleRegistry)
	{
		LE_ASSERT_DESC(false, "Module Registry is not initialized")
	}

	return *gModuleRegistry;
}

void RegisterModuleRegistry(ModuleRegistry* Registry)
{
	if (gModuleRegistry)
	{
		LE_ASSERT_DESC(false, "Module Registry is already registered")
	}
	
	gModuleRegistry = Registry;
}
}
