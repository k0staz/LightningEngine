#pragma once

#include <type_traits>

namespace LE
{

#define ENUM_CLASS_FLAGS(Enum) \
	inline Enum& operator|=(Enum& Lhs, Enum Rhs) { return Lhs = static_cast<Enum>((__underlying_type(Enum))Lhs | (__underlying_type(Enum))Rhs); } \
	inline constexpr Enum operator| (Enum  Lhs, Enum Rhs) { return static_cast<Enum>((__underlying_type(Enum))Lhs | (__underlying_type(Enum))Rhs); }

template <typename Enum>
constexpr bool EnumHasAnyFlags(Enum Flags, Enum Elements)
{
	using Type = __underlying_type(Enum);
	return (static_cast<Type>(Flags) & static_cast<Type>(Elements)) != 0;
}

template <typename Enum>
constexpr bool EnumHasAllFlags(Enum Flags, Enum Elements)
{
	using Type = __underlying_type(Enum);
	return (static_cast<Type>(Flags) & static_cast<Type>(Elements)) == static_cast<Type>(Elements);
}

template <typename Enum>
constexpr void EnumAddFlags(Enum& Flags, Enum Elements)
{
	using Type = __underlying_type(Enum);
	Flags = static_cast<Enum>(static_cast<Type>(Flags) | static_cast<Type>(Elements));
}

template <typename Enum>
constexpr void EnumRemoveFlags(Enum& Flags, Enum Elements)
{
	using Type = __underlying_type(Enum);
	Flags = static_cast<Enum>(static_cast<Type>(Flags) & ~static_cast<Type>(Elements));
}
}
