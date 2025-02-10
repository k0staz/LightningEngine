#pragma once

#include "Scene.h"

namespace LE
{
	class SceneManager
	{
	public:
		void LoadCurrentScene();
		void UnloadCurrentScene();
		
		void Update();
		void PostUpdate();

		friend EcsEntity* GetEntityById(EntityId EntityId);
		friend EcsEntity* CreateEntity();
		friend bool DeleteEntityById(EntityId EntityId);

		template<typename ComponentClass>
		friend ComponentClass& CreateComponent(const EntityId& EntityId);

		template<typename ComponentClass>
		friend const ComponentClass* ReadComponent(const EntityId& EntityId);

		template<typename ComponentClass>
		friend ComponentClass* EditComponent(const EntityId& EntityId);

		template<typename ComponentClass>
		friend bool HasComponent(const EntityId& EntityId);

		template<typename ComponentClass>
		friend void DeleteComponent(const EntityId& EntityId);

		friend std::unordered_set<EntityId> GetArchetypeMatchedEntities(const ComponentMask& Archetype);
	protected:
		Scene CurrentScene;
	};

	extern SceneManager gSceneManager;
}
