#pragma once

#include <bit>
#include "CoreDefinitions.h"

namespace LE
{
using EcsEntity = uint32;

template <typename Type>
struct EcsEntityTraits
{
};

template <>
struct EcsEntityTraits<uint32>
{
	using ValueType = uint32;

	using IdType = uint32; // 20 bits for Entity ID
	using GenerationType = uint16; // 12 Bits for Entity Generation

	static constexpr IdType IdMask = 0xFFFFF;
	static constexpr IdType GenerationMask = 0xFFF;
};

template<typename Traits>
struct EcsTraitsInterpreter
{
	using ValueType = typename Traits::ValueType;
	using IdType = typename Traits::IdType;
	using GenerationType = typename Traits::GenerationType;

	static constexpr IdType IdMask = Traits::IdMask;
	static constexpr IdType GenerationMask = Traits::GenerationMask;
	static constexpr IdType IdLength = std::popcount(IdMask);

	static constexpr ValueType GetAsValue(const ValueType Entity)
	{
		return Entity;
	}

	static constexpr IdType GetId(const ValueType Value) noexcept
	{
		return static_cast<IdType>(Value) & IdMask;
	}

	static constexpr ValueType GetGenerationAsValue(const ValueType Entity)
	{
		return (static_cast<ValueType>(Entity) >> IdLength) & GenerationMask;
	}

	static constexpr GenerationType GetGeneration(const ValueType Value) noexcept
	{
		return static_cast<GenerationType>(static_cast<IdType>(Value) >> IdLength) & GenerationMask;
	}

	static constexpr ValueType CreateCombined(const ValueType EntityId, const ValueType Generation)
	{
		return ValueType{ (EntityId & IdMask) | (static_cast<ValueType>(Generation & GenerationMask) << IdLength) };
	}

	static constexpr ValueType IncrementGeneration(const ValueType Value) noexcept
	{
		const GenerationType newGen = GetGeneration(Value) + 1;
		return CreateCombined(Value, static_cast<ValueType>(newGen + (newGen == GenerationMask)));
	}
};

template<typename Type>
struct EcsTraits : EcsTraitsInterpreter<EcsEntityTraits<Type>>
{
	static constexpr uint64 PageSize = ENTITY_SPARSE_PAGE;
};

struct NullEntity
{
	template<typename EntityType>
	constexpr operator EntityType() const noexcept
	{
		using Traits = EcsTraits<EntityType>;
		constexpr EntityType result = Traits::CreateCombined(Traits::IdMask, Traits::GenerationMask);
		return result;
	}

	constexpr bool operator==(const NullEntity) const noexcept
	{
		return true;
	}

	constexpr bool operator!=(const NullEntity) const noexcept
	{
		return false;
	}

	template<typename EntityType>
	friend constexpr bool operator==(const EntityType Lhs, const NullEntity Rhs) noexcept
	{
		using Traits = EcsTraits<EntityType>;
		return Traits::GetId(Lhs) == Traits::GetId(Rhs);
	}

	template<typename EntityType>
	friend constexpr bool operator!=(const EntityType Lhs, const NullEntity Rhs) noexcept
	{
		return !(Lhs == Rhs);
	}

	template<typename EntityType>
	friend constexpr bool operator==(const NullEntity Lhs, const EntityType Rhs) noexcept
	{
		return Rhs == Lhs;
	}

	template<typename EntityType>
	friend constexpr bool operator!=(const NullEntity Lhs, const EntityType Rhs) noexcept
	{
		return !(Lhs == Rhs);
	}
};

inline constexpr NullEntity EcsEntityNull{};

}
