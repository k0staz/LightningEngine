#include "ECS/Components/ComponentRegistry.h"

/*#include "Components/CameraComponent.h"
#include "Components/StaticMeshComponent.h"*/
#include "ECS/Components/TransformComponent.h"

namespace LE
{
	void RegisterComponents(EcsComponentManager& ComponentManager)
	{
		ComponentManager.RegisterComponent<LE::TransformComponent>();
		/*ComponentManager.RegisterComponent<LE::StaticMeshComponent>();
		ComponentManager.RegisterComponent<LE::CameraComponent>();*/
	}
}