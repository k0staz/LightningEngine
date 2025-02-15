#pragma once

#include "Math/Math.h"

#include "Vector3.h"

namespace LE
{
	template <Numeric T>
	struct Vector4
	{
	public:
		static consteval Vector4 Zero() { return Vector4(static_cast<T>(0)); }
		static consteval Vector4 One() { return Vector4(static_cast<T>(1)); }
		static consteval Vector4 Up() { return Vector4(static_cast<T>(0), static_cast<T>(1), static_cast<T>(0), static_cast<T>(0)); }
		static consteval Vector4 Down() { return Vector4(static_cast<T>(0), static_cast<T>(-1), static_cast<T>(0), static_cast<T>(0)); }
		static consteval Vector4 Left() { return Vector4(static_cast<T>(-1), static_cast<T>(0), static_cast<T>(0), static_cast<T>(0)); }
		static consteval Vector4 Right() { return Vector4(static_cast<T>(1), static_cast<T>(0), static_cast<T>(0), static_cast<T>(0)); }
		static consteval Vector4 Forward() { return Vector4(static_cast<T>(0), static_cast<T>(0), static_cast<T>(1), static_cast<T>(0)); }
		static consteval Vector4 Backward() { return Vector4(static_cast<T>(0), static_cast<T>(0), static_cast<T>(-1), static_cast<T>(0)); }

		constexpr Vector4() = default;
		constexpr Vector4(T Value);
		constexpr Vector4(T XValue, T YValue, T ZValue, T WValue);
		constexpr Vector4(const Vector3<T>& Vector3, T WValue);

		constexpr Vector4 operator+(const Vector4& Other) const;
		constexpr Vector4 operator-(const Vector4& Other) const;
		constexpr Vector4 operator*(const Vector4& Other) const;
		constexpr Vector4 operator/(const Vector4& Other) const;

		constexpr Vector4& operator+=(const Vector4& Other);
		constexpr Vector4& operator-=(const Vector4& Other);
		constexpr Vector4& operator*=(const Vector4& Other);
		constexpr Vector4& operator/=(const Vector4& Other);

		constexpr Vector4 operator+(T Scalar) const;
		constexpr Vector4 operator-(T Scalar) const;
		constexpr Vector4 operator*(T Scalar) const;
		constexpr Vector4 operator/(T Scalar) const;

		constexpr Vector4& operator+=(T Scalar);
		constexpr Vector4& operator-=(T Scalar);
		constexpr Vector4& operator*=(T Scalar);
		constexpr Vector4& operator/=(T Scalar);

		constexpr T& operator[](size_t Index) noexcept;
		constexpr const T& operator[](size_t Index) const noexcept;

		constexpr T Dot(const Vector4& Other) const;
		constexpr T DotW(const Vector4& Other) const;
		constexpr Vector3<T> Cross(const Vector4& Other) const;

		constexpr float Length() const;
		constexpr float LengthW() const;

		constexpr T Length2() const;
		constexpr T Length2W() const;

		constexpr bool IsDir() const;
		constexpr bool IsPos() const;

		constexpr Vector3<T> XYZ() const;

	public:
		T X = static_cast<T>(0);
		T Y = static_cast<T>(0);
		T Z = static_cast<T>(0);
		T W = static_cast<T>(0);
	};

	template <Numeric T>
	constexpr Vector4<T>::Vector4(T Value)
		: X(Value)
		  , Y(Value)
		  , Z(Value)
		  , W(static_cast<T>(1))
	{
	}

	template <Numeric T>
	constexpr Vector4<T>::Vector4(T XValue, T YValue, T ZValue, T WValue)
		: X(XValue)
		, Y(YValue)
		, Z(ZValue)
		, W(WValue)
	{
	}

	template <Numeric T>
	constexpr Vector4<T>::Vector4(const Vector3<T>& Vector3, T WValue)
		: X(Vector3.X)
		, Y(Vector3.Y)
		, Z(Vector3.Z)
		, W(WValue)
	{
	}

	template <Numeric T>
	constexpr Vector4<T> Vector4<T>::operator+(const Vector4& Other) const
	{
		return Vector4(X + Other.X, Y + Other.Y, Z + Other.Z, W + Other.W);
	}

	template <Numeric T>
	constexpr Vector4<T> Vector4<T>::operator-(const Vector4& Other) const
	{
		return Vector4(X - Other.X, Y - Other.Y, Z - Other.Z, W - Other.W);
	}

	template <Numeric T>
	constexpr Vector4<T> Vector4<T>::operator*(const Vector4& Other) const
	{
		return Vector4(X * Other.X, Y * Other.Y, Z * Other.Z, W * Other.W);
	}

	template <Numeric T>
	constexpr Vector4<T> Vector4<T>::operator/(const Vector4& Other) const
	{
		return Vector4(X / Other.X, Y / Other.Y, Z / Other.Z, W / Other.W);
	}

	template <Numeric T>
	constexpr Vector4<T>& Vector4<T>::operator+=(const Vector4& Other)
	{
		X += Other.X;
		Y += Other.Y;
		Z += Other.Z;
		W += Other.W;
		return *this;
	}

	template <Numeric T>
	constexpr Vector4<T>& Vector4<T>::operator-=(const Vector4& Other)
	{
		X -= Other.X;
		Y -= Other.Y;
		Z -= Other.Z;
		W -= Other.W;
		return *this;
	}

	template <Numeric T>
	constexpr Vector4<T>& Vector4<T>::operator*=(const Vector4& Other)
	{
		X *= Other.X;
		Y *= Other.Y;
		Z *= Other.Z;
		W *= Other.W;
		return *this;
	}

	template <Numeric T>
	constexpr Vector4<T>& Vector4<T>::operator/=(const Vector4& Other)
	{
		X /= Other.X;
		Y /= Other.Y;
		Z /= Other.Z;
		W /= Other.W;
		return *this;
	}

	template <Numeric T>
	constexpr Vector4<T> Vector4<T>::operator+(T Scalar) const
	{
		return Vector4(X + Scalar, Y + Scalar, Z + Scalar, W + Scalar);
	}

	template <Numeric T>
	constexpr Vector4<T> Vector4<T>::operator-(T Scalar) const
	{
		return Vector4(X - Scalar, Y - Scalar, Z - Scalar, W - Scalar);
	}

	template <Numeric T>
	constexpr Vector4<T> Vector4<T>::operator*(T Scalar) const
	{
		return Vector4(X * Scalar, Y * Scalar, Z * Scalar, W * Scalar);
	}

	template <Numeric T>
	constexpr Vector4<T> Vector4<T>::operator/(T Scalar) const
	{
		return Vector4(X / Scalar, Y / Scalar, Z / Scalar, W / Scalar);
	}

	template <Numeric T>
	constexpr Vector4<T>& Vector4<T>::operator+=(T Scalar)
	{
		X += Scalar;
		Y += Scalar;
		Z += Scalar;
		W += Scalar;
		return *this;
	}

	template <Numeric T>
	constexpr Vector4<T>& Vector4<T>::operator-=(T Scalar)
	{
		X -= Scalar;
		Y -= Scalar;
		Z -= Scalar;
		W -= Scalar;
		return *this;
	}

	template <Numeric T>
	constexpr Vector4<T>& Vector4<T>::operator*=(T Scalar)
	{
		X *= Scalar;
		Y *= Scalar;
		Z *= Scalar;
		W *= Scalar;
		return *this;
	}

	template <Numeric T>
	constexpr Vector4<T>& Vector4<T>::operator/=(T Scalar)
	{
		X /= Scalar;
		Y /= Scalar;
		Z /= Scalar;
		W /= Scalar;
		return *this;
	}

	template <Numeric T>
	constexpr T& Vector4<T>::operator[](size_t Index) noexcept
	{
		return (&X)[Index];
	}

	template <Numeric T>
	constexpr const T& Vector4<T>::operator[](size_t Index) const noexcept
	{
		return (&X)[Index];
	}

	template <Numeric T>
	constexpr T Vector4<T>::Dot(const Vector4& Other) const
	{
		return XYZ().Dot(Other.XYZ());
	}

	template <Numeric T>
	constexpr T Vector4<T>::DotW(const Vector4& Other) const
	{
		return (X * Other.X + Y * Other.Y + Z * Other.Z + W * Other.W);
	}

	template <Numeric T>
	constexpr Vector3<T> Vector4<T>::Cross(const Vector4& Other) const
	{
		return XYZ().Cross(Other.XYZ());
	}

	template <Numeric T>
	constexpr float Vector4<T>::Length() const
	{
		return XYZ().Length();
	}

	template <Numeric T>
	constexpr float Vector4<T>::LengthW() const
	{
		return Sqrt(Length2W());
	}

	template <Numeric T>
	constexpr T Vector4<T>::Length2() const
	{
		return XYZ().Length2();
	}

	template <Numeric T>
	constexpr T Vector4<T>::Length2W() const
	{
		return (X * X + Y * Y + Z * Z + W * W);
	}

	template <Numeric T>
	constexpr bool Vector4<T>::IsDir() const
	{
		return W == static_cast<T>(0);
	}

	template <Numeric T>
	constexpr bool Vector4<T>::IsPos() const
	{
		return W > static_cast<T>(0);
	}

	template <Numeric T>
	constexpr Vector3<T> Vector4<T>::XYZ() const
	{
		return Vector3<T>(X, Y, Z);
	}


	template <Numeric T>
	constexpr bool operator==(const Vector4<T>& Left, const Vector4<T>& Right)
	{
		return AreClose(Left.X, Right.X, 0.005f)
			&& AreClose(Left.Y, Right.Y, 0.005f)
			&& AreClose(Left.Z, Right.Z, 0.005f)
			&& AreClose(Left.W, Right.W, 0.005f);
	}

	template <Numeric T>
	constexpr Vector4<T> operator-(const Vector4<T>& Vector)
	{
		return Vector4<T>(-Vector.X, -Vector.Y, -Vector.Z, -Vector.W);
	}

	using Vector4F = Vector4<float>;
	using Vector4I = Vector4<int32_t>;
	using Vector4U = Vector4<uint32_t>;
}

template<Numeric T>
constexpr LE::Vector4<T> operator+(T Scalar, const LE::Vector4<T>& Vector)
{
	return Vector + Scalar;
}

template<Numeric T>
constexpr LE::Vector4<T> operator-(T Scalar, const LE::Vector4<T>& Vector)
{
	return Vector - Scalar;
}

template<Numeric T>
constexpr LE::Vector4<T> operator*(T Scalar, const LE::Vector4<T>& Vector)
{
	return Vector * Scalar;
}

template<Numeric T>
constexpr LE::Vector4<T> operator/(T Scalar, const LE::Vector4<T>& Vector)
{
	return Vector / Scalar;
}