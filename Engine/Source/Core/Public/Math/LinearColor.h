#pragma once
#include <cstdint>

#include "Math.h"

namespace LE
{
	struct LinearColor
	{
	public:
		static consteval LinearColor Black() { return {0.0f, 0.0f, 0.0f}; }
		static consteval LinearColor Bronze() { return {0.8f, 0.5f, 0.2f}; }
		static consteval LinearColor Brown() { return {0.6f, 0.4f, 0.2f}; }
		static consteval LinearColor Blue() { return {0.0f, 0.0f, 1.0f}; }
		static consteval LinearColor Cyan() { return {0.0f, 1.0f, 1.0f}; }
		static consteval LinearColor DarkBlue() { return {0.0f, 0.0f, 0.5f}; }
		static consteval LinearColor DarkCyan() { return {0.0f, 0.5f, 0.5f}; }
		static consteval LinearColor DarkGray() { return {0.25f, 0.25f, 0.25f}; }
		static consteval LinearColor DarkGreen() { return {0.0f, 0.5f, 0.0f}; }
		static consteval LinearColor DarkMagenta() { return {0.5f, 0.0f, 0.5f}; }
		static consteval LinearColor DarkRed() { return {0.5f, 0.0f, 0.0f}; }
		static consteval LinearColor DarkYellow() { return {0.5f, 0.5f, 0.0f}; }
		static consteval LinearColor Gold() { return {1.0f, 0.84f, 0.0f}; }
		static consteval LinearColor Gray() { return {0.5f, 0.5f, 0.5f}; }
		static consteval LinearColor Green() { return {0.0f, 1.0f, 0.0f}; }
		static consteval LinearColor LightBlue() { return {0.5f, 0.5f, 1.0f}; }
		static consteval LinearColor LightCyan() { return {0.5f, 1.0f, 1.0f}; }
		static consteval LinearColor LightGray() { return {0.75f, 0.75f, 0.75f}; }
		static consteval LinearColor LightGreen() { return {0.5f, 1.0f, 0.5f}; }
		static consteval LinearColor LightMagenta() { return {1.0f, 0.5f, 1.0f}; }
		static consteval LinearColor LightRed() { return {1.0f, 0.5f, 0.5f}; }
		static consteval LinearColor LightYellow() { return {1.0f, 1.0f, 0.5f}; }
		static consteval LinearColor Magenta() { return {1.0f, 0.0f, 1.0f}; }
		static consteval LinearColor Orange() { return {1.0f, 0.5f, 0.0f}; }
		static consteval LinearColor Pink() { return {1.0f, 0.75f, 0.8f}; }
		static consteval LinearColor Purple() { return {0.5f, 0.0f, 0.5f}; }
		static consteval LinearColor Red() { return {1.0f, 0.0f, 0.0f}; }
		static consteval LinearColor Silver() { return {0.75f, 0.75f, 0.75f}; }
		static consteval LinearColor Transparent() { return {0.0f, 0.0f, 0.0f, 0.0f}; }
		static consteval LinearColor Violet() { return {0.5f, 0.0f, 1.0f}; }
		static consteval LinearColor Yellow() { return {1.0f, 1.0f, 0.0f}; }
		static consteval LinearColor White() { return {1.0f, 1.0f, 1.0f}; }

	public:
		constexpr LinearColor() = default;
		constexpr LinearColor(uint32_t RGBA) noexcept;

		constexpr LinearColor(float Red, float Green, float Blue, float Alpha = 1.0f) noexcept;

		constexpr LinearColor operator+(const LinearColor& Other) const noexcept;
		constexpr LinearColor operator-(const LinearColor& Other) const noexcept;
		constexpr LinearColor operator*(float Scalar) const noexcept;
		constexpr LinearColor operator/(float Scalar) const noexcept;

		constexpr LinearColor& operator+=(const LinearColor& Other) noexcept;
		constexpr LinearColor& operator-=(const LinearColor& Other) noexcept;
		constexpr LinearColor& operator*=(float Scalar) noexcept;
		constexpr LinearColor& operator/=(float Scalar) noexcept;

		constexpr void FromRGBA(uint32_t RGBA) noexcept;
		constexpr uint32_t ToRGBA() const noexcept;

		constexpr bool operator==(const LinearColor& Other) const noexcept;

	public:
		float R = 0.f;
		float G = 0.f;
		float B = 0.f;
		float A = 1.0f;
	};

	constexpr LinearColor::LinearColor(uint32_t RGBA) noexcept
	{
		FromRGBA(RGBA);
	}

	constexpr LinearColor::LinearColor(float Red, float Green, float Blue, float Alpha) noexcept
		: R(Red), G(Green), B(Blue), A(Alpha)
	{
	}

	constexpr LinearColor LinearColor::operator+(const LinearColor& Other) const noexcept
	{
		return LinearColor(R + Other.R, G + Other.G, B + Other.B, A + Other.A);
	}

	constexpr LinearColor LinearColor::operator-(const LinearColor& Other) const noexcept
	{
		return LinearColor(R - Other.R, G - Other.G, B - Other.B, A - Other.A);
	}

	constexpr LinearColor LinearColor::operator*(float Scalar) const noexcept
	{
		return LinearColor(R * Scalar, G * Scalar, B * Scalar, A * Scalar);
	}

	constexpr LinearColor LinearColor::operator/(float Scalar) const noexcept
	{
		return LinearColor(R / Scalar, G / Scalar, B / Scalar, A / Scalar);
	}

	constexpr LinearColor& LinearColor::operator+=(const LinearColor& Other) noexcept
	{
		*this = *this + Other;
		return *this;
	}

	constexpr LinearColor& LinearColor::operator-=(const LinearColor& Other) noexcept
	{
		*this = *this - Other;
		return *this;
	}

	constexpr LinearColor& LinearColor::operator*=(float Scalar) noexcept
	{
		*this = *this * Scalar;
		return *this;
	}

	constexpr LinearColor& LinearColor::operator/=(float Scalar) noexcept
	{
		*this = *this / Scalar;
		return *this;
	}

	constexpr void LinearColor::FromRGBA(uint32_t RGBA) noexcept
	{
		constexpr float inv = 1.f / 255.f;

		const uint8_t r = (RGBA >> 24) & 0xFF;
		const uint8_t g = (RGBA >> 16) & 0xFF;
		const uint8_t b = (RGBA >> 8) & 0xFF;
		const uint8_t a = (RGBA) & 0xFF;

		R = static_cast<float>(r) * inv;
		G = static_cast<float>(g) * inv;
		B = static_cast<float>(b) * inv;
		A = static_cast<float>(a) * inv;
	}

	constexpr uint32_t LinearColor::ToRGBA() const noexcept
	{
		uint32_t rgba = 0;

		const uint8_t r = static_cast<uint8_t>(R * 255.f);
		const uint8_t g = static_cast<uint8_t>(G * 255.f);
		const uint8_t b = static_cast<uint8_t>(B * 255.f);
		const uint8_t a = static_cast<uint8_t>(A * 255.f);

		rgba |= (r << 24);
		rgba |= (g << 16);
		rgba |= (b << 8);
		rgba |= (a);

		return rgba;
	}

	constexpr bool LinearColor::operator==(const LinearColor& Other) const noexcept
	{
		return AreClose(R, Other.R) && AreClose(G, Other.G) && AreClose(B, Other.B) && AreClose(A, Other.A);
	}

	constexpr LinearColor operator*(float Scalar, LinearColor LColor) noexcept
	{
		return LColor * Scalar;
	}

	constexpr LinearColor operator/(float Scalar, LinearColor LColor) noexcept
	{
		return LColor / Scalar;
	}
}
