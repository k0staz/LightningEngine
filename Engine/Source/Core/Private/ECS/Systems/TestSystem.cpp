#include "ECS/Systems/TestSystem.h"

#include "ECS/Components/TransformComponent.h"
#include "ECS/Ecs.h"

namespace LE
{
	void TestSystem::Initialize()
	{
		ComponentMask Archetype;
		Archetype.set(GetComponentTypeId<TransformComponent>());

		ArchetypeMatchListener = [this](const ArchetypeMatched& Event) { OnArchetypeMatched(Event); };
		ArchetypeUnmatchListener = [this](const ArchetypeUnmatched& Event) { OnArchetypeUnmatched(Event); };
		ArchetypeChangeListener = [this](const ArchetypeChange& Event) { OnArchetypeChanged(Event); };

		gEventManager.ListenToArchetypeMatchedEvent(Archetype, ArchetypeMatchListener);
		gEventManager.ListenToArchetypeUnmatchedEvent(Archetype, ArchetypeUnmatchListener);
		gEventManager.ListenToArchetypeChangeEvent(Archetype, ArchetypeChangeListener);
		
	}

	void TestSystem::Update()
	{
		ComponentMask Archetype;
		Archetype.set(GetComponentTypeId<TransformComponent>());

		auto Entities = GetArchetypeMatchedEntities(Archetype);
		for (const EntityId& it : Entities)
		{
			DeleteComponent<TransformComponent>(it);
		}

	}

	void TestSystem::Shutdown()
	{
		
	}

	void TestSystem::OnArchetypeMatched(const ArchetypeMatched& Event)
	{
		LE_INFO("Event Fired for Entity {}", Event.EntityId);
	}

	void TestSystem::OnArchetypeUnmatched(const ArchetypeUnmatched& Event)
	{
		LE_INFO("Event Fired for Entity {}", Event.EntityId);
	}

	void TestSystem::OnArchetypeChanged(const ArchetypeChange& Event)
	{
		LE_INFO("Event Fired for Entity {}", Event.EntityId);
	}
}