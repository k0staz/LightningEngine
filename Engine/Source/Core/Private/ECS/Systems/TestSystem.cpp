#include "ECS/Systems/TestSystem.h"

#include "ECS/Components/TransformComponent.h"
#include "ECS/Ecs.h"

namespace LE
{
	void TestSystem::Initialize()
	{
		/*ComponentMask Archetype;
		Archetype.set(GetComponentTypeId<TransformComponent>());

		ArchetypeMatchListener = [this](const ArchetypeMatched& Event) { OnArchetypeMatched(Event); };
		ArchetypeUnmatchListener = [this](const ArchetypeUnmatched& Event) { OnArchetypeUnmatched(Event); };

		gEventManager.ListenToArchetypeMatchedEvent(Archetype, ArchetypeMatchListener);
		gEventManager.ListenToArchetypeUnmatchedEvent(Archetype, ArchetypeUnmatchListener);*/

		TestObserver = ObserverComponents<TransformComponent>(ComponentChangeType::ComponentAdded);
		
	}

	void TestSystem::Update(const float DeltaSeconds)
	{
		static float time = 0.0f;

		time += DeltaSeconds;
		if (time > TWO_PI)
		{
			time = 0.0f;
		}

		auto view = ViewComponents<TransformComponent>();
		for (auto entity : view)
		{
			TransformComponent& transformComponent = view.GetComponents<TransformComponent>(entity);

			Vector3F pos = transformComponent.Transform.GetPosition();

			pos.X = Sin(time);
			pos.Y = Sin(time);
			pos.Z = Sin(time);

			transformComponent.Transform.Translate(pos);

			LE_INFO("Change transform to X {} Y {} Z {} for Entity {}", pos.X, pos.Y, pos.Z, entity);
		}

		for (auto entity : TestObserver)
		{
			LE_INFO("Observed change for entity: {}", entity);
		}
		TestObserver.ClearObserverEntities();

	}

	void TestSystem::Shutdown()
	{
		
	}

	/*void TestSystem::OnArchetypeMatched(const ArchetypeMatched& Event)
	{
		LE_INFO("Event Fired for Entity {}", Event.EntityId);
	}

	void TestSystem::OnArchetypeUnmatched(const ArchetypeUnmatched& Event)
	{
		LE_INFO("Event Fired for Entity {}", Event.EntityId);
	}*/
}