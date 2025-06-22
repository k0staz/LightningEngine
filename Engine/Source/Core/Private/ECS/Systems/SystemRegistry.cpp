#include "ECS/Systems/SystemRegistry.h"

#include "ECS/Systems/TestSystem.h"
#include "Systems/RenderSystem.h"

namespace LE
{
	void RegisterSystems(EcsSystemManager& SystemManager)
	{
		SystemManager.RegisterSystem<TestSystem>();
		SystemManager.RegisterSystem<RenderSystem>();
	}
}
