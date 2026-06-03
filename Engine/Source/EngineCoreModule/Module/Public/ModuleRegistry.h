#pragma once
#include <memory>
#include "CoreDefinitions.h"
#include "Module.h"
#include "Math/Math.h"

namespace LE
{
/**
 * @brief Registry which contains Modules
 */
class ModuleRegistry
{
public:
	/**
	 * @brief Registers Module. Module type can be registered only once. 
	 * Modules can't be unregistered after registration
	 * @tparam Module Module Type which needs to be registered
	 */
	template <ModuleType Module>
	void RegisterModule()
	{
		ModuleTypeId id = ModuleTypeIdGetter<Module>::Value;
		if (Modules.contains(id))
		{
			LE_ERROR("Module: {} is already registered", ModuleTypeIdGetter<Module>::TypeName);
			return;
		}

		std::unique_ptr<Module> newModule = std::make_unique<Module>();
		Modules.emplace(id, std::move(newModule));
	}
	
	template <ModuleType Module>
	Module& GetModule()
	{
		ModuleTypeId id = ModuleTypeIdGetter<Module>::Value;
		if (!Modules.contains(id))
		{
			LE_ERROR("Module: {} is not registered", ModuleTypeIdGetter<Module>::TypeName);
		}
		
		return static_cast<Module&>(*Modules.at(id));
	}
	
	template <ModuleType Module>
	const Module& GetModule() const
	{
		ModuleTypeId id = ModuleTypeIdGetter<Module>::Value;
		if (!Modules.contains(id))
		{
			LE_ERROR("Module: {} is not registered", ModuleTypeIdGetter<Module>::TypeName);
		}
		
		return static_cast<const Module&>(*Modules.at(id));
	}
	
	void RegisterServices() const;
	void RegisterReflection() const;
	void ShutdownServices() const;

private:
	std::unordered_map<ModuleTypeId, std::unique_ptr<ModuleBase>> Modules;
};

ModuleRegistry& GetModuleRegistry();
void RegisterModuleRegistry(ModuleRegistry* Registry);
}
