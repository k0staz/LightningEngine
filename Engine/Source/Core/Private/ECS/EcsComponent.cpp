#include "ECS/EcsComponent.h"

#include "EventCore/EventManager.h"
#include "EventCore/Events/ArchetypeEvents.h"

namespace LE
{
	ComponentTypeId ComponentTypeIdStorage::ComponentTypeCounter {0};

	EcsComponentManager::EcsComponentManager()
	{
	}

	void EcsComponentManager::PostUpdate()
	{
		for (auto& it : EntityArchetypesDirty)
		{
			const EntityId& entityId = it.first;
			const ComponentMask& previousArchetype = it.second;
			const ComponentMask& currentArchetype = EntityArchetypes[entityId];
			
			ComponentMask change = currentArchetype ^ previousArchetype;
			if (change.none())
			{
				continue;
			}

			if ((change & currentArchetype).any())
			{
				std::unique_ptr<ArchetypeMatched> event = std::make_unique<ArchetypeMatched>();
				event->EntityId = entityId;
				event->Archetype = currentArchetype;
				
				QueueCoreEvent(std::move(event));
			}

			if ((change & previousArchetype).any())
			{
				std::unique_ptr<ArchetypeUnmatched> event = std::make_unique<ArchetypeUnmatched>();
				event->EntityId = entityId;
				event->Archetype = previousArchetype;

				QueueCoreEvent(std::move(event));
			}
		}

		for (auto& it : EntityArchetypesChanged)
		{
			std::unique_ptr<ArchetypeChange> event = std::make_unique<ArchetypeChange>();
			event->EntityId = it.first;
			event->Archetype = it.second;

			QueueCoreEvent(std::move(event));
		}

		EntityArchetypesDirty.clear();
		EntityArchetypesChanged.clear();
	}

	// TODO: We should collect all the deleted entities and delete them at the end of the frame
	void EcsComponentManager::OnEntityDeleted(const EntityId& EntityID)
	{
		if (EntityArchetypes.count(EntityID) == 0)
		{
			return;
		}

		const ComponentMask archetype = EntityArchetypes[EntityID];
		if (archetype.none())
		{
			return;
		}

		for (size_t i = 0; i < archetype.size(); ++i)
		{
			if (archetype.test(i))
			{
				ComponentContainers[static_cast<ComponentTypeId>(i)]->DeleteComponent(EntityID);
			}
		}

		EntityArchetypes.erase(EntityID);
		EntityArchetypesDirty.erase(EntityID);
		EntityArchetypesChanged.erase(EntityID);
		ArchetypesToEntities[archetype].erase(EntityID);
	}

	std::unordered_set<EntityId> EcsComponentManager::GetArchetypeMatchedEntities(const ComponentMask& Archetype)
	{
		return ArchetypesToEntities[Archetype];
	}

	EcsComponent::EcsComponent()
		: OwnerEntityId(0)
	{
	}

	EcsComponent::EcsComponent(EntityId OwnerId)
		: OwnerEntityId(OwnerId)
	{
	}
	bool EcsComponent::operator==(const EcsComponent& Other) const noexcept
	{
		return OwnerEntityId == Other.OwnerEntityId;
	}
}
