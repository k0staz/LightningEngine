#pragma once
#include <type_traits>

namespace LE
{
template <typename... Types>
struct TypeList
{
	using Type = TypeList;
	static constexpr auto size = sizeof...(Types);
};

template <std::size_t, typename>
struct TypeListElement;

template <std::size_t Index, typename First, typename... Other>
struct TypeListElement<Index, TypeList<First, Other...>> : TypeListElement<Index - 1u, TypeList<Other...>>
{
};

template <typename First, typename... Other>
struct TypeListElement<0u, TypeList<First, Other...>>
{
	using Type = First;
};

template <std::size_t Index, typename List>
using TypeListElementType = typename TypeListElement<Index, List>::Type;

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

}
