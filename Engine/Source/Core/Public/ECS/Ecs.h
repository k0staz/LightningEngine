#pragma once

#include "CoreDefinitions.h"
#include "EcsComponent.h"
#include "EcsEntity.h"
#include "EcsModule.h"

namespace LE
{
	void RegisterECSModule(UniquePtr<ECSModule> Module);
	ECSModule& GetECSModule();

	static EcsEntity* GetEntityById(EntityId EntityId)
	{
		return GetECSModule().GetEntityManager()->GetEntityById(EntityId);
	}

	static EcsEntity* CreateEntity()
	{
		return  GetECSModule().GetEntityManager()->CreateEntity();
	}

	static bool DeleteEntityById(EntityId EntityId)
	{
		if (GetECSModule().GetEntityManager()->DeleteEntityById(EntityId))
		{
			GetECSModule().GetComponentManager()->OnEntityDeleted(EntityId);
			return true;
		}
		
		return false;
	}

	template<typename ComponentClass>
	static ComponentClass& CreateComponent(const EntityId& EntityId)
	{
		return GetECSModule().GetComponentManager()->CreateComponent<ComponentClass>(EntityId);
	}

	template<typename ComponentClass>
	static const ComponentClass* ReadComponent(const EntityId& EntityId)
	{
		return GetECSModule().GetComponentManager()->ReadComponent<ComponentClass>(EntityId);
	}

	template<typename ComponentClass>
	static ComponentClass* EditComponent(const EntityId& EntityId)
	{
		return GetECSModule().GetComponentManager()->EditComponent<ComponentClass>(EntityId);
	}

	template<typename ComponentClass>
	static bool HasComponent(const EntityId& EntityId)
	{
		return GetECSModule().GetComponentManager()->HasComponent<ComponentClass>(EntityId);
	}

	template<typename ComponentClass>
	static void DeleteComponent(const EntityId& EntityId)
	{
		GetECSModule().GetComponentManager()->DeleteComponent<ComponentClass>(EntityId);
	}

	static std::unordered_set<EntityId> GetArchetypeMatchedEntities(const ComponentMask& Archetype)
	{
		return GetECSModule().GetComponentManager()->GetArchetypeMatchedEntities(Archetype);
	}
}