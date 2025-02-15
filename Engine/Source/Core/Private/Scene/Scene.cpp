#include "Scene/Scene.h"

#include "EventCore/EventManager.h"
#include "ECS/Components/ComponentRegistry.h"
#include "ECS/Systems/SystemRegistry.h"

#include "ECS/Components/TransformComponent.h"
#include "ECS/Ecs.h"

#include "System/Public/Time/Clock.h"

namespace LE
{
	static float locCalculateDeltaSeconds()
	{
		static Clock::TimePoint prev = Clock::Now();
		const Clock::TimePoint cur = Clock::Now();
		const float deltaSeconds = Clock::GetSecondsBetween(prev, cur);
		prev = cur;

		return deltaSeconds;
	}

	void Scene::Initialize()
	{
		RegisterComponents(ComponentManager);
		RegisterSystems(SystemManager);

		LE::EcsEntity& entity = *EntityManager.CreateEntity();
		ComponentManager.CreateComponent<LE::TransformComponent>(entity.GetId());
	}

	void Scene::Shutdown()
	{
		SystemManager.Shutdown();
	}

	void Scene::Update()
	{
		const float deltaSeconds = locCalculateDeltaSeconds();
		SystemManager.Update(deltaSeconds);
	}

	void Scene::PostUpdate()
	{
		ComponentManager.PostUpdate();
		gEventManager.DispatchEvents();
	}
}
