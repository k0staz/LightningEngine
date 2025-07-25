#include "ECS/EcsModule.h"

namespace LE
{
void ECSModule::Initialize(EcsEntityManager* InEntityManager, EcsComponentManager* InComponentManager, EcsSystemManager* InSystemManager)
{
	EntityManager = InEntityManager;
	ComponentManager = InComponentManager;
	SystemManager = InSystemManager;
}
}

