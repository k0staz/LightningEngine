#pragma once
#include <algorithm>

#include "EcsStorageView.h"
#include "Containers/ECSStorage.h"
#include "Containers/SparseSet.h"

namespace LE
{
template <typename Entity>
class EcsRegistry
{
	using Traits = EcsEntityTraits<Entity>;

public:
	using size_type = std::size_t;

	EcsRegistry()
		: EcsRegistry(0)
	{
	}

	EcsRegistry(const size_type ComponentTypeCount)
	{
		ComponentStorages.reserve(ComponentTypeCount);
	}

	EcsRegistry(const EcsRegistry&) = delete;

	EcsRegistry(EcsRegistry&& Other) noexcept
		: EntityStorage(std::move(Other.EntityStorage))
		  , ComponentStorages(std::move(Other.ComponentStorages))
	{
	}

	~EcsRegistry() = default;

	EcsRegistry& operator=(const EcsRegistry&) = delete;

	EcsRegistry& operator=(EcsRegistry&& Other) noexcept
	{
		Swap(Other);
		return *this;
	}

	void Swap(EcsRegistry& Other) noexcept
	{
		std::swap(EntityStorage, Other.EntityStorage);
		std::swap(ComponentStorages, Other.ComponentStorages);
	}

	bool IsEntityValid(const Entity EcsEntity)
	{
		return EntityStorage.Has(EcsEntity);
	}

	Entity CreateEntity()
	{
		return EntityStorage.CreateEntity();
	}

	void DeleteEntity(const Entity EcsEntity)
	{
		LE_ASSERT_DESC(IsEntityValid(EcsEntity), "Attempting to delete an invalid Entity")
		for (auto& storage : ComponentStorages)
		{
			storage.second->Delete(EcsEntity);
		}
		return EntityStorage.Delete(EcsEntity);
	}

	template <typename ComponentType, typename... ComponentArgs>
	ComponentType& AddComponentToEntity(const Entity EcsEntity, ComponentArgs&&... Args)
	{
		LE_ASSERT_DESC(IsEntityValid(EcsEntity), "Attempting to add component to an invalid Entity")
		return GetCreateComponentStorage<ComponentType>().CreateComponent(EcsEntity, std::forward<ComponentArgs>(Args)...);
	}

	template <typename ComponentType, typename EntityIterator>
	void CreateComponent(EntityIterator FirstEntity, EntityIterator LastEntity, const ComponentType& Component = {})
	{
		LE_ASSERT_DESC(std::all_of(FirstEntity, LastEntity, [this](const Entity EcsEntity) { return IsEntityValid(EcsEntity); }),
		               "Attempting to add component to an invalid Entity")

		GetCreateComponentStorage<ComponentType>().CreateComponent(FirstEntity, LastEntity, Component);
	}

	template <typename ComponentType, typename EntityIterator, typename ComponentIterator, typename = std::enable_if<std::is_same_v<
		          typename std::iterator_traits<ComponentIterator>::value_type, ComponentType>>>
	void CreateComponents(EntityIterator FirstEntity, EntityIterator LastEntity, ComponentIterator FirstComponent)
	{
		LE_ASSERT_DESC(std::all_of(FirstEntity, LastEntity, [this](const Entity EcsEntity) { return IsEntityValid(EcsEntity); }),
		               "Attempting to add component to an invalid Entity")

		GetCreateComponentStorage<ComponentType>().CreateComponent(FirstEntity, LastEntity, FirstComponent);
	}

	template <typename ComponentType, typename... ComponentArgs>
	ComponentType& AddReplaceComponentToEntity(const Entity EcsEntity, ComponentArgs&&... Args)
	{
		LE_ASSERT_DESC(IsEntityValid(EcsEntity), "Attempting to add component to an invalid Entity")

		EcsComponentStorage<ComponentType, Entity>& storage = GetCreateComponentStorage<ComponentType>();
		if (storage.Has(EcsEntity))
		{
			return storage.RunOnComponent([&Args...](auto&... current)
			{
				((current = ComponentType{std::forward<ComponentArgs>(Args)...}), ...);
			});
		}
		else
		{
			return storage.CreateComponent(EcsEntity, std::forward<ComponentArgs>(Args)...);
		}
	}

	template <typename ComponentType, typename... Func>
	ComponentType& RunOnComponent(const Entity EcsEntity, Func&&... InFunc)
	{
		LE_ASSERT_DESC(IsEntityValid(EcsEntity), "Invalid Entity")
		return GetCreateComponentStorage<ComponentType>().RunOnComponent(EcsEntity, std::forward<Func>(InFunc)...);
	}

	template <typename ComponentType, typename... OtherComponents>
	void DeleteComponent(const Entity EcsEntity)
	{
		(GetCreateComponentStorage<ComponentType>().Delete(EcsEntity), (GetCreateComponentStorage<OtherComponents>().Delete(EcsEntity), ...
		));
	}

	template <typename... ComponentType>
	bool HasAllComponents(const Entity EcsEntity) const
	{
		if constexpr (sizeof...(ComponentType) == 1u)
		{
			auto* storage = GetComponentStorage<ComponentType...>();
			return storage && storage->Has(EcsEntity);
		}

		return (HasAllComponents<ComponentType>(EcsEntity) && ...);
	}

	template <typename... ComponentType>
	bool HasAnyComponents(const Entity EcsEntity) const
	{
		return (HasAllComponents<ComponentType>() || ...);
	}

	template <typename... ComponentType>
	decltype(auto) GetComponent(const Entity EcsEntity) const
	{
		if constexpr (sizeof...(ComponentType) == 1u)
		{
			return (GetComponentStorage<ComponentType>()->GetComponent(EcsEntity), ...);
		}

		return std::forward_as_tuple(GetComponent<ComponentType>(EcsEntity)...);
	}

	template <typename... ComponentType, typename... ExcludedComponents>
	EcsStorageView<IncludedComponentTypes<EcsComponentStorage<ComponentType, Entity>...>, ExcludedComponentTypes<EcsComponentStorage<
		               ExcludedComponents, Entity>...>>
	View(ExcludedComponentTypes<ExcludedComponents...>  = ExcludedComponentTypes{})
	{
		return { GetCreateComponentStorage<ComponentType>()..., GetCreateComponentStorage<ExcludedComponents>()... };
	}

private:
	template <typename ComponentType>
	EcsComponentStorage<ComponentType, Entity>& GetCreateComponentStorage(
		EcsComponentType ComponentTypeId = ComponentTypeIdGetter<ComponentType>::Value)
	{
		static_assert(!std::is_same_v<ComponentType, Entity>, "Attempting to pass Entity as Component");
		using ComponentStorageType = EcsComponentStorage<ComponentType, Entity>;

		auto it = ComponentStorages.find(ComponentTypeId);
		if (it != ComponentStorages.cend())
		{
			return static_cast<ComponentStorageType&>(*it->second);
		}

		std::shared_ptr<SparseSet<Entity>> storage = std::make_shared<ComponentStorageType>();
		ComponentStorages.emplace(ComponentTypeId, storage);

		return static_cast<ComponentStorageType&>(*storage);
	}

	template <typename ComponentType>
	const EcsComponentStorage<ComponentType, Entity>& GetComponentStorage(
		EcsComponentType ComponentTypeId = ComponentTypeIdGetter<ComponentType>::Value) const
	{
		static_assert(!std::is_same_v<ComponentType, Entity>, "Attempting to pass Entity as Component");
		using ComponentStorageType = EcsComponentStorage<ComponentType, Entity>;

		auto it = ComponentStorages.find(ComponentTypeId);
		if (it != ComponentStorages.cend())
		{
			return static_cast<ComponentStorageType&>(*it->second);
		}

		return nullptr;
	}

private:
	EcsEntityStorage<Entity> EntityStorage;
	std::unordered_map<EcsComponentType, std::shared_ptr<SparseSet<Entity>>> ComponentStorages; // Stores pointers to EcsComponentStorage
};
}
