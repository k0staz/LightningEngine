#pragma once

#include "ECS/EcsEntity.h"
#include "ECS/EcsComponent.h"
#include "ECS/EcsSystem.h"

namespace LE
{
	class SceneManager;

	class Scene
	{
		friend SceneManager;
	public:
		void Initialize();
		void Shutdown();

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

	private:
		EcsEntityManager EntityManager;
		EcsComponentManager ComponentManager;
		EcsSystemManager SystemManager;
	};
}
