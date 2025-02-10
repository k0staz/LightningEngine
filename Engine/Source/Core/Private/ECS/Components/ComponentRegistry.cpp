#include "ECS/Components/ComponentRegistry.h"

#include "ECS/Components/TransformComponent.h"

namespace LE
{
	void RegisterComponents(EcsComponentManager& ComponentManager)
	{
		ComponentManager.RegisterComponent<LE::TransformComponent>();
	}
}