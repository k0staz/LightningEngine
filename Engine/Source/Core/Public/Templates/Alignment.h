#pragma once
#include "CoreDefinitions.h"
#include "CoreConcepts.h"

namespace LE
{
template <Alignable T>
constexpr T Align(T Value, uint64 Alignment)
{
	if constexpr (std::is_pointer_v<T>)
	{
		std::uintptr_t intValue = reinterpret_cast<std::uintptr_t>(Value);
		uint64 aligned = (intValue + Alignment - 1) & ~(Alignment - 1);
		return reinterpret_cast<T>(aligned);
	}
	else
	{
		uint64 intValue = static_cast<uint64>(Value);
		uint64 aligned = (intValue + Alignment - 1) & ~(Alignment - 1);
		return static_cast<T>(aligned);
	}
}

template <Alignable T>
constexpr T AlignDown(T Value, uint64 Alignment)
{
	if constexpr (std::is_pointer_v<T>)
	{
		std::uintptr_t intValue = reinterpret_cast<std::uintptr_t>(Value);
		uint64 aligned = intValue & ~(Alignment - 1);
		return reinterpret_cast<T>(aligned);
	}
	else
	{
		uint64 intValue = static_cast<uint64>(Value);
		uint64 aligned = intValue & ~(Alignment - 1);
		return static_cast<T>(aligned);
	}
}

template <Alignable T>
constexpr bool IsAligned(T Value, uint64 Alignment)
{
	if constexpr (std::is_pointer_v<T>)
	{
		std::uintptr_t intValue = reinterpret_cast<std::uintptr_t>(Value);
		return !(intValue & (Alignment - 1));
	}
	else
	{
		uint64 intValue = static_cast<uint64>(Value);
		return !(intValue & (Alignment - 1));
	}
}
}
