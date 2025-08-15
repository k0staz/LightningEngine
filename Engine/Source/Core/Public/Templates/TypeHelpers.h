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
}
