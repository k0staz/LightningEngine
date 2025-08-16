#pragma once

#include <cmath>
#include <limits>
#include <utility>

#include "CoreConcepts.h"
#include "CoreMinimum.h"

#undef max
#undef min

namespace LE
{
constexpr float PI = 3.14159265358979323846f;
constexpr float TWO_PI = 6.28318530717958647692f;
constexpr float HALF_PI = 1.57079632679489661923f;

template <typename T>
struct Constants
{
	static constexpr T CMax = std::numeric_limits<T>::max();
	static constexpr T CMin = std::numeric_limits<T>::min();
	static constexpr T CEpsilon = std::numeric_limits<T>::epsilon();
};

template <Numeric T>
constexpr T Abs(T Value)
{
	return (Value >= static_cast<T>(0) ? Value : -Value);
}

template<Numeric T>
constexpr T Min(T ValueA, T ValueB)
{
	return std::min<T>(ValueA, ValueB);
}

template<Numeric T>
constexpr T Max(T ValueA, T ValueB)
{
	return std::max<T>(ValueA, ValueB);
}

template <Numeric T>
constexpr bool AreClose(T Num1, T Num2, float Tolerance = 0.000001f)
{
	return (Abs(Num1 - Num2) <= Tolerance);
}

inline float Sin(const float Value)
{
	return std::sin(Value);
}

inline float Cos(const float Value)
{
	return std::cos(Value);
}

inline float Tan(const float Value)
{
	return std::tan(Value);
}

inline float Asin(const float Value)
{
	return std::asin(Value);
}

inline float Acos(const float Value)
{
	return std::acos(Value);
}

inline float Atan2(const float Left, const float Right)
{
	return std::atan2(Left, Right);
}

template <Numeric T>
inline T Sqrt(const T Value)
{
	return static_cast<T>(std::sqrt(Value));
}

inline int32_t CountBits(uint64_t Bits)
{
	// https://en.wikipedia.org/wiki/Hamming_weight
	Bits -= (Bits >> 1) & 0x5555555555555555ull;
	Bits = (Bits & 0x3333333333333333ull) + ((Bits >> 2) & 0x3333333333333333ull);
	Bits = (Bits + (Bits >> 4)) & 0x0f0f0f0f0f0f0f0full;
	return (Bits * 0x0101010101010101) >> 56;
}

template<Unsigned T>
T FastMod(const T Value, const T PowerOfTwoMod) noexcept
{
	LE_ASSERT_DESC(std::has_single_bit(PowerOfTwoMod), "Mod must be power of two, supplied mode: {}", PowerOfTwoMod)
	return Value & (PowerOfTwoMod - 1);
}

constexpr uint32 FNV1AHash(std::string_view String)
{
	uint32 result = 2166136261u; // Offset Basis
	for (const char c : String)
	{
		result ^= static_cast<unsigned char>(c);
		result *= 16777619u; // FNV Prime
	}

	return result;
}
}
