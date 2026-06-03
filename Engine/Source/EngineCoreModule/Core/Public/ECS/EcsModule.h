#pragma once
#include "EcsRegistry.h"


namespace LE
{
class EcsSystemRegistry;

class ECSModule
{
public:
	ECSModule() = default;

	void Initialize(EcsRegistry<EcsEntity>* InRegistry, EcsSystemRegistry* InSystemRegistry);

	EcsRegistry<EcsEntity>& GetRegistry();
	EcsSystemRegistry& GetSystemRegistry();

private:
	EcsRegistry<EcsEntity>* Registry;
	EcsSystemRegistry* SystemRegistry;
};
}
