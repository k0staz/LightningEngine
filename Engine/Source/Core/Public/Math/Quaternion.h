#pragma once

#include "Vector3.h"
#include "Matrix3x3.h"

namespace LE
{
	template <Numeric T>
	struct Quaternion
	{
	public:
		static consteval Quaternion Identity() { return Quaternion(); }

	public:
		constexpr Quaternion() = default;
		constexpr Quaternion(T ValueX, T ValueY, T ValueZ, T ValueW);
		constexpr Quaternion(Vector3<T> Vector, T Scalar);

		constexpr Quaternion operator*(const Quaternion& Other) const;
		constexpr Quaternion& operator*=(const Quaternion& Other);

		constexpr const Vector3<T>& GetVectorPart() const;
		constexpr Matrix3x3<T> GetRotationMatrix() const;

		constexpr void SetRotationMatrix(const Matrix3x3<T>& Matrix);

		constexpr Vector3<T> Rotate(const Vector3<T>& Vector);

	public:
		T X = static_cast<T>(0);
		T Y = static_cast<T>(0);
		T Z = static_cast<T>(0);
		T W = static_cast<T>(1);
	};

	template <Numeric T>
	constexpr Quaternion<T>::Quaternion(T ValueX, T ValueY, T ValueZ, T ValueW)
		: X(ValueX), Y(ValueY), Z(ValueZ), W(ValueW)
	{
	}

	template <Numeric T>
	constexpr Quaternion<T>::Quaternion(Vector3<T> Vector, T Scalar)
		: X(Vector.X), Y(Vector.Y), Z(Vector.Z), W(Scalar)
	{
	}

	template <Numeric T>
	constexpr Quaternion<T> Quaternion<T>::operator*(const Quaternion& Other) const
	{
		const T x = W * Other.X + X * Other.W + Y * Other.Z - Z * Other.Y;
		const T y = W * Other.Y - X * Other.Z + Y * Other.W + Z * Other.X;
		const T z = W * Other.Z + X * Other.Y - Y * Other.X + Z * Other.W;
		const T w = W * Other.W - X * Other.X - Y * Other.Y - Z * Other.Z;

		return {x, y, z, w};
	}

	template <Numeric T>
	constexpr Quaternion<T>& Quaternion<T>::operator*=(const Quaternion& Other)
	{
		*this = *this * Other;
		return *this;
	}

	template <Numeric T>
	constexpr const Vector3<T>& Quaternion<T>::GetVectorPart() const
	{
		return (*reinterpret_cast<const Vector3<T>*>(&X));
	}

	template <Numeric T>
	constexpr Matrix3x3<T> Quaternion<T>::GetRotationMatrix() const
	{
		const T x2 = X * X;
		const T y2 = Y * Y;
		const T z2 = Z * Z;
		const T xy = X * Y;
		const T xz = X * Z;
		const T yz = Y * Z;
		const T wx = W * X;
		const T wy = W * Y;
		const T wz = W * Z;

		Matrix3x3<T> result(static_cast<T>(1) - static_cast<T>(2) * (y2 + z2), static_cast<T>(2) * (xy - wz), static_cast<T>(2) * (xz + wy),
		                    static_cast<T>(2) * (xy + wz), static_cast<T>(1) - static_cast<T>(2) * (x2 + z2), static_cast<T>(2) * (yz - wx),
		                    static_cast<T>(2) * (xz - wy), static_cast<T>(2) * (yz + wx),
		                    static_cast<T>(1) - static_cast<T>(2) * (x2 + y2));

		result.Transpose();
		return result;
	}

	template <Numeric T>
	constexpr void Quaternion<T>::SetRotationMatrix(const Matrix3x3<T>& Matrix)
	{
		const Matrix3x3<T> temp = Matrix3x3<T>::GetTransposed(Matrix);
		const T m00 = temp[0][0];
		const T m11 = temp[1][1];
		const T m22 = temp[2][2];

		const T sum = m00 + m11 + m22;
		if (sum > static_cast<T>(0))
		{
			W = Sqrt(sum + static_cast<T>(1)) * 0.5f;
			const float f = 0.25f / W;
			X = (temp[2][1] - temp[1][2]) * f;
			Y = (temp[0][2] - temp[2][0]) * f;
			Z = (temp[1][0] - temp[0][1]) * f;
		}
		else if ((temp[0][0] > temp[1][1]) && (temp[0][0] > temp[2][2]))
		{
			X = Sqrt(m00 - m11 - m22 + static_cast<T>(1)) * 0.5f;
			const float f = 0.25f / X;
			Y = (temp[1][0] - temp[0][1]) * f;
			Z = (temp[0][2] - temp[2][0]) * f;
			W = (temp[2][1] - temp[1][2]) * f;
		}
		else if (temp[1][1] > temp[2][2])
		{
			Y = Sqrt(m11 - m00 - m22 + static_cast<T>(1)) * 0.5f;
			const float f = 0.25f / Y;
			X = (temp[1][0] - temp[0][1]) * f;
			Z = (temp[2][1] - temp[1][2]) * f;
			W = (temp[0][2] - temp[2][0]) * f;
		}
		else
		{
			Z = Sqrt(m22 - m00 - m11 + static_cast<T>(1)) * 0.5f;
			const float f = 0.25f / Y;
			X = (temp[0][2] - temp[2][0]) * f;
			Y = (temp[2][1] - temp[1][2]) * f;
			W = (temp[1][0] - temp[0][1]) * f;
		}
	}

	template <Numeric T>
	constexpr Vector3<T> Quaternion<T>::Rotate(const Vector3<T>& Vector)
	{
		const Vector3<T>& b = GetVectorPart();
		const T b2 = b.Length2();

		return (Vector * (W * W - b2) + b * (Vector3<T>::Dot(Vector, b) * static_cast<T>(2)) + Vector3<T>::Cross(b, Vector) * (W *
			static_cast<T>(2)));
	}

	template <Numeric T>
	constexpr bool operator==(const Quaternion<T>& Left, const Quaternion<T>& Right)
	{
		return AreClose(Left.X, Right.X, 0.005f)
			&& AreClose(Left.Y, Right.Y, 0.005f)
			&& AreClose(Left.Z, Right.Z, 0.005f)
			&& AreClose(Left.W, Right.W, 0.005f);
	}

	using QuaternionF = Quaternion<float>;
}
