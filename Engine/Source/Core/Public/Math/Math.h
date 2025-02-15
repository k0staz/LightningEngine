#pragma once

#include <cmath>
#include <limits>

#include "CoreConcepts.h"

namespace LE
{
	constexpr float PI = 3.14159265358979323846f;
	constexpr float TWO_PI = 6.28318530717958647692f;
	constexpr float HALF_PI = 1.57079632679489661923f;

	template<typename T>
	struct Constants
	{
		static constexpr T CMax = std::numeric_limits<T>::max();
		static constexpr T CMin = std::numeric_limits<T>::min();
		static constexpr T CEpsilon = std::numeric_limits<T>::epsilon();
	};

	template<Numeric T>
	constexpr T Abs(T Value)
	{
		return (Value >= static_cast<T>(0) ? Value : -Value);
	}

	template<Numeric T>
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

	template<Numeric T>
	inline T Sqrt(const T Value)
	{
		return static_cast<T>(std::sqrt(Value));
	}

}