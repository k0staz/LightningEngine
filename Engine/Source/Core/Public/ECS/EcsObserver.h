#pragma once
#include "EcsDefinitions.h"
#include "EcsSignals.h"
#include <set>
#include <map>

namespace LE
{
template <typename Iterator, typename Entity>
bool EachButExcludedContainerHas(Iterator FirstContainer, Iterator LastContainer, const Entity EcsEntity,
                                 const std::unordered_map<Entity, std::set<std::size_t>>& ExcludedComponentIndices) noexcept
{
	auto begin = FirstContainer;
	auto end = LastContainer;

	const auto it = ExcludedComponentIndices.find(EcsEntity);
	if (it != ExcludedComponentIndices.end())
	{
		auto first = begin;
		for (std::size_t index : it->second)
		{
			auto last = begin + index;
			if (!EachContainerHas(first, last, EcsEntity))
			{
				return false;
			}

			if (last == end)
			{
				return true;
			}

			first = last + 1;
		}

		return EachContainerHas(first, end, EcsEntity);
	}

	return EachContainerHas(begin, end, EcsEntity);
}

template <typename BaseStorageType, std::size_t ObservedNumber, std::size_t FilteredNumber>
class EcsObserverIterator
{
	using iterator_type = typename std::set<typename BaseStorageType::value_type>::const_iterator;
	using iterator_traits = std::iterator_traits<iterator_type>;

public:
	using value_type = typename iterator_traits::value_type;
	using pointer = typename iterator_traits::pointer;
	using reference = typename iterator_traits::reference;
	using difference_type = typename iterator_traits::difference_type;
	using iterator_category = std::forward_iterator_tag;

	constexpr EcsObserverIterator() noexcept
		: Iterator()
		  , LastIterator()
		  , ObservedComponentStorages()
		  , FilteredComponentStorages()
		  , ExcludedComponentIndices(nullptr)
	{
		Advance();
	}

	EcsObserverIterator(iterator_type First, iterator_type Last, std::array<const BaseStorageType*, ObservedNumber> Storages,
	                    std::array<const BaseStorageType*, FilteredNumber> ExcludedStorages,
	                    const std::unordered_map<value_type, std::set<std::size_t>>* ExcludedIndices)
		: Iterator(First)
		  , LastIterator(Last)
		  , ObservedComponentStorages(Storages)
		  , FilteredComponentStorages(ExcludedStorages)
		  , ExcludedComponentIndices(ExcludedIndices)
	{
		Advance();
	}

	EcsObserverIterator& operator++() noexcept
	{
		++Iterator;
		Advance();
		return *this;
	}

	EcsObserverIterator operator++(int) noexcept
	{
		const EcsObserverIterator current = *this;
		++(*this);
		return current;
	}

	pointer operator->() const noexcept
	{
		return &*Iterator;
	}

	reference operator*() const noexcept
	{
		return *operator->();
	}

	template <typename LhsType, auto... LhsArgs, typename RhsType, auto... RhsArgs>
	friend constexpr bool operator==(const EcsObserverIterator<LhsType, LhsArgs...>&,
	                                 const EcsObserverIterator<RhsType, RhsArgs...>&) noexcept;

private:
	void Advance()
	{
		for (; Iterator != LastIterator && !IsValid(*Iterator); ++Iterator)
		{
		}
	}

	bool IsValid(const value_type Entity) const noexcept
	{
		if (!EachButExcludedContainerHas(ObservedComponentStorages.begin(), ObservedComponentStorages.end(), Entity,
		                                 *ExcludedComponentIndices))
		{
			return false;
		}

		if (FilteredNumber != 0u)
		{
			if (!NoneOfContainersHas(FilteredComponentStorages.begin(), FilteredComponentStorages.end(), Entity))
			{
				return false;
			}
		}

		return true;
	}

private:
	iterator_type Iterator;
	iterator_type LastIterator;
	std::array<const BaseStorageType*, ObservedNumber> ObservedComponentStorages;
	std::array<const BaseStorageType*, FilteredNumber> FilteredComponentStorages;
	const std::unordered_map<value_type, std::set<std::size_t>>* ExcludedComponentIndices;
};

template <typename LhsType, auto... LhsArgs, typename RhsType, auto... RhsArgs>
[[nodiscard]] constexpr bool operator==(const EcsObserverIterator<LhsType, LhsArgs...>& Lhs,
                                        const EcsObserverIterator<RhsType, RhsArgs...>& Rhs) noexcept
{
	return Lhs.Iterator == Rhs.Iterator;
}

template <typename LhsType, auto... LhsArgs, typename RhsType, auto... RhsArgs>
[[nodiscard]] constexpr bool operator!=(const EcsObserverIterator<LhsType, LhsArgs...>& Lhs,
                                        const EcsObserverIterator<RhsType, RhsArgs...>& Rhs) noexcept
{
	return !(Lhs == Rhs);
}

template <typename BaseStorageType, std::size_t ObservedNumber, std::size_t FilteredNumber>
class EcsObserverBase
{
public:
	using base_storage_type = BaseStorageType;
	using entity_type = typename BaseStorageType::value_type;
	using size_type = std::size_t;
	using difference_type = std::ptrdiff_t;
	using iterator = EcsObserverIterator<BaseStorageType, ObservedNumber, FilteredNumber>;

	size_type Count() const noexcept
	{
		return ObservedEntities.size();
	}

	bool IsEmpty() const noexcept
	{
		return Count() == 0u;
	}

	bool Has(entity_type Entity) const noexcept
	{
		if (!ObservedEntities.contains(Entity))
		{
			return false;
		}

		if (!EachButExcludedContainerHas(ObservedComponentStorages.begin(), ObservedComponentStorages.end(), Entity,
		                                 ExcludedComponentIndices))
		{
			return false;
		}

		if (!NoneOfContainersHas(FilteredComponentStorages.begin(), FilteredComponentStorages.end(), Entity))
		{
			return false;
		}

		return true;
	}

	iterator begin() const noexcept
	{
		if (IsEmpty())
		{
			return {};
		}

		return {ObservedEntities.begin(), ObservedEntities.end(), ObservedComponentStorages, FilteredComponentStorages, &ExcludedComponentIndices};
	}

	iterator end() const noexcept
	{
		if (IsEmpty())
		{
			return {};
		}

		return {ObservedEntities.end(), ObservedEntities.end(), ObservedComponentStorages, FilteredComponentStorages, &ExcludedComponentIndices};
	}

	void ResetObservedEntities()
	{
		ObservedEntities.clear();
		ExcludedComponentIndices.clear();
	}

	void Swap(EcsObserverBase& Other)
	{
		std::swap(ObserverType, Other.ObserverType);
		std::swap(ObservedComponentStorages, Other.ObservedComponentStorages);
		std::swap(FilteredComponentStorages, Other.FilteredComponentStorages);
		std::swap(ObservedEntities, Other.ObservedEntities);
		std::swap(ExcludedComponentIndices, Other.ExcludedComponentIndices);
	}

protected:
	EcsObserverBase() noexcept
		: ObserverType(ComponentChangeType::None)
	{
	}

	EcsObserverBase(ComponentChangeType InType, std::array<const BaseStorageType*, ObservedNumber> ObservedStorages,
	                std::array<const BaseStorageType*, FilteredNumber> FilteredStorages)
		: ObserverType(InType)
		  , ObservedComponentStorages(ObservedStorages)
		  , FilteredComponentStorages(FilteredStorages)
	{
	}

	const BaseStorageType* GetComponentStorageAt(const size_type Index) const noexcept
	{
		return ObservedComponentStorages[Index];
	}

protected:
	ComponentChangeType ObserverType;
	std::array<const BaseStorageType*, ObservedNumber> ObservedComponentStorages;
	std::array<const BaseStorageType*, FilteredNumber> FilteredComponentStorages;
	std::set<entity_type> ObservedEntities;
	std::unordered_map<entity_type, std::set<size_type>> ExcludedComponentIndices;
};

template <typename, typename>
class EcsObserver;

template <typename... Components, typename... ExcludedComponents>
class EcsObserver<IncludedComponentTypes<Components...>, ExcludedComponentTypes<ExcludedComponents...>>
	: public EcsObserverBase<std::common_type_t<typename Components::base_type...>, sizeof...(Components), sizeof...(ExcludedComponents)>
{
	using base_type = EcsObserverBase<std::common_type_t<typename Components::base_type...>, sizeof...(Components), sizeof...(
		                                  ExcludedComponents)>;

	template <std::size_t Index>
	using StorageTypeAt = ComponentStorageTypeAtIndex<Index, ComponentTypeList<Components..., ExcludedComponents...>>;

public:
	using common_type = typename base_type::base_storage_type;
	using entity_type = typename base_type::entity_type;
	using size_type = typename base_type::size_type;
	using difference_type = std::ptrdiff_t;
	using iterator = typename base_type::iterator;

	EcsObserver() noexcept = default;

	EcsObserver(ComponentChangeType InType, Components&... ComponentsIn, ExcludedComponents&... Excluded) noexcept
		: base_type(InType, {&ComponentsIn...}, {&Excluded...})
	{
		SubscribeToComponentChanges();
	}

	EcsObserver(const EcsObserver&) = delete;

	EcsObserver(EcsObserver&& Other) noexcept
	{
		base_type::Swap(Other);
		SubscribeToComponentChanges();
	}

	EcsObserver& operator=(const EcsObserver&) = delete;

	EcsObserver& operator=(EcsObserver&& Other) noexcept
	{
		UnsubscribeFromComponentChanges();
		base_type::Swap(Other);
		SubscribeToComponentChanges();
		return *this;
	}

	~EcsObserver()
	{
		UnsubscribeFromComponentChanges();
	}

	template <typename ComponentType>
	auto* GetComponentStorage() const noexcept
	{
		return GetComponentStorage<ComponentStorageIndex<ComponentType>>();
	}

	template <size_type Index>
	auto* GetComponentStorage() const noexcept
	{
		return static_cast<StorageTypeAt<Index>*>(const_cast<TransferConstnessType<common_type, StorageTypeAt<Index>>*>(
			base_type::GetComponentStorageAt(Index)));
	}

	template <typename ComponentType, typename... OtherComponentTypes>
	decltype(auto) GetComponents(const entity_type Entity) const
	{
		return GetComponents<ComponentStorageIndex<ComponentType>, ComponentStorageIndex<OtherComponentTypes>...>(Entity);
	}

	template <size_type... Index>
	decltype(auto) GetComponents(const entity_type Entity) const
	{
		if constexpr (sizeof...(Index) == 1)
		{
			return (GetComponentStorage<Index>()->GetComponent(Entity), ...);
		}
		else
		{
			return std::tuple_cat(GetComponentStorage<Index>()->GetComponentAsTuple(Entity)...);
		}
	}

	template <std::size_t Index>
	void OnStorageChange(const entity_type Entity)
	{
		this->ObservedEntities.insert(Entity);
		if (this->ObserverType == ComponentChangeType::ComponentRemoved)
		{
			this->ExcludedComponentIndices[Entity].insert(Index);
		}
	}

	void OnFromStorageRemoved(const entity_type Entity)
	{
		this->ObservedEntities.erase(Entity);
	}

private:
	template <std::size_t Index>
	void SubscribeStorage() noexcept
	{
		ComponentsSinks[Index].template Attach<&EcsObserver::OnStorageChange<Index>>(this);
	}

	template <std::size_t Index>
	void UnsubscribeStorage() noexcept
	{
		ComponentsSinks[Index].template Detach<&EcsObserver::OnStorageChange<Index>>(this);
	}

	void SubscribeToComponentChanges() noexcept
	{
		switch (this->ObserverType)
		{
		case ComponentChangeType::ComponentAdded:
			((ComponentsSinks.emplace_back((*GetComponentStorage<typename Components::value_type>()).GetOnAddedSink())),
				...);
			break;
		case ComponentChangeType::ComponentRemoved:
			((ComponentsSinks.emplace_back((*GetComponentStorage<typename Components::value_type>()).GetOnRemovedSink()))
				, ...);
			break;
		case ComponentChangeType::ComponentUpdated:
			((ComponentsSinks.emplace_back((*GetComponentStorage<typename Components::value_type>()).GetOnUpdatedSink()))
				, ...);
			break;
		case ComponentChangeType::None:
			LE_ASSERT_DESC(false, "Uninitialized Ecs Observer")
			break;
		}

		((SubscribeStorage<ComponentStorageIndex<typename Components::value_type>>()), ...);

		if (this->ObserverType != ComponentChangeType::ComponentRemoved)
		{
			((RemovedSinks.emplace_back((*GetComponentStorage<typename Components::value_type>()).GetOnRemovedSink())),
				...);
			for (auto& sink : RemovedSinks)
			{
				sink.template Attach<&EcsObserver::OnFromStorageRemoved>(this);
			}
		}
	}

	void UnsubscribeFromComponentChanges() noexcept
	{
		if (!ComponentsSinks.empty())
		{
			((UnsubscribeStorage<ComponentStorageIndex<typename Components::value_type>>()), ...);
			ComponentsSinks.clear();
		}

		if (this->ObserverType != ComponentChangeType::ComponentRemoved)
		{
			for (auto& sink : RemovedSinks)
			{
				sink.template Detach<&EcsObserver::OnFromStorageRemoved>(this);
			}
			RemovedSinks.clear();
		}
	}

private:
	template <typename ComponentType>
	static constexpr size_type ComponentStorageIndex = ComponentIndexInList<
		ComponentType, ComponentTypeList<typename Components::value_type...>>;

	std::vector<Sink<Signal<void(const entity_type)>>> ComponentsSinks;
	std::vector<Sink<Signal<void(const entity_type)>>> RemovedSinks;
};
}
