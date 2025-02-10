#pragma once

#include <cinttypes>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <string>
#include <bitset>

#include "CoreMinimum.h"
#include "ECS/EcsEntity.h"

namespace LE
{
	inline constexpr size_t COMPONENT_NUMBER = 100;
	inline constexpr size_t MAX_COMPONENT_TYPES = 100;

#define INVALID_COMPONENT_TYPE_ID 0

	using ComponentType = std::string;
	using ComponentTypeId = std::uint64_t;
	using ComponentMask = std::bitset<MAX_COMPONENT_TYPES>;


	class EcsComponent
	{
	public:
		EcsComponent();
		EcsComponent(EntityId OwnerId);
		virtual ~EcsComponent() = default;

		EntityId GetOwnerEntityId() const { return OwnerEntityId; }

		bool operator==(const EcsComponent& Other) const noexcept;

	private:
		EntityId OwnerEntityId;
	};

	class ComponentTypeIdStorage
	{
	public:
		static ComponentTypeId ComponentTypeCounter;
	};

	template<class ComponentClass>
	inline ComponentTypeId GetComponentTypeId()
	{
		static ComponentTypeId id = INVALID_COMPONENT_TYPE_ID;
		if (id == INVALID_COMPONENT_TYPE_ID)
		{
			id = ++ComponentTypeIdStorage::ComponentTypeCounter;
		}

		const int maxComponentTypes = MAX_COMPONENT_TYPES;
		LE_ASSERT(id < maxComponentTypes, "Exceeded components type limit {}", maxComponentTypes);

		return id;
	}

#define COMPONENT_CLASS(component_name)                     \
	static std::string GetStaticComponentName()				\
	{														\
		return std::string(component_name);					\
	}														\

	struct IComponentContainer
	{
		virtual ~IComponentContainer() = default;

		virtual void Clear() = 0;
		virtual void DeleteComponent(const EntityId& EntityId) = 0;
	};

	template<class ComponentClass>
	class ComponentContainer final : public IComponentContainer
	{
	public:
		ComponentContainer();
		ComponentContainer(ComponentContainer&) = delete;
		ComponentContainer(ComponentContainer&&) = delete;

		ComponentClass& CreateComponent(const EntityId& EntityId);
		ComponentClass* GetComponent(const EntityId& EntityId) const;
		bool HasComponent(const EntityId& EntityId) const;
		
		void Clear() override;
		void DeleteComponent(const EntityId& EntityId) override;

	private:
		std::vector<ComponentClass> Components;
		std::unordered_map<EntityId, ComponentClass*> LookUpTable;
	};

	class EcsComponentManager
	{
	public:
		EcsComponentManager();
		EcsComponentManager(EcsComponentManager&) = delete;
		EcsComponentManager(EcsComponentManager&&) = delete;

		void PostUpdate();

		template<typename ComponentClass>
		void RegisterComponent();

		template<typename ComponentClass>
		ComponentClass& CreateComponent(const EntityId& EntityId);

		template<typename ComponentClass>
		const ComponentClass* ReadComponent(const EntityId& EntityId) const;

		template<typename ComponentClass>
		ComponentClass* EditComponent(const EntityId& EntityId);

		template<typename ComponentClass>
		bool HasComponent(const EntityId& EntityId) const;
		
		template<typename ComponentClass>
		void DeleteComponent(const EntityId& EntityId);

		void OnEntityDeleted(const EntityId& EntityID);

		std::unordered_set<EntityId> GetArchetypeMatchedEntities(const ComponentMask& Archetype);

	private:
		std::unordered_map<ComponentTypeId, std::shared_ptr<IComponentContainer>> ComponentContainers;
		std::unordered_map<EntityId, ComponentMask> EntityArchetypes;
		std::unordered_map<EntityId, ComponentMask> EntityArchetypesDirty;
		std::unordered_map<EntityId, ComponentMask> EntityArchetypesChanged;
		std::unordered_map<ComponentMask, std::unordered_set<EntityId>> ArchetypesToEntities;
	};

	
	template<class ComponentClass>
	ComponentContainer<ComponentClass>::ComponentContainer()
	{
		Components.reserve(COMPONENT_NUMBER);
		LE_INFO("Component container for type {} was initialized", ComponentClass::GetStaticComponentName().c_str());
	}
	
	template<class ComponentClass>
	ComponentClass& ComponentContainer<ComponentClass>::CreateComponent(const EntityId& EntityId)
	{
		if (HasComponent(EntityId))
		{
			LE_ASSERT(false, "[ComponentContainer] Entity: {} already has a component: {}", EntityId, ComponentClass::GetStaticComponentName().c_str());
		}

		ComponentClass& component = Components.emplace_back(EntityId);
		LookUpTable[EntityId] = &component;

		LE_INFO("Component {} was added to entity with ID: {}", ComponentClass::GetStaticComponentName().c_str(), EntityId);

		return component;
	}

	template<class ComponentClass>
	ComponentClass* ComponentContainer<ComponentClass>::GetComponent(const EntityId& EntityId) const
	{
		auto component = LookUpTable.find(EntityId);
		if (component != LookUpTable.end())
		{
			return component->second;
		}

		return nullptr;
	}

	template<class ComponentClass>
	inline bool ComponentContainer<ComponentClass>::HasComponent(const EntityId& EntityId) const
	{
		auto component = LookUpTable.find(EntityId);

		return component != LookUpTable.end();
	}

	template<class ComponentClass>
	void ComponentContainer<ComponentClass>::Clear()
	{
		Components.clear();
		LookUpTable.clear();
	}

	template<class ComponentClass>
	void ComponentContainer<ComponentClass>::DeleteComponent(const EntityId& EntityId)
	{
		auto component = LookUpTable.find(EntityId);
		if (component != LookUpTable.end())
		{
			auto it = std::find(Components.begin(), Components.end(), *component->second);

			if (it != Components.end())
			{
				LE_INFO("Component {} was deleted from entity with ID: {}", ComponentClass::GetStaticComponentName().c_str(), EntityId);
				Components.erase(it);
			}

			LookUpTable.erase(component);
		}
	}
	
	template<typename ComponentClass>
	void EcsComponentManager::RegisterComponent()
	{
		const ComponentTypeId componentType = GetComponentTypeId<ComponentClass>();
		auto it = ComponentContainers.find(componentType);
		if (it == ComponentContainers.end())
		{
			LE_INFO("Component [{}] was registred", ComponentClass::GetStaticComponentName().c_str());
			ComponentContainers.insert({ componentType, std::make_shared<ComponentContainer<ComponentClass>>() });
		}
		else
		{
			LE_WARN("Component type {} is already registered", ComponentClass::GetStaticComponentName());
		}
	}
	
	template<typename ComponentClass>
	ComponentClass& EcsComponentManager::CreateComponent(const EntityId& EntityId)
	{
		const ComponentTypeId componentType = GetComponentTypeId<ComponentClass>();
		auto it = ComponentContainers.find(componentType);

		LE_ASSERT(it != ComponentContainers.end(), "[ComponentManager] Failed to find container for component type {}", ComponentClass::GetStaticComponentName().c_str());
		auto container = std::static_pointer_cast<ComponentContainer<ComponentClass>>(it->second);
		if (container->HasComponent(EntityId))
		{
			LE_WARN("[ComponentManager] Trying to create a second component [{}] for Entity {}, existing component was returned", ComponentClass::GetStaticComponentName().c_str(), EntityId);

			return *container->GetComponent(EntityId);
		}

		ComponentMask& entityArchetype = EntityArchetypes[EntityId];
		if (EntityArchetypesDirty.count(EntityId) == 0)
		{
			EntityArchetypesDirty[EntityId] = entityArchetype;
		}

		ArchetypesToEntities[entityArchetype].erase(EntityId);

		entityArchetype.set(componentType);

		ArchetypesToEntities[entityArchetype].emplace(EntityId);

		return container->CreateComponent(EntityId);
	}
	
	template<typename ComponentClass>
	const ComponentClass* EcsComponentManager::ReadComponent(const EntityId& EntityId) const
	{
		const ComponentTypeId componentType = GetComponentTypeId<ComponentClass>();
		auto it = ComponentContainers.find(componentType);

		LE_ASSERT(it != ComponentContainers.end(), "[ComponentManager] Failed to find container for component type {}", ComponentClass::GetStaticComponentName().c_str());

		auto container = std::static_pointer_cast<ComponentContainer<ComponentClass>>(it->second);
		return container->GetComponent(EntityId);
	}

	template<typename ComponentClass>
	ComponentClass* EcsComponentManager::EditComponent(const EntityId& EntityId)
	{
		const ComponentTypeId componentType = GetComponentTypeId<ComponentClass>();
		auto it = ComponentContainers.find(componentType);

		LE_ASSERT(it != ComponentContainers.end(), "[ComponentManager] Failed to find container for component type {}", ComponentClass::GetStaticComponentName().c_str());

		ComponentMask& entityArchetype = EntityArchetypes[EntityId];
		if (EntityArchetypesChanged.count(EntityId) == 0)
		{
			EntityArchetypesChanged[EntityId] = entityArchetype;
		}

		auto container = std::static_pointer_cast<ComponentContainer<ComponentClass>>(it->second);
		return container->GetComponent(EntityId);
	}

	template<typename ComponentClass>
	inline bool EcsComponentManager::HasComponent(const EntityId& EntityId) const
	{

		const ComponentTypeId componentType = GetComponentTypeId<ComponentClass>();
		auto it = ComponentContainers.find(componentType);

		LE_ASSERT(it != ComponentContainers.end(), "[ComponentManager] Failed to find container for component type {}", ComponentClass::GetStaticComponentName().c_str());

		auto container = std::static_pointer_cast<ComponentContainer<ComponentClass>>(it->second);
		return container->HasComponent(EntityId);
	}
	
	template<typename ComponentClass>
	void EcsComponentManager::DeleteComponent(const EntityId& EntityId)
	{
		const ComponentTypeId componentType = GetComponentTypeId<ComponentClass>();
		auto it = ComponentContainers.find(componentType);

		LE_ASSERT(it != ComponentContainers.end(), "[ComponentManager] Failed to find container for component type {}", ComponentClass::GetStaticComponentName().c_str());

		ComponentMask& entityArchetype = EntityArchetypes[EntityId];
		if (EntityArchetypesDirty.count(EntityId) == 0)
		{
			EntityArchetypesDirty[EntityId] = entityArchetype;
		}

		ArchetypesToEntities[entityArchetype].erase(EntityId);

		entityArchetype.reset(componentType);

		ArchetypesToEntities[entityArchetype].emplace(EntityId);

		auto container = std::static_pointer_cast<ComponentContainer<ComponentClass>>(it->second);
		container->DeleteComponent(EntityId);
	}
}