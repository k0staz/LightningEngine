#pragma once

#include "Scene/SceneManager.h"

namespace LE
{
	static EcsEntity* GetEntityById(EntityId EntityId)
	{
		return gSceneManager.CurrentScene.EntityManager.GetEntityById(EntityId);
	}

	static EcsEntity* CreateEntity()
	{
		return gSceneManager.CurrentScene.EntityManager.CreateEntity();
	}

	static bool DeleteEntityById(EntityId EntityId)
	{
		if (gSceneManager.CurrentScene.EntityManager.DeleteEntityById(EntityId))
		{
			gSceneManager.CurrentScene.ComponentManager.OnEntityDeleted(EntityId);
			return true;
		}
		
		return false;
	}

	template<typename ComponentClass>
	static ComponentClass& CreateComponent(const EntityId& EntityId)
	{
		return gSceneManager.CurrentScene.ComponentManager.CreateComponent<ComponentClass>(EntityId);
	}

	template<typename ComponentClass>
	static const ComponentClass* ReadComponent(const EntityId& EntityId)
	{
		return gSceneManager.CurrentScene.ComponentManager.ReadComponent(EntityId);
	}

	template<typename ComponentClass>
	static ComponentClass* EditComponent(const EntityId& EntityId)
	{
		return gSceneManager.CurrentScene.ComponentManager.EditComponent<ComponentClass>(EntityId);
	}

	template<typename ComponentClass>
	static bool HasComponent(const EntityId& EntityId)
	{
		return gSceneManager.CurrentScene.ComponentManager.HasComponent<ComponentClass>(EntityId);
	}

	template<typename ComponentClass>
	static void DeleteComponent(const EntityId& EntityId)
	{
		gSceneManager.CurrentScene.ComponentManager.DeleteComponent<ComponentClass>(EntityId);
	}

	static std::unordered_set<EntityId> GetArchetypeMatchedEntities(const ComponentMask& Archetype)
	{
		return gSceneManager.CurrentScene.ComponentManager.GetArchetypeMatchedEntities(Archetype);
	}
}