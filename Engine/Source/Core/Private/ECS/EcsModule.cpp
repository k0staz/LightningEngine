#include "ECS/EcsModule.h"

namespace LE
{
void ECSModule::Initialize(EcsRegistry<EcsEntity>* InRegistry, EcsSystemManager* InSystemManager)
{
	Registry = InRegistry;
	SystemManager = InSystemManager;
}
}

