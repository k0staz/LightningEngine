#pragma once

#include <bit>

#include "CoreDefinitions.h"

namespace LE
{
template <typename Type>
struct IdTraits
{
};

template <>
struct IdTraits<uint32>
{
	using ValueType = uint32;

	using IdType = uint32; // 20 bits for ID
	using GenerationType = uint16; // 12 Bits for ID Generation

	static constexpr IdType IdMask = 0xFFFFF;
	static constexpr IdType GenerationMask = 0xFFF;
};

template <typename Traits>
struct IdTraitsInterpreter
{
	using ValueType = typename Traits::ValueType;
	using IdType = typename Traits::IdType;
	using GenerationType = typename Traits::GenerationType;

	static constexpr IdType IdMask = Traits::IdMask;
	static constexpr IdType GenerationMask = Traits::GenerationMask;
	static constexpr IdType IdLength = std::popcount(IdMask);

	static constexpr ValueType GetAsValue(const ValueType Id)
	{
		return Id;
	}

	static constexpr IdType GetId(const ValueType Value) noexcept
	{
		return static_cast<IdType>(Value) & IdMask;
	}

	static constexpr ValueType GetGenerationAsValue(const ValueType Value)
	{
		return (static_cast<ValueType>(Value) >> IdLength) & GenerationMask;
	}

	static constexpr GenerationType GetGeneration(const ValueType Value) noexcept
	{
		return static_cast<GenerationType>(static_cast<IdType>(Value) >> IdLength) & GenerationMask;
	}

	static constexpr ValueType CreateCombined(const ValueType Id, const ValueType Generation)
	{
		return ValueType{(Id & IdMask) | (static_cast<ValueType>(Generation & GenerationMask) << IdLength)};
	}

	static constexpr ValueType IncrementGeneration(const ValueType Value) noexcept
	{
		const GenerationType newGen = GetGeneration(Value) + 1;
		return CreateCombined(Value, static_cast<ValueType>(newGen + (newGen == GenerationMask)));
	}
};

struct NullId
{
	template<typename IdType>
	constexpr operator IdType() const noexcept
	{
		using Traits = IdTraitsInterpreter<IdTraits<IdType>>;
		constexpr IdType result = Traits::CreateCombined(Traits::IdMask, Traits::GenerationMask);
		return result;
	}

	constexpr bool operator==(const NullId) const noexcept
	{
		return true;
	}

	constexpr bool operator!=(const NullId) const noexcept
	{
		return false;
	}

	template<typename IdType>
	friend constexpr bool operator==(const IdType Lhs, const NullId Rhs) noexcept
	{
		using Traits = IdTraitsInterpreter<IdTraits<IdType>>;
		return Traits::GetId(Lhs) == Traits::GetId(Rhs);
	}

	template<typename IdType>
	friend constexpr bool operator!=(const IdType Lhs, const NullId Rhs) noexcept
	{
		return !(Lhs == Rhs);
	}

	template<typename IdType>
	friend constexpr bool operator==(const NullId Lhs, const IdType Rhs) noexcept
	{
		return Rhs == Lhs;
	}

	template<typename IdType>
	friend constexpr bool operator!=(const NullId Lhs, const IdType Rhs) noexcept
	{
		return !(Lhs == Rhs);
	}
};
}
