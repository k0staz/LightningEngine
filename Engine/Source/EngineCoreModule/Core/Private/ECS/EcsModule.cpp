#include "ECS/EcsModule.h"

namespace LE
{
void ECSModule::Initialize(EcsRegistry<EcsEntity>* InRegistry, EcsSystemRegistry* InSystemRegistry)
{
	Registry = InRegistry;
	SystemRegistry = InSystemRegistry;
}
EcsRegistry<EcsEntity>& ECSModule::GetRegistry()
{
	if (!Registry)
	{
		LE_ASSERT_DESC(false, "Ecs Module is not initialized.");
	}
	
	return *Registry;
}
EcsSystemRegistry& ECSModule::GetSystemRegistry()
{
	if (!SystemRegistry)
	{
		LE_ASSERT_DESC(false, "Ecs Module is not initialized.");
	}
	
	return *SystemRegistry;
}
}

