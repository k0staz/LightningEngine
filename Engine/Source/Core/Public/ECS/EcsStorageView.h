#pragma once

#include "CoreMinimum.h"
#include "Containers/ECSStorage.h"

#include <array>

namespace LE
{
template <typename Iterator, typename Entity>
bool EachContainerHas(Iterator FirstContainer, Iterator LastContainer, const Entity EcsEntity) noexcept
{
	for (Iterator current = FirstContainer; current != LastContainer; ++current)
	{
		if (!(*current)->Has(EcsEntity))
		{
			return false;
		}
	}

	return true;
}

template <typename Iterator, typename Entity>
bool NoneOfContainersHas(Iterator FirstContainer, Iterator LastContainer, const Entity EcsEntity) noexcept
{
	for (Iterator current = FirstContainer; current != LastContainer; ++current)
	{
		if ((*current)->Has(EcsEntity))
		{
			return false;
		}
	}

	return true;
}

template <typename... ComponentType>
struct ComponentTypeList
{
	using Type = ComponentTypeList;
	static constexpr std::size_t ListSize = sizeof...(ComponentType);
};

template <typename, typename>
struct ComponentTypeListIndex;

template <typename ComponentType>
struct ComponentTypeListIndex<ComponentType, ComponentTypeList<>>
{
	static_assert(AlwaysFalse<ComponentType>::value, "ComponentType is not in ComponentTypeList");
	static constexpr std::size_t Index = 0u;
};

template <typename ComponentType, typename... Other>
struct ComponentTypeListIndex<ComponentType, ComponentTypeList<ComponentType, Other...>>
{
	static constexpr std::size_t Index = 0u;
};

template <typename ComponentType, typename First, typename... Other>
struct ComponentTypeListIndex<ComponentType, ComponentTypeList<First, Other...>>
{
	static constexpr std::size_t Index = 1u + ComponentTypeListIndex<ComponentType, ComponentTypeList<Other...>>::Index;
};

template <typename ComponentType, typename List>
inline constexpr std::size_t ComponentIndexInList = ComponentTypeListIndex<ComponentType, List>::Index;

template <typename... ComponentTypes>
struct IncludedComponentTypes final : ComponentTypeList<ComponentTypes...>
{
	explicit constexpr IncludedComponentTypes() = default;
};

template <typename... ExcludedComponents>
struct ExcludedComponentTypes final : ComponentTypeList<ExcludedComponents...>
{
	explicit constexpr ExcludedComponentTypes() = default;
};

template <typename... ExcludedComponents>
inline constexpr ExcludedComponentTypes<ExcludedComponents...> ExcludeComponentTypes{};

template <std::size_t, typename>
struct ComponentStorageType;

template <std::size_t Index, typename FirstType, typename... OtherTypes>
struct ComponentStorageType<Index, ComponentTypeList<FirstType, OtherTypes...>>
	: ComponentStorageType<Index - 1u, ComponentTypeList<OtherTypes...>>
{
};

template <typename FirstType, typename... OtherTypes>
struct ComponentStorageType<0u, ComponentTypeList<FirstType, OtherTypes...>>
{
	using Type = FirstType;
};

template <std::size_t Index, typename List>
using ComponentStorageTypeAtIndex = typename ComponentStorageType<Index, List>::Type;

template <typename To, typename From>
struct TransferConstness
{
	using Type = std::remove_const_t<To>;
};

template <typename To, typename From>
struct TransferConstness<To, const From>
{
	using Type = const To;
};

template <typename To, typename From>
using TransferConstnessType = typename TransferConstness<To, From>::Type;

template <typename BaseStorageType, std::size_t Num, std::size_t ExcludeNum>
class EcsComponentStorageViewIterator
{
	using iterator_type = typename BaseStorageType::const_iterator;
	using iterator_traits = std::iterator_traits<iterator_type>;

public:
	using value_type = typename iterator_traits::value_type;
	using pointer = typename iterator_traits::pointer;
	using reference = typename iterator_traits::reference;
	using difference_type = typename iterator_traits::difference_type;
	using iterator_category = std::forward_iterator_tag;

	constexpr EcsComponentStorageViewIterator() noexcept
		: Iterator{}
		  , ComponentStorages{}
		  , ExcludedComponentStorages{}
		  , Index{}
	{
	}

	EcsComponentStorageViewIterator(iterator_type First, std::array<const BaseStorageType*, Num> Storages,
	                                std::array<const BaseStorageType*, ExcludeNum> ExcludedStorages, std::size_t IndexIn) noexcept
		: Iterator(First)
		  , ComponentStorages(Storages)
		  , ExcludedComponentStorages(ExcludedStorages)
		  , Index(static_cast<difference_type>(IndexIn))
	{
	}

	EcsComponentStorageViewIterator& operator++() noexcept
	{
		++Iterator;
		Advance();
		return *this;
	}

	EcsComponentStorageViewIterator operator++(int) noexcept
	{
		const EcsComponentStorageViewIterator current = *this;
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
	friend constexpr bool operator==(const EcsComponentStorageViewIterator<LhsType, LhsArgs...>&,
	                                 const EcsComponentStorageViewIterator<RhsType, RhsArgs...>&) noexcept;

private:
	bool IsValid(const typename iterator_traits::value_type Entity) const noexcept
	{
		if (Num != 1u)
		{
			if (!EachContainerHas(ComponentStorages.begin(), ComponentStorages.begin() + Index, Entity) ||
				!EachContainerHas(ComponentStorages.begin() + Index + 1, ComponentStorages.end(), Entity))
			{
				return false;
			}
		}

		if (ExcludeNum != 0u)
		{
			if (!NoneOfContainersHas(ExcludedComponentStorages.begin(), ExcludedComponentStorages.end(), Entity))
			{
				return false;
			}
		}

		return true;
	}


	void Advance()
	{
		for (constexpr iterator_type nullIterator; Iterator != nullIterator && !IsValid(*Iterator); ++Iterator)
		{
		}
	}

private:
	iterator_type Iterator;
	std::array<const BaseStorageType*, Num> ComponentStorages;
	std::array<const BaseStorageType*, ExcludeNum> ExcludedComponentStorages;
	difference_type Index;
};

template <typename LhsType, auto... LhsArgs, typename RhsType, auto... RhsArgs>
[[nodiscard]] constexpr bool operator==(const EcsComponentStorageViewIterator<LhsType, LhsArgs...>& Lhs,
                                        const EcsComponentStorageViewIterator<RhsType, RhsArgs...>& Rhs) noexcept
{
	return Lhs.Iterator == Rhs.Iterator;
}

template <typename LhsType, auto... LhsArgs, typename RhsType, auto... RhsArgs>
[[nodiscard]] constexpr bool operator!=(const EcsComponentStorageViewIterator<LhsType, LhsArgs...>& Lhs,
                                        const EcsComponentStorageViewIterator<RhsType, RhsArgs...>& Rhs) noexcept
{
	return !(Lhs == Rhs);
}

template <typename BaseStorageType, std::size_t Num, std::size_t ExcludeNum>
class EcsComponentStorageViewBase
{
public:
	using base_storage_type = BaseStorageType;
	using entity_type = typename BaseStorageType::value_type;
	using size_type = std::size_t;
	using difference_type = std::ptrdiff_t;
	using iterator = EcsComponentStorageViewIterator<BaseStorageType, Num, ExcludeNum>;

	void RefreshLeadingStorage() noexcept
	{
		if (LeadingStorageIndex == Num)
		{
			return;
		}

		for (size_type current = 0; current < Num; ++current)
		{
			if (ComponentStorages[current] == nullptr)
			{
				return;
			}
		}

		UpdateLeadingStorageIndex();
	}

	const base_storage_type* GetLeadingStorage() const noexcept
	{
		return LeadingStorageIndex != Num ? ComponentStorages[LeadingStorageIndex] : nullptr;
	}

	size_type Size() const noexcept
	{
		return LeadingStorageIndex != Num ? GetLeadingStorageSize() : 0u;
	}

	iterator begin() const noexcept
	{
		if (LeadingStorageIndex == Num)
		{
			return {};
		}

		return {
			ComponentStorages[LeadingStorageIndex]->end() - static_cast<difference_type>(GetLeadingStorageSize()), ComponentStorages,
			ExcludedComponentStorages, LeadingStorageIndex
		};
	}

	iterator end() const noexcept
	{
		if (LeadingStorageIndex == Num)
		{
			return {};
		}

		return {ComponentStorages[LeadingStorageIndex]->end(), ComponentStorages, ExcludedComponentStorages, LeadingStorageIndex};
	}

	entity_type front() const noexcept
	{
		const auto it = begin();
		return it != end() ? *it : EcsEntityNull;
	}

	bool Has(const entity_type Entity) const noexcept
	{
		if (LeadingStorageIndex == Num)
		{
			return false;
		}

		if (!EachContainerHas(ComponentStorages.begin(), ComponentStorages.end(), Entity))
		{
			return false;
		}

		if (!NoneOfContainersHas(ExcludedComponentStorages.begin(), ExcludedComponentStorages.end(), Entity))
		{
			return false;
		}

		return true;
	}

	iterator Find(const entity_type Entity)
	{
		if (!Has(Entity))
		{
			return end();
		}

		return {ComponentStorages[LeadingStorageIndex]->Find(Entity), ComponentStorages, ExcludedComponentStorages, LeadingStorageIndex};
	}

	explicit operator bool() const noexcept
	{
		return LeadingStorageIndex != Num;
	}

protected:
	EcsComponentStorageViewBase() noexcept
		: LeadingStorageIndex(Num)
	{
	}

	EcsComponentStorageViewBase(std::array<const BaseStorageType*, Num> Storages,
	                            std::array<const BaseStorageType*, ExcludeNum> ExcludedStorages) noexcept
		: ComponentStorages(Storages)
		  , ExcludedComponentStorages(ExcludedStorages)
		  , LeadingStorageIndex(Num)
	{
		UpdateLeadingStorageIndex();
	}

	const BaseStorageType* GetComponentStorageAt(const size_type Index) const noexcept
	{
		return ComponentStorages[Index];
	}

	void SetComponentStorageAt(const size_type Index, const BaseStorageType* NewStorage) noexcept
	{
		LE_ASSERT_DESC(NewStorage != nullptr, "Invalid new storage")
		ComponentStorages[Index] = NewStorage;
		RefreshLeadingStorage();
	}

	const BaseStorageType* GetExcludedComponentStorageAt(const size_type Index) const noexcept
	{
		return ExcludedComponentStorages[Index];
	}

	void SetExcludedComponentStorageAt(const size_type Index, const BaseStorageType* NewStorage) noexcept
	{
		LE_ASSERT_DESC(NewStorage != nullptr, "Invalid new excluded storage")
		ExcludedComponentStorages[Index] = NewStorage;
	}

	void SetLeadingStorage(const size_type Index) noexcept
	{
		LeadingStorageIndex = LeadingStorageIndex != Num ? Index : Num;
	}

private:
	void UpdateLeadingStorageIndex() noexcept
	{
		LeadingStorageIndex = 0u;
		if (Num > 1u)
		{
			for (size_type current = 1u; current < Num; ++current)
			{
				if (ComponentStorages[current]->Count() < ComponentStorages[LeadingStorageIndex]->Count())
				{
					LeadingStorageIndex = current;
				}
			}
		}
	}

	size_type GetLeadingStorageSize() const noexcept
	{
		return ComponentStorages[LeadingStorageIndex]->Count();
	}

private:
	std::array<const BaseStorageType*, Num> ComponentStorages;
	std::array<const BaseStorageType*, ExcludeNum> ExcludedComponentStorages;
	size_type LeadingStorageIndex;
};

template <typename, typename>
class EcsStorageView;

template <typename... Components, typename... ExcludedComponents>
class EcsStorageView<IncludedComponentTypes<Components...>, ExcludedComponentTypes<ExcludedComponents...>> : public
	EcsComponentStorageViewBase<
		std::common_type_t<typename Components::base_type...>, sizeof...(Components), sizeof...(ExcludedComponents)>
{
	using base_type = EcsComponentStorageViewBase<std::common_type_t<typename Components::base_type...>, sizeof...(Components), sizeof...(
		                                              ExcludedComponents)>;

	template <std::size_t Index>
	using StorageTypeAt = ComponentStorageTypeAtIndex<Index, ComponentTypeList<Components..., ExcludedComponents...>>;

public:
	using common_type = typename base_type::base_storage_type;
	using entity_type = typename base_type::entity_type;
	using size_type = typename base_type::size_type;
	using difference_type = std::ptrdiff_t;
	using iterator = typename base_type::iterator;

	EcsStorageView() noexcept = default;

	EcsStorageView(Components&... ComponentsIn, ExcludedComponents&... Excluded) noexcept
		: base_type({&ComponentsIn...}, {&Excluded...})
	{
	}

	template <typename ComponentType>
	void SetLeadingComponent() noexcept
	{
		SetLeadingStorage(ComponentStorageIndex<ComponentType>);
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

private:
	template <typename ComponentType>
	static constexpr size_type ComponentStorageIndex = ComponentIndexInList<
		ComponentType, ComponentTypeList<typename Components::value_type...>>;
};
}
