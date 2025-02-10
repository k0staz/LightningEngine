#include "ECS/Systems/SystemRegistry.h"

#include "ECS/Systems/TestSystem.h"

namespace LE
{
	void RegisterSystems(EcsSystemManager& SystemManager)
	{
		SystemManager.RegisterSystem<TestSystem>();
	}
}