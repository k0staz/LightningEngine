#include "Scene/Scene.h"

#include "EventCore/EventManager.h"
#include "ECS/Components/ComponentRegistry.h"
#include "ECS/Systems/SystemRegistry.h"

#include "ECS/Components/TransformComponent.h"
#include "ECS/Ecs.h"

namespace LE
{
	void Scene::Initialize()
	{
		RegisterComponents(ComponentManager);
		RegisterSystems(SystemManager);

		LE::EcsEntity& entity = *EntityManager.CreateEntity();

		ComponentManager.CreateComponent<LE::TransformComponent>(entity.GetId());
		ComponentManager.EditComponent<LE::TransformComponent>(entity.GetId());
		//ComponentManager.DeleteComponent<LE::TransformComponent>(entity.GetId());

		//EntityManager.GetEntityById(entity.GetId());
		DeleteEntityById(entity.GetId());
	}

	void Scene::Shutdown()
	{
		SystemManager.Shutdown();
	}

	void Scene::Update()
	{
		SystemManager.Update();
	}

	void Scene::PostUpdate()
	{
		ComponentManager.PostUpdate();
		gEventManager.DispatchEvents();
	}
}
