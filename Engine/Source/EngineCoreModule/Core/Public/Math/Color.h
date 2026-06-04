#pragma once
#include <cstdint>
#include <iomanip>
#include <ios>
#include <sstream>

#include "LinearColor.h"

namespace LE
{
struct Color
{
public:
	static consteval Color Black() { return {0, 0, 0}; }
	static consteval Color Bronze() { return {205, 127, 50}; }
	static consteval Color Brown() { return {165, 42, 42}; }
	static consteval Color Blue() { return {0, 0, 255}; }
	static consteval Color Cyan() { return {0, 255, 255}; }
	static consteval Color DarkBlue() { return {0, 0, 128}; }
	static consteval Color DarkCyan() { return {0, 128, 128}; }
	static consteval Color DarkGray() { return {64, 64, 64}; }
	static consteval Color DarkGreen() { return {0, 128, 0}; }
	static consteval Color DarkMagenta() { return {128, 0, 128}; }
	static consteval Color DarkRed() { return {128, 0, 0}; }
	static consteval Color DarkYellow() { return {128, 128, 0}; }
	static consteval Color Gold() { return {255, 215, 0}; }
	static consteval Color Gray() { return {128, 128, 128}; }
	static consteval Color Green() { return {0, 255, 0}; }
	static consteval Color LightBlue() { return {128, 128, 255}; }
	static consteval Color LightCyan() { return {128, 255, 255}; }
	static consteval Color LightGray() { return {192, 192, 192}; }
	static consteval Color LightGreen() { return {128, 255, 128}; }
	static consteval Color LightMagenta() { return {255, 128, 255}; }
	static consteval Color LightRed() { return {255, 128, 128}; }
	static consteval Color LightYellow() { return {255, 255, 128}; }
	static consteval Color Magenta() { return {255, 0, 255}; }
	static consteval Color Orange() { return {255, 165, 0}; }
	static consteval Color Pink() { return {255, 192, 203}; }
	static consteval Color Purple() { return {128, 0, 128}; }
	static consteval Color Red() { return {255, 0, 0}; }
	static consteval Color Silver() { return {192, 192, 192}; }
	static consteval Color Transparent() { return {0, 0, 0, 0}; }
	static consteval Color Violet() { return {238, 130, 238}; }
	static consteval Color Yellow() { return {255, 255, 0}; }
	static consteval Color White() { return {255, 255, 255}; }

public:
	constexpr Color() = default;
	constexpr Color(uint32_t RGBA) noexcept;
	constexpr Color(LinearColor LColor) noexcept;
	constexpr Color(uint8_t Red, uint8_t Green, uint8_t Blue, uint8_t Alpha = 255) noexcept;

	constexpr Color operator+(const Color& Other) const noexcept;
	constexpr Color operator-(const Color& Other) const noexcept;
	constexpr Color operator*(float Scalar) const noexcept;
	constexpr Color operator/(float Scalar) const noexcept;

	constexpr Color& operator+=(const Color& Other) noexcept;
	constexpr Color& operator-=(const Color& Other) noexcept;
	constexpr Color& operator*=(float Scalar) noexcept;
	constexpr Color& operator/=(float Scalar) noexcept;

	constexpr void FromRGBA(uint32_t RGBA) noexcept;
	constexpr void FromLinearColor(LinearColor LColor) noexcept;

	constexpr uint32_t ToRGBA() const noexcept;
	constexpr LinearColor ToLinearColor() const noexcept;

	constexpr bool operator==(const Color& Other) const noexcept;

public:
	uint8_t R = 0;
	uint8_t G = 0;
	uint8_t B = 0;
	uint8_t A = 255;
};

constexpr Color::Color(uint32_t RGBA) noexcept
{
	FromRGBA(RGBA);
}

constexpr Color::Color(LinearColor LColor) noexcept
{
	FromLinearColor(LColor);
}

constexpr Color::Color(uint8_t Red, uint8_t Green, uint8_t Blue, uint8_t Alpha) noexcept
	: R(Red), G(Green), B(Blue), A(Alpha)
{
}

constexpr Color Color::operator+(const Color& Other) const noexcept
{
	return Color(R + Other.R, G + Other.G, B + Other.B, A + Other.A);
}

constexpr Color Color::operator-(const Color& Other) const noexcept
{
	return Color(R - Other.R, G - Other.G, B - Other.B, A - Other.A);
}

constexpr Color Color::operator*(float Scalar) const noexcept
{
	return Color(static_cast<uint8_t>(R * Scalar), static_cast<uint8_t>(G * Scalar), static_cast<uint8_t>(B * Scalar),
	             static_cast<uint8_t>(A * Scalar));
}

constexpr Color Color::operator/(float Scalar) const noexcept
{
	return Color(static_cast<uint8_t>(R / Scalar), static_cast<uint8_t>(G / Scalar), static_cast<uint8_t>(B / Scalar),
	             static_cast<uint8_t>(A / Scalar));
}

constexpr Color& Color::operator+=(const Color& Other) noexcept
{
	*this = *this + Other;
	return *this;
}

constexpr Color& Color::operator-=(const Color& Other) noexcept
{
	*this = *this - Other;
	return *this;
}

constexpr Color& Color::operator*=(float Scalar) noexcept
{
	*this = *this * Scalar;
	return *this;
}

constexpr Color& Color::operator/=(float Scalar) noexcept
{
	*this = *this / Scalar;
	return *this;
}

constexpr void Color::FromRGBA(uint32_t RGBA) noexcept
{
	R = static_cast<uint8_t>((RGBA >> 24) & 0xFF);
	G = static_cast<uint8_t>((RGBA >> 16) & 0xFF);
	B = static_cast<uint8_t>((RGBA >> 8) & 0xFF);
	A = static_cast<uint8_t>(RGBA & 0xFF);
}

constexpr void Color::FromLinearColor(LinearColor LColor) noexcept
{
	R = static_cast<uint8_t>(LColor.R * 255.f);
	G = static_cast<uint8_t>(LColor.G * 255.f);
	B = static_cast<uint8_t>(LColor.B * 255.f);
	A = static_cast<uint8_t>(LColor.A * 255.f);
}

constexpr uint32_t Color::ToRGBA() const noexcept
{
	uint32_t rgba = 0;
	rgba |= static_cast<uint32_t>(R) << 24;
	rgba |= static_cast<uint32_t>(G) << 16;
	rgba |= static_cast<uint32_t>(B) << 8;
	rgba |= static_cast<uint32_t>(A);

	return rgba;
}

constexpr LinearColor Color::ToLinearColor() const noexcept
{
	LinearColor lColor;
	constexpr float inv = 1.f / 255.f;
	lColor.R = static_cast<float>(R) * inv;
	lColor.G = static_cast<float>(G) * inv;
	lColor.B = static_cast<float>(B) * inv;
	lColor.A = static_cast<float>(A) * inv;

	return lColor;
}

constexpr bool Color::operator==(const Color& Other) const noexcept
{
	return AreClose(R, Other.R) && AreClose(G, Other.G) && AreClose(B, Other.B) && AreClose(A, Other.A);
}

constexpr Color operator*(float Scalar, Color Color) noexcept
{
	return Color * Scalar;
}

constexpr Color operator/(float Scalar, Color Color) noexcept
{
	return Color / Scalar;
}

inline std::ostream& operator<<(std::ostream& os, const Color& Color)
{
	os << '#'
		<< std::uppercase << std::hex << std::setfill('0')
		<< std::setw(2) << Color.R
		<< std::setw(2) << Color.G
		<< std::setw(2) << Color.B
		<< std::setw(2) << Color.A
		<< std::dec;
	return os;
}

inline std::string ToCssHex(Color Color)
{
	std::ostringstream oss;
	oss << '#' << std::hex << std::nouppercase << std::setfill('0')
		<< std::setw(2) << static_cast<int>(Color.R)
		<< std::setw(2) << static_cast<int>(Color.G)
		<< std::setw(2) << static_cast<int>(Color.B);
	return oss.str();
}
}
