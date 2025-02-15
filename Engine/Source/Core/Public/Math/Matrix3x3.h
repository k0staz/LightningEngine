#pragma once

#include "Vector3.h"

namespace LE
{
	template <Numeric T>
	struct Matrix3x3
	{
	public:
		static consteval Matrix3x3 Identity() { return Matrix3x3(); }

		constexpr Matrix3x3();
		constexpr Matrix3x3(const Vector3<T>& AxisX, const Vector3<T>& AxisY, const Vector3<T>& AxisZ);
		constexpr Matrix3x3(T M00, T M01, T M02,
		                    T M10, T M11, T M12,
		                    T M20, T M21, T M22);

		constexpr Matrix3x3<T> operator*(const Matrix3x3<T>& Other) const;
		constexpr Matrix3x3<T>& operator*=(const Matrix3x3<T>& Other);
		constexpr Vector2<T> operator*(const Vector2<T>& Vector) const;
		constexpr Vector3<T> operator*(const Vector3<T>& Vector) const;

		constexpr Vector3<T>& operator[](size_t Index) noexcept;
		constexpr const Vector3<T>& operator[](size_t Index) const noexcept;

		constexpr T Determinant() const;

		constexpr static Matrix3x3<T> GetTransposed(const Matrix3x3<T>& Matrix);
		constexpr void Transpose();

		constexpr static Matrix3x3<T> GetInverted(const Matrix3x3<T>& Matrix);
		constexpr void Invert();

		constexpr bool IsOrthogonal() const;

		constexpr static Matrix3x3 MakeRotationX(float Radians);
		constexpr static Matrix3x3 MakeRotationY(float Radians);
		constexpr static Matrix3x3 MakeRotationZ(float Radians);
		constexpr static Matrix3x3 MakeRotation(float Radians, const Vector3<T>& ArbitraryAxis);

		constexpr void RotateX(float Radians);
		constexpr void RotateY(float Radians);
		constexpr void RotateZ(float Radians);
		constexpr void Rotate(float Radians, const Vector3<T>& ArbitraryAxis);

		constexpr static Matrix3x3 MakeReflection(const Vector3<T>& Vector);
		constexpr static Matrix3x3 MakeInvolution(const Vector3<T>& Vector);

		constexpr void Reflect(const Vector3<T>& Vector);
		constexpr void Involute(const Vector3<T>& Vector);

		constexpr static Matrix3x3 MakeScale(float ScaleX, float ScaleY, float ScaleZ);
		constexpr static Matrix3x3 MakeScale(float Scale, const Vector3<T>& Direction);

		constexpr void Scale(float ScaleX, float ScaleY, float ScaleZ);
		constexpr void Scale(float Scale, const Vector3<T>& Direction);

		constexpr static Matrix3x3 MakeSkew(float Value, const Vector3<T>& DirectionA, const Vector3<T>& DirectionB);

		constexpr void Skew(float Value, const Vector3<T>& DirectionA, const Vector3<T>& DirectionB);

	public:
		Vector3<T> M[3];
	};

	template <Numeric T>
	constexpr Matrix3x3<T>::Matrix3x3()
		: M{{1, 0, 0}, {0, 1, 0}, {0, 0, 1}}
	{
	}

	template <Numeric T>
	constexpr Matrix3x3<T>::Matrix3x3(const Vector3<T>& AxisX, const Vector3<T>& AxisY, const Vector3<T>& AxisZ)
		: M{AxisX, AxisY, AxisZ}
	{
	}

	template <Numeric T>
	constexpr Matrix3x3<T>::Matrix3x3(T M00, T M01, T M02, T M10, T M11, T M12, T M20, T M21, T M22)
		: M{{M00, M01, M02}, {M10, M11, M12}, {M20, M21, M22}}
	{
	}

	template <Numeric T>
	constexpr Matrix3x3<T> Matrix3x3<T>::operator*(const Matrix3x3<T>& Other) const
	{
		Matrix3x3 result;
		for (size_t i = 0; i < 3; ++i)
		{
			for (size_t j = 0; j < 3; ++j)
			{
				result.M[j][i] = M[0][i] * Other.M[j][0]
					+ M[1][i] * Other.M[j][1]
					+ M[2][i] * Other.M[j][2];
			}
		}

		return result;
	}

	template <Numeric T>
	constexpr Matrix3x3<T>& Matrix3x3<T>::operator*=(const Matrix3x3<T>& Other)
	{
		*this = *this * Other;
		return *this;
	}

	template <Numeric T>
	constexpr Vector2<T> Matrix3x3<T>::operator*(const Vector2<T>& Vector) const
	{
		Vector3<T> result = *this * Vector3<T>(Vector, 1);

		return Vector2<T>(result.X, result.Y);
	}

	template <Numeric T>
	constexpr Vector3<T> Matrix3x3<T>::operator*(const Vector3<T>& Vector) const
	{
		Vector3<T> result;
		for (size_t i = 0; i < 3; ++i)
		{
			result[i] = M[0][i] * Vector[0]
				+ M[1][i] * Vector[1]
				+ M[2][i] * Vector[2];
		}

		return result;
	}

	template <Numeric T>
	constexpr Vector3<T>& Matrix3x3<T>::operator[](size_t Index) noexcept
	{
		return M[Index];
	}

	template <Numeric T>
	constexpr const Vector3<T>& Matrix3x3<T>::operator[](size_t Index) const noexcept
	{
		return M[Index];
	}

	template <Numeric T>
	constexpr T Matrix3x3<T>::Determinant() const
	{
		return M[0][0] * (M[1][1] * M[2][2] - M[1][2] * M[2][1])
			- M[0][1] * (M[1][0] * M[2][2] - M[1][2] * M[2][0])
			+ M[0][2] * (M[1][0] * M[2][1] - M[1][1] * M[2][0]);
	}

	template <Numeric T>
	constexpr Matrix3x3<T> Matrix3x3<T>::GetTransposed(const Matrix3x3<T>& Matrix)
	{
		Matrix3x3<T> result = Matrix;
		result.Transpose();
		return result;
	}

	template <Numeric T>
	constexpr void Matrix3x3<T>::Transpose()
	{
		Matrix3x3 temp(M[0][0], M[1][0], M[2][0],
		               M[0][1], M[1][1], M[2][1],
		               M[0][2], M[1][2], M[2][2]);
		*this = temp;
	}

	template <Numeric T>
	constexpr Matrix3x3<T> Matrix3x3<T>::GetInverted(const Matrix3x3<T>& Matrix)
	{
		Matrix3x3<T> result = Matrix;
		result.Invert();
		return result;
	}

	template <Numeric T>
	constexpr void Matrix3x3<T>::Invert()
	{
		const Vector3<T> a(M[0][0], M[1][0], M[2][0]);
		const Vector3<T> b(M[0][1], M[1][1], M[2][1]);
		const Vector3<T> c(M[0][2], M[1][2], M[2][2]);

		const Vector3<T>& r0 = Vector3<T>::Cross(b, c);
		const Vector3<T>& r1 = Vector3<T>::Cross(c, a);
		const Vector3<T>& r2 = Vector3<T>::Cross(a, b);

		const T det = Vector3<T>::Dot(r2, c);
		if (det == static_cast<T>(0))
		{
			return;
		}

		const T invDet = static_cast<T>(1) / det;

		Matrix3x3 temp(r0.X * invDet, r0.Y * invDet, r0.Z * invDet,
		               r1.X * invDet, r1.Y * invDet, r1.Z * invDet,
		               r2.X * invDet, r2.Y * invDet, r2.Z * invDet);

		*this = temp;
	}

	template <Numeric T>
	constexpr bool Matrix3x3<T>::IsOrthogonal() const
	{
		const Matrix3x3 inverse = GetInverted(*this);
		const Matrix3x3 transpose = GetTransposed(*this);
		return inverse == transpose;
	}

	template <Numeric T>
	constexpr Matrix3x3<T> Matrix3x3<T>::MakeRotationX(float Radians)
	{
		const T cos = static_cast<T>(Cos(Radians));
		const T sin = static_cast<T>(Sin(Radians));

		Matrix3x3 result(static_cast<T>(1), static_cast<T>(0), static_cast<T>(0),
		                 static_cast<T>(0), cos, -sin,
		                 static_cast<T>(0), sin, cos);

		result.Transpose();
		return result;
	}

	template <Numeric T>
	constexpr Matrix3x3<T> Matrix3x3<T>::MakeRotationY(float Radians)
	{
		const T cos = static_cast<T>(Cos(Radians));
		const T sin = static_cast<T>(Sin(Radians));

		Matrix3x3 result(cos, static_cast<T>(0), sin,
		                 static_cast<T>(0), static_cast<T>(1), static_cast<T>(0),
		                 -sin, static_cast<T>(0), cos);

		result.Transpose();
		return result;
	}

	template <Numeric T>
	constexpr Matrix3x3<T> Matrix3x3<T>::MakeRotationZ(float Radians)
	{
		const T cos = static_cast<T>(Cos(Radians));
		const T sin = static_cast<T>(Sin(Radians));

		Matrix3x3 result(cos, -sin, static_cast<T>(0),
		                 sin, cos, static_cast<T>(0),
		                 static_cast<T>(0), static_cast<T>(0), static_cast<T>(1));

		result.Transpose();
		return result;
	}

	template <Numeric T>
	constexpr Matrix3x3<T> Matrix3x3<T>::MakeRotation(float Radians, const Vector3<T>& ArbitraryAxis)
	{
		const T cos = static_cast<T>(Cos(Radians));
		const T sin = static_cast<T>(Sin(Radians));
		const T d = static_cast<T>(1) - cos;

		const T x = ArbitraryAxis.X * d;
		const T y = ArbitraryAxis.Y * d;
		const T z = ArbitraryAxis.Z * d;

		const T axay = x * ArbitraryAxis.Y;
		const T axaz = x * ArbitraryAxis.z;
		const T ayaz = y * ArbitraryAxis.z;

		Matrix3x3 result(cos + x * ArbitraryAxis.X, axay - sin * ArbitraryAxis.Z, axaz + sin * ArbitraryAxis.Y,
		                 axay + sin * ArbitraryAxis.Z, cos + y * ArbitraryAxis.Y, ayaz - sin * ArbitraryAxis.X,
		                 axaz - sin * ArbitraryAxis.Y, ayaz + sin * ArbitraryAxis.X, cos + z * ArbitraryAxis.Z);

		result.Transpose();
		return result;
	}

	template <Numeric T>
	constexpr void Matrix3x3<T>::RotateX(float Radians)
	{
		*this *= MakeRotationX(Radians);
	}

	template <Numeric T>
	constexpr void Matrix3x3<T>::RotateY(float Radians)
	{
		*this *= MakeRotationY(Radians);
	}

	template <Numeric T>
	constexpr void Matrix3x3<T>::RotateZ(float Radians)
	{
		*this *= MakeRotationZ(Radians);
	}

	template <Numeric T>
	constexpr void Matrix3x3<T>::Rotate(float Radians, const Vector3<T>& ArbitraryAxis)
	{
		*this *= MakeRotation(Radians, ArbitraryAxis);
	}

	template <Numeric T>
	constexpr Matrix3x3<T> Matrix3x3<T>::MakeReflection(const Vector3<T>& Vector)
	{
		const T x = Vector.X * static_cast<T>(-2);
		const T y = Vector.Y * static_cast<T>(-2);
		const T z = Vector.Z * static_cast<T>(-2);

		const T axay = x * Vector.Y;
		const T axaz = x * Vector.Z;
		const T ayaz = y * Vector.Z;

		Matrix3x3 result(x * Vector.X + static_cast<T>(1), axay, axaz,
		                 axay, y * Vector.Y + static_cast<T>(1), ayaz,
		                 axaz, ayaz, z * Vector.Z + static_cast<T>(1));

		result.Transpose();
		return result;
	}

	template <Numeric T>
	constexpr Matrix3x3<T> Matrix3x3<T>::MakeInvolution(const Vector3<T>& Vector)
	{
		const T x = Vector.X * static_cast<T>(2);
		const T y = Vector.Y * static_cast<T>(2);
		const T z = Vector.Z * static_cast<T>(2);

		const T axay = x * Vector.Y;
		const T axaz = x * Vector.Z;
		const T ayaz = y * Vector.Z;

		Matrix3x3 result(x * Vector.X - static_cast<T>(1), axay, axaz,
		                 axay, y * Vector.Y - static_cast<T>(1), ayaz,
		                 axaz, ayaz, z * Vector.Z - static_cast<T>(1));

		result.Transpose();
		return result;
	}

	template <Numeric T>
	constexpr void Matrix3x3<T>::Reflect(const Vector3<T>& Vector)
	{
		*this *= MakeReflection(Vector);
	}

	template <Numeric T>
	constexpr void Matrix3x3<T>::Involute(const Vector3<T>& Vector)
	{
		*this *= MakeInvolution(Vector);
	}

	template <Numeric T>
	constexpr Matrix3x3<T> Matrix3x3<T>::MakeScale(float ScaleX, float ScaleY, float ScaleZ)
	{
		Matrix3x3 result = Matrix3x3::Identity();
		result[0] *= ScaleX;
		result[1] *= ScaleY;
		result[2] *= ScaleZ;

		return result;
	}

	template <Numeric T>
	constexpr Matrix3x3<T> Matrix3x3<T>::MakeScale(float Scale, const Vector3<T>& Direction)
	{
		const T s = static_cast<T>(Scale - 1.0f);

		const T x = Direction.X * s;
		const T y = Direction.Y * s;
		const T z = Direction.Z * s;

		const T axay = x * Direction.Y;
		const T axaz = x * Direction.Z;
		const T ayaz = y * Direction.Z;

		Matrix3x3 result(x * Direction.X + static_cast<T>(1), axay, axaz,
		                 axay, y * Direction.Y + static_cast<T>(1), ayaz,
		                 axaz, ayaz, z * Direction.Z + static_cast<T>(1));
		result.Transpose();
		return result;
	}

	template <Numeric T>
	constexpr void Matrix3x3<T>::Scale(float ScaleX, float ScaleY, float ScaleZ)
	{
		*this *= MakeScale(ScaleX, ScaleY, ScaleZ);
	}

	template <Numeric T>
	constexpr void Matrix3x3<T>::Scale(float Scale, const Vector3<T>& Direction)
	{
		*this *= MakeScale(Scale, Direction);
	}

	template <Numeric T>
	constexpr Matrix3x3<T> Matrix3x3<T>::MakeSkew(float Value, const Vector3<T>& DirectionA, const Vector3<T>& DirectionB)
	{
		const T tan = static_cast<T>(Tan(Value));
		const T x = DirectionA.X * tan;
		const T y = DirectionA.Y * tan;
		const T z = DirectionA.Z * tan;

		Matrix3x3 result(x * DirectionB.X + static_cast<T>(1), x * DirectionB.Y, x * DirectionB.Z,
		                 y * DirectionB.X, y * DirectionB.Y + static_cast<T>(1), y * DirectionB.Z,
		                 z * DirectionB.X, z * DirectionB.Y, z * DirectionB.Z + static_cast<T>(1));
		result.Transpose();
		return result;
	}

	template <Numeric T>
	constexpr void Matrix3x3<T>::Skew(float Value, const Vector3<T>& DirectionA, const Vector3<T>& DirectionB)
	{
		*this *= MakeSkew(Value, DirectionA, DirectionB);
	}


	template <Numeric T>
	constexpr bool operator==(const Matrix3x3<T>& Left, const Matrix3x3<T>& Right)
	{
		return Left.M[0] == Right.M[0] && Left.M[1] == Right.M[1] && Left.M[2] == Right.M[2];
	}

	using Matrix3x3F = Matrix3x3<float>;
	using Matrix3x3I = Matrix3x3<int32_t>;
	using Matrix3x3U = Matrix3x3<uint32_t>;
}
