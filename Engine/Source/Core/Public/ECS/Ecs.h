#pragma once

#include "GameEngine.h"

namespace LE
{
	static EcsEntity* GetEntityById(EntityId EntityId)
	{
		return gGameEngine.GameWorld->EntityManager.GetEntityById(EntityId);
	}

	static EcsEntity* CreateEntity()
	{
		return gGameEngine.GameWorld->EntityManager.CreateEntity();
	}

	static bool DeleteEntityById(EntityId EntityId)
	{
		if (gGameEngine.GameWorld->EntityManager.DeleteEntityById(EntityId))
		{
			gGameEngine.GameWorld->ComponentManager.OnEntityDeleted(EntityId);
			return true;
		}
		
		return false;
	}

	template<typename ComponentClass>
	static ComponentClass& CreateComponent(const EntityId& EntityId)
	{
		return gGameEngine.GameWorld->ComponentManager.CreateComponent<ComponentClass>(EntityId);
	}

	template<typename ComponentClass>
	static const ComponentClass* ReadComponent(const EntityId& EntityId)
	{
		return gGameEngine.GameWorld->ComponentManager.ReadComponent<ComponentClass>(EntityId);
	}

	template<typename ComponentClass>
	static ComponentClass* EditComponent(const EntityId& EntityId)
	{
		return gGameEngine.GameWorld->ComponentManager.EditComponent<ComponentClass>(EntityId);
	}

	template<typename ComponentClass>
	static bool HasComponent(const EntityId& EntityId)
	{
		return gGameEngine.GameWorld->ComponentManager.HasComponent<ComponentClass>(EntityId);
	}

	template<typename ComponentClass>
	static void DeleteComponent(const EntityId& EntityId)
	{
		gGameEngine.GameWorld->ComponentManager.DeleteComponent<ComponentClass>(EntityId);
	}

	static std::unordered_set<EntityId> GetArchetypeMatchedEntities(const ComponentMask& Archetype)
	{
		return gGameEngine.GameWorld->ComponentManager.GetArchetypeMatchedEntities(Archetype);
	}
}