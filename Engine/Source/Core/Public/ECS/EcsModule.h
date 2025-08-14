#pragma once
#include "EcsRegistry.h"


namespace LE
{
class EcsSystemManager;

class ECSModule
{
public:
	ECSModule() = default;

	void Initialize(EcsRegistry<EcsEntity>* InRegistry, EcsSystemManager* SystemManager);

	EcsRegistry<EcsEntity>* GetRegistry() { return Registry; }
	EcsSystemManager* GetSystemManager() { return SystemManager; }

private:
	EcsRegistry<EcsEntity>* Registry;
	EcsSystemManager* SystemManager;
};
}
