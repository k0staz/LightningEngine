#pragma once

#include "Math.h"
#include "Vector2.h"

namespace LE
{
	template <Numeric T>
	struct Vector3
	{
	public:
		static consteval Vector3 Zero() { return Vector3(static_cast<T>(0)); }
		static consteval Vector3 One() { return Vector3(static_cast<T>(1)); }
		static consteval Vector3 Up() { return Vector3(static_cast<T>(0), static_cast<T>(1), static_cast<T>(0)); }
		static consteval Vector3 Down() { return Vector3(static_cast<T>(0), static_cast<T>(-1), static_cast<T>(0)); }
		static consteval Vector3 Left() { return Vector3(static_cast<T>(-1), static_cast<T>(0), static_cast<T>(0)); }
		static consteval Vector3 Right() { return Vector3(static_cast<T>(1), static_cast<T>(0), static_cast<T>(0)); }
		static consteval Vector3 Forward() { return Vector3(static_cast<T>(0), static_cast<T>(0), static_cast<T>(1)); }
		static consteval Vector3 Backward() { return Vector3(static_cast<T>(0), static_cast<T>(0), static_cast<T>(-1)); }

		constexpr Vector3() = default;
		constexpr Vector3(T Value);
		constexpr Vector3(T XValue, T YValue, T ZValue);
		constexpr Vector3(const Vector2<T>& Vector2, T ZValue);

		constexpr Vector3 operator+(const Vector3& Other) const;
		constexpr Vector3 operator-(const Vector3& Other) const;
		constexpr Vector3 operator*(const Vector3& Other) const;
		constexpr Vector3 operator/(const Vector3& Other) const;

		constexpr Vector3& operator+=(const Vector3& Other);
		constexpr Vector3& operator-=(const Vector3& Other);
		constexpr Vector3& operator*=(const Vector3& Other);
		constexpr Vector3& operator/=(const Vector3& Other);

		constexpr Vector3 operator+(T Scalar) const;
		constexpr Vector3 operator-(T Scalar) const;
		constexpr Vector3 operator*(T Scalar) const;
		constexpr Vector3 operator/(T Scalar) const;

		constexpr Vector3& operator+=(T Scalar);
		constexpr Vector3& operator-=(T Scalar);
		constexpr Vector3& operator*=(T Scalar);
		constexpr Vector3& operator/=(T Scalar);

		constexpr T& operator[](size_t Index) noexcept;
		constexpr const T& operator[](size_t Index) const noexcept;

		constexpr static T Dot(const Vector3& Left, const Vector3& Right);
		constexpr T Dot(const Vector3& Other) const;

		constexpr static Vector3 Cross(const Vector3& Left, const Vector3& Right);
		constexpr Vector3 Cross(const Vector3& Other) const;

		constexpr static Vector3 Project(const Vector3& Vector, const Vector3& To);
		constexpr Vector3 Project(const Vector3& To) const;

		constexpr static Vector3 Reject(const Vector3& Vector, const Vector3& From);
		constexpr Vector3 Reject(const Vector3& From) const;

		constexpr static float Length(const Vector3& Vector);
		constexpr float Length() const;

		constexpr static T Length2(const Vector3& Vector);
		constexpr T Length2() const;

		constexpr static float Distance(const Vector3& From, const Vector3& To);
		constexpr float Distance(const Vector3& Other) const;

		constexpr static T Distance2(const Vector3& From, const Vector3& To);
		constexpr T Distance2(const Vector3& Other) const;

		constexpr static Vector3 GetNormalized(const Vector3& Vector);
		constexpr Vector3 GetNormalized() const;
		constexpr void Normalize();

		constexpr static float Angle(const Vector3& Left, const Vector3& Right);
		constexpr static float AngleSigned(const Vector3& Left, const Vector3& Right, const Vector3& Reference);
		constexpr float Angle(const Vector3& Other) const;
		constexpr float AngleSigned(const Vector3& Other) const;

	public:
		T X = static_cast<T>(0);
		T Y = static_cast<T>(0);
		T Z = static_cast<T>(0);
	};

	template <Numeric T>
	constexpr Vector3<T>::Vector3(T Value)
		: X(Value)
		  , Y(Value)
		  , Z(Value)
	{
	}

	template <Numeric T>
	constexpr Vector3<T>::Vector3(T XValue, T YValue, T ZValue)
		: X(XValue)
		  , Y(YValue)
		  , Z(ZValue)
	{
	}

	template <Numeric T>
	constexpr Vector3<T>::Vector3(const Vector2<T>& Vector2, T ZValue)
		: X(Vector2.X)
		  , Y(Vector2.Y)
		  , Z(ZValue)
	{
	}

	template <Numeric T>
	constexpr Vector3<T> Vector3<T>::operator+(const Vector3& Other) const
	{
		return Vector3(X + Other.X, Y + Other.Y, Z + Other.Z);
	}

	template <Numeric T>
	constexpr Vector3<T> Vector3<T>::operator-(const Vector3& Other) const
	{
		return Vector3(X - Other.X, Y - Other.Y, Z - Other.Z);
	}

	template <Numeric T>
	constexpr Vector3<T> Vector3<T>::operator*(const Vector3& Other) const
	{
		return Vector3(X * Other.X, Y * Other.Y, Z * Other.Z);
	}

	template <Numeric T>
	constexpr Vector3<T> Vector3<T>::operator/(const Vector3& Other) const
	{
		return Vector3(X / Other.X, Y / Other.Y, Z / Other.Z);
	}

	template <Numeric T>
	constexpr Vector3<T>& Vector3<T>::operator+=(const Vector3& Other)
	{
		X += Other.X;
		Y += Other.Y;
		Z += Other.Z;
		return *this;
	}

	template <Numeric T>
	constexpr Vector3<T>& Vector3<T>::operator-=(const Vector3& Other)
	{
		X -= Other.X;
		Y -= Other.Y;
		Z -= Other.Z;
		return *this;
	}

	template <Numeric T>
	constexpr Vector3<T>& Vector3<T>::operator*=(const Vector3& Other)
	{
		X *= Other.X;
		Y *= Other.Y;
		Z *= Other.Z;
		return *this;
	}

	template <Numeric T>
	constexpr Vector3<T>& Vector3<T>::operator/=(const Vector3& Other)
	{
		X /= Other.X;
		Y /= Other.Y;
		Z /= Other.Z;
		return *this;
	}

	template <Numeric T>
	constexpr Vector3<T> Vector3<T>::operator+(T Scalar) const
	{
		return Vector3(X + Scalar, Y + Scalar, Z + Scalar);
	}

	template <Numeric T>
	constexpr Vector3<T> Vector3<T>::operator-(T Scalar) const
	{
		return Vector3(X - Scalar, Y - Scalar, Z - Scalar);
	}

	template <Numeric T>
	constexpr Vector3<T> Vector3<T>::operator*(T Scalar) const
	{
		return Vector3(X * Scalar, Y * Scalar, Z * Scalar);
	}

	template <Numeric T>
	constexpr Vector3<T> Vector3<T>::operator/(T Scalar) const
	{
		return Vector3(X / Scalar, Y / Scalar, Z / Scalar);
	}

	template <Numeric T>
	constexpr Vector3<T>& Vector3<T>::operator+=(T Scalar)
	{
		X += Scalar;
		Y += Scalar;
		Z += Scalar;
		return *this;
	}

	template <Numeric T>
	constexpr Vector3<T>& Vector3<T>::operator-=(T Scalar)
	{
		X -= Scalar;
		Y -= Scalar;
		Z -= Scalar;
		return *this;
	}

	template <Numeric T>
	constexpr Vector3<T>& Vector3<T>::operator*=(T Scalar)
	{
		X *= Scalar;
		Y *= Scalar;
		Z *= Scalar;
		return *this;
	}

	template <Numeric T>
	constexpr Vector3<T>& Vector3<T>::operator/=(T Scalar)
	{
		X /= Scalar;
		Y /= Scalar;
		Z /= Scalar;
		return *this;
	}

	template <Numeric T>
	constexpr T& Vector3<T>::operator[](size_t Index) noexcept
	{
		return (&X)[Index];
	}

	template <Numeric T>
	constexpr const T& Vector3<T>::operator[](size_t Index) const noexcept
	{
		return (&X)[Index];
	}

	template <Numeric T>
	constexpr T Vector3<T>::Dot(const Vector3& Left, const Vector3& Right)
	{
		return Left.Dot(Right);
	}

	template <Numeric T>
	constexpr T Vector3<T>::Dot(const Vector3& Other) const
	{
		return (X * Other.X + Y * Other.Y + Z * Other.Z);
	}

	template <Numeric T>
	constexpr Vector3<T> Vector3<T>::Cross(const Vector3& Left, const Vector3& Right)
	{
		return Left.Cross(Right);
	}

	template <Numeric T>
	constexpr Vector3<T> Vector3<T>::Cross(const Vector3& Other) const
	{
		return Vector3(Y * Other.Z - Z * Other.Y,
		               Z * Other.X - X * Other.Z,
		               X * Other.Y - Y * Other.X);
	}

	template <Numeric T>
	constexpr Vector3<T> Vector3<T>::Project(const Vector3& Vector, const Vector3& To)
	{
		return Vector.Project(To);
	}

	template <Numeric T>
	constexpr Vector3<T> Vector3<T>::Project(const Vector3& To) const
	{
		return (To * (Dot(*this, To) / Dot(To, To)));
	}

	template <Numeric T>
	constexpr Vector3<T> Vector3<T>::Reject(const Vector3& Vector, const Vector3& From)
	{
		return Vector.Reject(From);
	}

	template <Numeric T>
	constexpr Vector3<T> Vector3<T>::Reject(const Vector3& From) const
	{
		return (*this - From * (Dot(*this, From) / Dot(From, From)));
	}

	template <Numeric T>
	constexpr float Vector3<T>::Length(const Vector3& Vector)
	{
		return Vector.Length();
	}

	template <Numeric T>
	constexpr float Vector3<T>::Length() const
	{
		return Sqrt(Length2());
	}

	template <Numeric T>
	constexpr T Vector3<T>::Length2(const Vector3& Vector)
	{
		return Vector.Length2();
	}

	template <Numeric T>
	constexpr T Vector3<T>::Length2() const
	{
		return (X * X + Y * Y + Z * Z);
	}

	template <Numeric T>
	constexpr float Vector3<T>::Distance(const Vector3& From, const Vector3& To)
	{
		return From.Distance(To);
	}

	template <Numeric T>
	constexpr float Vector3<T>::Distance(const Vector3& Other) const
	{
		return (*this - Other).Length();
	}

	template <Numeric T>
	constexpr T Vector3<T>::Distance2(const Vector3& From, const Vector3& To)
	{
		return From.Distance2(To);
	}

	template <Numeric T>
	constexpr T Vector3<T>::Distance2(const Vector3& Other) const
	{
		return (*this - Other).Length2();
	}

	template <Numeric T>
	constexpr Vector3<T> Vector3<T>::GetNormalized(const Vector3& Vector)
	{
		return Vector.GetNormalized();
	}

	template <Numeric T>
	constexpr Vector3<T> Vector3<T>::GetNormalized() const
	{
		Vector3<T> result = *this;
		result.Normalize();
		return result;
	}

	template <Numeric T>
	constexpr void Vector3<T>::Normalize()
	{
		const float length = Length();
		if (length != 0.0f)
		{
			X /= static_cast<T>(length);
			Y /= static_cast<T>(length);
			Z /= static_cast<T>(length);
		}
	}

	template <Numeric T>
	constexpr float Vector3<T>::Angle(const Vector3& Left, const Vector3& Right)
	{
		const Vector3<T> a = Left.GetNormalized();
		const Vector3<T> b = Right.GetNormalized();

		const float dot = static_cast<float>(a.Dot(b));

		return Acos(dot);
	}

	template <Numeric T>
	constexpr float Vector3<T>::AngleSigned(const Vector3& Left, const Vector3& Right, const Vector3& Reference)
	{
		const Vector3<T> a = Left.GetNormalized();
		const Vector3<T> b = Right.GetNormalized();
		const Vector3<T> cross = a.Cross(b);

		const T dotAB = a.Dot(b);
		const T dotCrossRef = Reference.Dot(Cross());

		return Atan2(dotCrossRef, dotAB);
	}

	template <Numeric T>
	constexpr float Vector3<T>::Angle(const Vector3& Other) const
	{
		return Angle(*this, Other);
	}

	template <Numeric T>
	constexpr float Vector3<T>::AngleSigned(const Vector3& Other) const
	{
		return AngleSigned(*this, Other);
	}

	template <Numeric T>
	constexpr bool operator==(const Vector3<T>& Left, const Vector3<T>& Right)
	{
		return AreClose(Left.X, Right.X, 0.005f)
			&& AreClose(Left.Y, Right.Y, 0.005f)
			&& AreClose(Left.Z, Right.Z, 0.005f);
	}

	template <Numeric T>
	constexpr Vector3<T> operator-(const Vector3<T>& Vector)
	{
		return Vector3<T>(-Vector.X, -Vector.Y, -Vector.Z);
	}

	using Vector3F = Vector3<float>;
	using Vector3I = Vector3<int32_t>;
	using Vector3U = Vector3<uint32_t>;
}

template<Numeric T>
constexpr LE::Vector3<T> operator+(T Scalar, const LE::Vector3<T>& Vector)
{
	return Vector + Scalar;
}

template<Numeric T>
constexpr LE::Vector3<T> operator-(T Scalar, const LE::Vector3<T>& Vector)
{
	return Vector - Scalar;
}

template<Numeric T>
constexpr LE::Vector3<T> operator*(T Scalar, const LE::Vector3<T>& Vector)
{
	return Vector * Scalar;
}

template<Numeric T>
constexpr LE::Vector3<T> operator/(T Scalar, const LE::Vector3<T>& Vector)
{
	return Vector / Scalar;
}