#pragma once

#include "Math.h"

namespace LE
{
	template <Numeric T>
	struct Vector2
	{
	public:
		static consteval Vector2 Zero() { return Vector2(static_cast<T>(0)); }
		static consteval Vector2 One() { return Vector2(static_cast<T>(1)); }
		static consteval Vector2 Up() { return Vector2(static_cast<T>(0), static_cast<T>(1)); }
		static consteval Vector2 Down() { return Vector2(static_cast<T>(0), static_cast<T>(-1)); }
		static consteval Vector2 Left() { return Vector2(static_cast<T>(-1), static_cast<T>(0)); }
		static consteval Vector2 Right() { return Vector2(static_cast<T>(1), static_cast<T>(0)); }

		constexpr Vector2() = default;
		constexpr Vector2(T Value);
		constexpr Vector2(T XValue, T YValue);

		constexpr Vector2 operator+(const Vector2& Other) const;
		constexpr Vector2 operator-(const Vector2& Other) const;
		constexpr Vector2 operator*(const Vector2& Other) const;
		constexpr Vector2 operator/(const Vector2& Other) const;

		constexpr Vector2& operator+=(const Vector2& Other);
		constexpr Vector2& operator-=(const Vector2& Other);
		constexpr Vector2& operator*=(const Vector2& Other);
		constexpr Vector2& operator/=(const Vector2& Other);

		constexpr Vector2 operator+(T Scalar) const;
		constexpr Vector2 operator-(T Scalar) const;
		constexpr Vector2 operator*(T Scalar) const;
		constexpr Vector2 operator/(T Scalar) const;

		constexpr Vector2& operator+=(T Scalar);
		constexpr Vector2& operator-=(T Scalar);
		constexpr Vector2& operator*=(T Scalar);
		constexpr Vector2& operator/=(T Scalar);

		constexpr T& operator[](size_t Index) noexcept;
		constexpr const T& operator[](size_t Index) const noexcept;

		constexpr static T Dot(const Vector2& Left, const Vector2& Right);
		constexpr T Dot(const Vector2& Other) const;

		constexpr static Vector2 Project(const Vector2& Vector, const Vector2& To);
		constexpr Vector2 Project(const Vector2& To) const;

		constexpr static Vector2 Reject(const Vector2& Vector, const Vector2& From);
		constexpr Vector2 Reject(const Vector2& From) const;

		constexpr static float Length(const Vector2& Vector);
		constexpr float Length() const;

		constexpr static T Length2(const Vector2& Vector);
		constexpr T Length2() const;

		constexpr static float Distance(const Vector2& From, const Vector2& To);
		constexpr float Distance(const Vector2& Other) const;

		constexpr static T Distance2(const Vector2& From, const Vector2& To);
		constexpr T Distance2(const Vector2& Other) const;

		constexpr static Vector2 GetNormalized(const Vector2& Vector);
		constexpr Vector2 GetNormalized() const;
		constexpr void Normalize();

		constexpr static float Angle(const Vector2& Left, const Vector2& Right);
		constexpr static float AngleSigned(const Vector2& Left, const Vector2& Right);
		constexpr float Angle(const Vector2& Other) const;
		constexpr float AngleSigned(const Vector2& Other) const;

		constexpr static Vector2 Rotate(const Vector2& Vector, float Radians);
		constexpr static Vector2 RotateAround(const Vector2& Vector, const Vector2& Pivot, float Radians);
		constexpr void Rotate(float Radians);
		constexpr void RotateAround(const Vector2& Pivot, float Radians);

	public:
		T X = static_cast<T>(0);
		T Y = static_cast<T>(0);
	};

	template <Numeric T>
	constexpr Vector2<T>::Vector2(T Value)
		: X(Value)
		  , Y(Value)
	{
	}

	template <Numeric T>
	constexpr Vector2<T>::Vector2(T XValue, T YValue)
		: X(XValue)
		  , Y(YValue)
	{
	}

	template <Numeric T>
	constexpr Vector2<T> Vector2<T>::operator+(const Vector2& Other) const
	{
		return Vector2(X + Other.X, Y + Other.Y);
	}

	template <Numeric T>
	constexpr Vector2<T> Vector2<T>::operator-(const Vector2& Other) const
	{
		return Vector2(X - Other.X, Y - Other.Y);
	}

	template <Numeric T>
	constexpr Vector2<T> Vector2<T>::operator*(const Vector2& Other) const
	{
		return Vector2(X * Other.X, Y * Other.Y);
	}

	template <Numeric T>
	constexpr Vector2<T> Vector2<T>::operator/(const Vector2& Other) const
	{
		return Vector2(X / Other.X, Y / Other.Y);
	}

	template <Numeric T>
	constexpr Vector2<T>& Vector2<T>::operator+=(const Vector2& Other)
	{
		X += Other.X;
		Y += Other.Y;

		return this;
	}

	template <Numeric T>
	constexpr Vector2<T>& Vector2<T>::operator-=(const Vector2& Other)
	{
		X -= Other.X;
		Y -= Other.Y;

		return this;
	}

	template <Numeric T>
	constexpr Vector2<T>& Vector2<T>::operator*=(const Vector2& Other)
	{
		X *= Other.X;
		Y *= Other.Y;

		return this;
	}

	template <Numeric T>
	constexpr Vector2<T>& Vector2<T>::operator/=(const Vector2& Other)
	{
		X /= Other.X;
		Y /= Other.Y;

		return this;
	}

	template <Numeric T>
	constexpr Vector2<T> Vector2<T>::operator+(T Scalar) const
	{
		return Vector2(X + Scalar, Y + Scalar);
	}

	template <Numeric T>
	constexpr Vector2<T> Vector2<T>::operator-(T Scalar) const
	{
		return Vector2(X - Scalar, Y - Scalar);
	}

	template <Numeric T>
	constexpr Vector2<T> Vector2<T>::operator*(T Scalar) const
	{
		return Vector2(X * Scalar, Y * Scalar);
	}

	template <Numeric T>
	constexpr Vector2<T> Vector2<T>::operator/(T Scalar) const
	{
		return Vector2(X / Scalar, Y / Scalar);
	}

	template <Numeric T>
	constexpr Vector2<T>& Vector2<T>::operator+=(T Scalar)
	{
		X += Scalar;
		Y += Scalar;

		return *this;
	}

	template <Numeric T>
	constexpr Vector2<T>& Vector2<T>::operator-=(T Scalar)
	{
		X -= Scalar;
		Y -= Scalar;

		return *this;
	}

	template <Numeric T>
	constexpr Vector2<T>& Vector2<T>::operator*=(T Scalar)
	{
		X *= Scalar;
		Y *= Scalar;

		return *this;
	}

	template <Numeric T>
	constexpr Vector2<T>& Vector2<T>::operator/=(T Scalar)
	{
		X /= Scalar;
		Y /= Scalar;

		return *this;
	}

	template <Numeric T>
	constexpr T& Vector2<T>::operator[](size_t Index) noexcept
	{
		return (&X)[Index];
	}

	template <Numeric T>
	constexpr const T& Vector2<T>::operator[](size_t Index) const noexcept
	{
		return (&X)[Index];
	}

	template <Numeric T>
	constexpr T Vector2<T>::Dot(const Vector2& Left, const Vector2& Right)
	{
		return (Left.X * Right.X + Left.Y * Right.Y);
	}

	template <Numeric T>
	constexpr T Vector2<T>::Dot(const Vector2& Other) const
	{
		return (X * Other.X + Y * Other.Y);
	}

	template <Numeric T>
	constexpr Vector2<T> Vector2<T>::Project(const Vector2& Vector, const Vector2& To)
	{
		return Vector.Project(To);
	}

	template <Numeric T>
	constexpr Vector2<T> Vector2<T>::Project(const Vector2& To) const
	{
		return (To * (Dot(*this, To) / Dot(To, To)));
	}

	template <Numeric T>
	constexpr Vector2<T> Vector2<T>::Reject(const Vector2& Vector, const Vector2& From)
	{
		return Vector.Reject(From);
	}

	template <Numeric T>
	constexpr Vector2<T> Vector2<T>::Reject(const Vector2& From) const
	{
		return (*this - From * (Dot(*this, From) / Dot(From, From)));
	}

	template <Numeric T>
	constexpr float Vector2<T>::Length(const Vector2& Vector)
	{
		return Vector.Length();
	}

	template <Numeric T>
	constexpr float Vector2<T>::Length() const
	{
		return Sqrt(Length2());
	}

	template <Numeric T>
	constexpr T Vector2<T>::Length2(const Vector2& Vector)
	{
		return Vector.Length2();
	}

	template <Numeric T>
	constexpr T Vector2<T>::Length2() const
	{
		return (X * X) + (Y * Y);
	}

	template <Numeric T>
	constexpr float Vector2<T>::Distance(const Vector2& From, const Vector2& To)
	{
		return From.Distance(To);
	}

	template <Numeric T>
	constexpr float Vector2<T>::Distance(const Vector2& Other) const
	{
		return (*this - Other).Length();
	}

	template <Numeric T>
	constexpr T Vector2<T>::Distance2(const Vector2& From, const Vector2& To)
	{
		return From.Distance2(To);
	}

	template <Numeric T>
	constexpr T Vector2<T>::Distance2(const Vector2& Other) const
	{
		return (*this - Other).Length2();
	}

	template <Numeric T>
	constexpr Vector2<T> Vector2<T>::GetNormalized(const Vector2& Vector)
	{
		return Vector.GetNormalized();
	}

	template <Numeric T>
	constexpr Vector2<T> Vector2<T>::GetNormalized() const
	{
		Vector2<T> result = *this;
		result.Normalize();
		return result;
	}

	template <Numeric T>
	constexpr void Vector2<T>::Normalize()
	{
		const float length = Length();
		if (length != 0.0f)
		{
			X /= static_cast<T>(length);
			Y /= static_cast<T>(length);
		}
	}

	template <Numeric T>
	constexpr float Vector2<T>::Angle(const Vector2& Left, const Vector2& Right)
	{
		return Left.Angle(Right);
	}

	template <Numeric T>
	constexpr float Vector2<T>::AngleSigned(const Vector2& Left, const Vector2& Right)
	{
		return Left.AngleSigned(Right);
	}

	template <Numeric T>
	constexpr float Vector2<T>::Angle(const Vector2& Other) const
	{
		const T dot = Dot(Other);
		const float angle = Acos(dot / (Length() * Other.Length()));
		return angle;
	}

	template <Numeric T>
	constexpr float Vector2<T>::AngleSigned(const Vector2& Other) const
	{
		T atan2 = Atan2(Other.Y, Other.X) - std::atan2(Y, X);

		if (atan2 > PI)
		{
			atan2 -= TWO_PI;
		}
		else
		{
			atan2 += TWO_PI;
		}

		return atan2;
	}

	template <Numeric T>
	constexpr Vector2<T> Vector2<T>::Rotate(const Vector2& Vector, float Radians)
	{
		return Vector.Rotate(Radians);
	}

	template <Numeric T>
	constexpr Vector2<T> Vector2<T>::RotateAround(const Vector2& Vector, const Vector2& Pivot, float Radians)
	{
		return Vector.RotateAround(Pivot, Radians);
	}

	template <Numeric T>
	constexpr void Vector2<T>::Rotate(float Radians)
	{
		const float cos = Cos(Radians);
		const float sin = Sin(Radians);

		const T dx = X * cos - Y * sin;
		const T dy = X * sin + Y * cos;

		X = dx;
		Y = dy;
	}

	template <Numeric T>
	constexpr void Vector2<T>::RotateAround(const Vector2& Pivot, float Radians)
	{
		const float cos = Cos(Radians);
		const float sin = Sin(Radians);

		const Vector2<T> temp = *this - Pivot;

		X = temp.X * cos - temp.Y * sin + Pivot.X;
		Y = temp.X * sin + temp.Y * cos + Pivot.Y;
	}

	template <Numeric T>
	constexpr bool operator==(const Vector2<T>& Left, const Vector2<T>& Right)
	{
		return AreClose(Left.X, Right.X, 0.005f) && AreClose(Left.Y, Right.Y, 0.005f);
	}

	template<Numeric T>
	constexpr Vector2<T> operator-(const Vector2<T>& Vector)
	{
		return Vector2<T>(-Vector.X, -Vector.Y);
	}

	using Vector2F = Vector2<float>;
	using Vector2I = Vector2<int32_t>;
	using Vector2U = Vector2<uint32_t>;
}

template<Numeric T>
constexpr LE::Vector2<T> operator+(T Scalar, const LE::Vector2<T>& Vector2)
{
	return Vector2 + Scalar;
}

template<Numeric T>
constexpr LE::Vector2<T> operator-(T Scalar, const LE::Vector2<T>& Vector2)
{
	return Vector2 - Scalar;
}

template<Numeric T>
constexpr LE::Vector2<T> operator*(T Scalar, const LE::Vector2<T>& Vector2)
{
	return Vector2 * Scalar;
}

template<Numeric T>
constexpr LE::Vector2<T> operator/(T Scalar, const LE::Vector2<T>& Vector2)
{
	return Vector2 / Scalar;
}