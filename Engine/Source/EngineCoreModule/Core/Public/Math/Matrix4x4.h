#pragma once

#include "Core.h"
#include "Vector4.h"

namespace LE
{
	template <Numeric T>
	struct Matrix4x4
	{
	public:
		static consteval Matrix4x4 Identity() { return Matrix4x4(); }

		static consteval Matrix4x4 Zero()
		{
			return Matrix4x4(Vector4<T>::Zero(), Vector4<T>::Zero(), Vector4<T>::Zero(), Vector4<T>::Zero());
		}

		constexpr Matrix4x4();
		constexpr Matrix4x4(const Vector4<T>& AxisX, const Vector4<T>& AxisY, const Vector4<T>& AxisZ, const Vector4<T>& Point);
		constexpr Matrix4x4(T M00, T M01, T M02, T M03,
		                    T M10, T M11, T M12, T M13,
		                    T M20, T M21, T M22, T M23,
		                    T M30, T M31, T M32, T M33);

		constexpr Matrix4x4<T> operator*(const Matrix4x4<T>& Other) const;
		constexpr Matrix4x4<T>& operator*=(const Matrix4x4<T>& Other);
		constexpr Vector3<T> operator*(const Vector3<T>& Vector) const;
		constexpr Vector4<T> operator*(const Vector4<T>& Vector) const;

		constexpr Vector4<T>& operator[](size_t Index) noexcept;
		constexpr const Vector4<T>& operator[](size_t Index) const noexcept;

		constexpr static Matrix4x4<T> GetTranslation(const Vector3<T>& Vector);
		constexpr static Matrix4x4<T> GetTranslation(const T ValueX, const T ValueY, const T ValueZ);

		constexpr static Matrix4x4<T> MakeTranslation(const T ValueX, const T ValueY, const T ValueZ);

		constexpr void Translate(const Vector3<T>& Vector);
		constexpr void Translate(const T ValueX, const T ValueY, const T ValueZ);

		constexpr void SetPosition(const T ValueX, const T ValueY, const T ValueZ);
		constexpr void SetPosition(const Vector3<T>& Vector);

		constexpr Vector3<T> GetPosition() const;

		constexpr static Matrix4x4<T> GetTransposed(const Matrix4x4<T>& Matrix);
		constexpr void Transpose();

		constexpr static Matrix4x4<T> GetInverted(const Matrix4x4<T>& Matrix);
		constexpr void Invert();

		constexpr T Determinant() const;

		constexpr bool IsOrthogonal() const;

		constexpr static Matrix4x4 MakeRotationX(float Radians);
		constexpr static Matrix4x4 MakeRotationY(float Radians);
		constexpr static Matrix4x4 MakeRotationZ(float Radians);
		constexpr static Matrix4x4 MakeRotation(float Radians, const Vector3<T>& ArbitraryAxis);
		constexpr static Matrix4x4 MakeRotation(float Pitch, float Yaw, float Roll);
		constexpr static Matrix4x4 MakeRotation(const Vector3<T>& Radians);

		constexpr void RotateX(float Radians);
		constexpr void RotateY(float Radians);
		constexpr void RotateZ(float Radians);
		constexpr void Rotate(float Radians, const Vector3<T>& ArbitraryAxis);
		constexpr void Rotate(float Pitch, float Yaw, float Roll);
		constexpr void Rotate(const Vector3<T>& Radians);

		constexpr void SetRotationMatrix(Matrix4x4 RotationMatrix);

		constexpr void RotateSelfX(float Radians);
		constexpr void RotateSelfY(float Radians);
		constexpr void RotateSelfZ(float Radians);

		constexpr Vector3<T> GetRotation() const;
		constexpr Matrix4x4 GetRotationMatrix() const;

		constexpr static Matrix4x4 MakeReflection(const Vector3<T>& Vector);
		constexpr static Matrix4x4 MakeInvolution(const Vector3<T>& Vector);

		constexpr void Reflect(const Vector3<T>& Vector);
		constexpr void Involute(const Vector3<T>& Vector);

		constexpr static Matrix4x4 MakeScale(float ScaleX, float ScaleY, float ScaleZ);
		constexpr static Matrix4x4 MakeScale(const Vector3<T>& Scale);
		constexpr static Matrix4x4 MakeScale(float Scale, const Vector3<T>& Direction);

		constexpr Vector3<T> GetScale() const;

		constexpr void Scale(float ScaleX, float ScaleY, float ScaleZ);
		constexpr void Scale(const Vector3<T>& Scale);
		constexpr void Scale(float Scale, const Vector3<T>& Direction);

		constexpr static Matrix4x4 MakeSkew(float Value, const Vector3<T>& DirectionA, const Vector3<T>& DirectionB);

		constexpr void Skew(float Value, const Vector3<T>& DirectionA, const Vector3<T>& DirectionB);

	public:
		Vector4<T> M[4];
	};

	template <Numeric T>
	constexpr Matrix4x4<T>::Matrix4x4()
		: M{
			{1, 0, 0, 0},
			{0, 1, 0, 0},
			{0, 0, 1, 0},
			{0, 0, 0, 1}
		}
	{
	}

	template <Numeric T>
	constexpr Matrix4x4<T>::Matrix4x4(const Vector4<T>& AxisX, const Vector4<T>& AxisY, const Vector4<T>& AxisZ, const Vector4<T>& Point)
		: M{AxisX, AxisY, AxisZ, Point}
	{
	}

	template <Numeric T>
	constexpr Matrix4x4<T>::Matrix4x4(T M00, T M01, T M02, T M03, T M10, T M11, T M12, T M13, T M20, T M21, T M22, T M23, T M30, T M31,
	                                  T M32, T M33)
		: M{
			{M00, M01, M02, M03},
			{M10, M11, M12, M13},
			{M20, M21, M22, M23},
			{M30, M31, M32, M33}
		}
	{
	}

	template <Numeric T>
	constexpr Matrix4x4<T> Matrix4x4<T>::operator*(const Matrix4x4<T>& Other) const
	{
		Matrix4x4 result;
		for (size_t i = 0; i < 4; ++i)
		{
			for (size_t j = 0; j < 4; ++j)
			{
				result.M[i][j] = M[0][i] * Other.M[j][0] +
					M[1][i] * Other.M[j][1] +
					M[2][i] * Other.M[j][2] +
					M[3][i] * Other.M[j][3];
			}
		}

		return result;
	}

	template <Numeric T>
	constexpr Matrix4x4<T>& Matrix4x4<T>::operator*=(const Matrix4x4<T>& Other)
	{
		*this = *this * Other;
		return *this;
	}

	template <Numeric T>
	constexpr Vector3<T> Matrix4x4<T>::operator*(const Vector3<T>& Vector) const
	{
		Vector4<T> result = *this * Vector4<T>(Vector, static_cast<T>(1));
		return Vector3<T>(result.X, result.Y, result.Z);
	}

	template <Numeric T>
	constexpr Vector4<T> Matrix4x4<T>::operator*(const Vector4<T>& Vector) const
	{
		Vector4<T> result;
		for (size_t i = 0; i < 4; ++i)
		{
			result[i] = M[0][i] * Vector[0] +
				M[1][i] * Vector[1] +
				M[2][i] * Vector[2] +
				M[3][i] * Vector[3];
		}
		return result;
	}

	template <Numeric T>
	constexpr Vector4<T>& Matrix4x4<T>::operator[](size_t Index) noexcept
	{
		return M[Index];
	}

	template <Numeric T>
	constexpr const Vector4<T>& Matrix4x4<T>::operator[](size_t Index) const noexcept
	{
		return M[Index];
	}

	template <Numeric T>
	constexpr Matrix4x4<T> Matrix4x4<T>::GetTranslation(const Vector3<T>& Vector)
	{
		Matrix4x4<T> result = Matrix4x4::Identity();
		result[3] = Vector4<T>(Vector, static_cast<T>(1));
		return result;
	}

	template <Numeric T>
	constexpr Matrix4x4<T> Matrix4x4<T>::GetTranslation(const T ValueX, const T ValueY, const T ValueZ)
	{
		Matrix4x4<T> result = Matrix4x4::Identity();
		result[3] = Vector4<T>(ValueX, ValueY, ValueZ, static_cast<T>(1));
		return result;
	}

	template <Numeric T>
	constexpr Matrix4x4<T> Matrix4x4<T>::MakeTranslation(const T ValueX, const T ValueY, const T ValueZ)
	{
		Matrix4x4<T> result = Matrix4x4::Identity();
		result.M[3][0] = ValueX;
		result.M[3][1] = ValueY;
		result.M[3][2] = ValueZ;
		return result;
	}

	template <Numeric T>
	constexpr void Matrix4x4<T>::Translate(const Vector3<T>& Vector)
	{
		*this *= MakeTranslation(Vector.X, Vector.Y, Vector.Z);
	}

	template <Numeric T>
	constexpr void Matrix4x4<T>::Translate(const T ValueX, const T ValueY, const T ValueZ)
	{
		*this *= MakeTranslation(ValueX, ValueY, ValueZ);
	}

	template <Numeric T>
	constexpr void Matrix4x4<T>::SetPosition(const T ValueX, const T ValueY, const T ValueZ)
	{
		M[3][0] = ValueX;
		M[3][1] = ValueY;
		M[3][2] = ValueZ;
	}

	template <Numeric T>
	constexpr void Matrix4x4<T>::SetPosition(const Vector3<T>& Vector)
	{
		M[3][0] = Vector.X;
		M[3][1] = Vector.Y;
		M[3][2] = Vector.Z;
	}

	template <Numeric T>
	constexpr Vector3<T> Matrix4x4<T>::GetPosition() const
	{
		return Vector3<T>(M[3][0], M[3][1], M[3][2]);
	}

	template <Numeric T>
	constexpr Matrix4x4<T> Matrix4x4<T>::GetTransposed(const Matrix4x4<T>& Matrix)
	{
		Matrix4x4<T> result = Matrix;
		result.Transpose();
		return result;
	}

	template <Numeric T>
	constexpr void Matrix4x4<T>::Transpose()
	{
		Matrix4x4 temp(M[0][0], M[1][0], M[2][0], M[3][0],
		               M[0][1], M[1][1], M[2][1], M[3][1],
		               M[0][2], M[1][2], M[2][2], M[3][2],
		               M[0][3], M[1][3], M[2][3], M[3][3]);
		*this = temp;
	}

	template <Numeric T>
	constexpr Matrix4x4<T> Matrix4x4<T>::GetInverted(const Matrix4x4<T>& Matrix)
	{
		Matrix4x4<T> temp = Matrix;
		temp.Invert();
		return temp;
	}

	template <Numeric T>
	constexpr void Matrix4x4<T>::Invert()
	{
		const Vector3<T> a(M[0][0], M[1][0], M[2][0]);
		const Vector3<T> b(M[0][1], M[1][1], M[2][1]);
		const Vector3<T> c(M[0][2], M[1][2], M[2][2]);
		const Vector3<T> d(M[0][3], M[1][3], M[2][3]);

		const T& x = M[3][0];
		const T& y = M[3][1];
		const T& z = M[3][2];
		const T& w = M[3][3];

		Vector3<T> s = Vector3<T>::Cross(a, b);
		Vector3<T> t = Vector3<T>::Cross(c, d);
		Vector3<T> u = a * y - b * x;
		Vector3<T> v = c * w - d * z;

		const T det = Vector3<T>::Dot(s, v) + Vector3<T>::Dot(t, u);
		if (det == static_cast<T>(0))
		{
			return;
		}

		const T invDet = static_cast<T>(1) / det;
		s *= invDet;
		t *= invDet;
		u *= invDet;
		v *= invDet;

		Vector3<T> r0 = Vector3<T>::Cross(b, v) + t * y;
		Vector3<T> r1 = Vector3<T>::Cross(v, a) - t * x;
		Vector3<T> r2 = Vector3<T>::Cross(d, u) + s * w;
		Vector3<T> r3 = Vector3<T>::Cross(u, c) - s * z;

		Matrix4x4 temp(r0.X, r0.Y, r0.Z, -Vector3<T>::Dot(b, t),
		               r1.X, r1.Y, r1.Z, Vector3<T>::Dot(a, t),
		               r2.X, r2.Y, r2.Z, -Vector3<T>::Dot(d, s),
		               r3.X, r3.Y, r3.Z, Vector3<T>::Dot(c, s));

		*this = temp;
	}

	template <Numeric T>
	constexpr T Matrix4x4<T>::Determinant() const
	{
		const Vector3<T> a(M[0][0], M[1][0], M[2][0]);
		const Vector3<T> b(M[0][1], M[1][1], M[2][1]);
		const Vector3<T> c(M[0][2], M[1][2], M[2][2]);
		const Vector3<T> d(M[0][3], M[1][3], M[2][3]);

		const T& x = M[3][0];
		const T& y = M[3][1];
		const T& z = M[3][2];
		const T& w = M[3][3];

		Vector3<T> s = Vector3<T>::Cross(a, b);
		Vector3<T> t = Vector3<T>::Cross(c, d);
		Vector3<T> u = a * y - b * x;
		Vector3<T> v = c * w - d * z;

		return Vector3<T>::Dot(s, v) + Vector3<T>::Dot(t, u);
	}

	template <Numeric T>
	constexpr bool Matrix4x4<T>::IsOrthogonal() const
	{
		const Matrix4x4<T> inverse = GetInverted(*this);
		const Matrix4x4<T> transpose = GetTransposed(*this);

		return inverse == transpose;
	}

	template <Numeric T>
	constexpr Matrix4x4<T> Matrix4x4<T>::MakeRotationX(float Radians)
	{
		const T cos = static_cast<T>(Cos(Radians));
		const T sin = static_cast<T>(Sin(Radians));

		Matrix4x4 result(static_cast<T>(1), static_cast<T>(0), static_cast<T>(0), static_cast<T>(0),
		                 static_cast<T>(0), cos, -sin, static_cast<T>(0),
		                 static_cast<T>(0), sin, cos, static_cast<T>(0),
		                 static_cast<T>(0), static_cast<T>(0), static_cast<T>(0), static_cast<T>(1));

		result.Transpose();
		return result;
	}

	template <Numeric T>
	constexpr Matrix4x4<T> Matrix4x4<T>::MakeRotationY(float Radians)
	{
		const T cos = static_cast<T>(Cos(Radians));
		const T sin = static_cast<T>(Sin(Radians));

		Matrix4x4 result(cos, static_cast<T>(0), sin, static_cast<T>(0),
		                 static_cast<T>(0), static_cast<T>(1), static_cast<T>(0), static_cast<T>(0),
		                 -sin, static_cast<T>(0), cos, static_cast<T>(0),
		                 static_cast<T>(0), static_cast<T>(0), static_cast<T>(0), static_cast<T>(1));

		result.Transpose();
		return result;
	}

	template <Numeric T>
	constexpr Matrix4x4<T> Matrix4x4<T>::MakeRotationZ(float Radians)
	{
		const T cos = static_cast<T>(Cos(Radians));
		const T sin = static_cast<T>(Sin(Radians));

		Matrix4x4 result(cos, -sin, static_cast<T>(0), static_cast<T>(0),
		                 sin, cos, static_cast<T>(0), static_cast<T>(0),
		                 static_cast<T>(0), static_cast<T>(0), static_cast<T>(1), static_cast<T>(0),
		                 static_cast<T>(0), static_cast<T>(0), static_cast<T>(0), static_cast<T>(1));

		result.Transpose();
		return result;
	}

	template <Numeric T>
	constexpr Matrix4x4<T> Matrix4x4<T>::MakeRotation(float Radians, const Vector3<T>& ArbitraryAxis)
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

		Matrix4x4 result(cos + x * ArbitraryAxis.X, axay - sin * ArbitraryAxis.Z, axaz + sin * ArbitraryAxis.Y, static_cast<T>(0),
		                 axay + sin * ArbitraryAxis.Z, cos + y * ArbitraryAxis.Y, ayaz - sin * ArbitraryAxis.X, static_cast<T>(0),
		                 axaz - sin * ArbitraryAxis.Y, ayaz + sin * ArbitraryAxis.X, cos + z * ArbitraryAxis.Z, static_cast<T>(0),
		                 static_cast<T>(0), static_cast<T>(0), static_cast<T>(0), static_cast<T>(1));

		result.Transpose();

		return result;
	}

	template <Numeric T>
	constexpr Matrix4x4<T> Matrix4x4<T>::MakeRotation(float Pitch, float Yaw, float Roll)
	{
		return MakeRotationX(Pitch) * MakeRotationY(Yaw) * MakeRotationZ(Roll);
	}

	template <Numeric T>
	constexpr Matrix4x4<T> Matrix4x4<T>::MakeRotation(const Vector3<T>& Radians)
	{
		return MakeRotationX(Radians.X) * MakeRotationY(Radians.Y) * MakeRotationZ(Radians.Z);
	}

	template <Numeric T>
	constexpr void Matrix4x4<T>::RotateX(float Radians)
	{
		*this *= MakeRotationX(Radians);
	}

	template <Numeric T>
	constexpr void Matrix4x4<T>::RotateY(float Radians)
	{
		*this *= MakeRotationY(Radians);
	}

	template <Numeric T>
	constexpr void Matrix4x4<T>::RotateZ(float Radians)
	{
		*this *= MakeRotationZ(Radians);
	}

	template <Numeric T>
	constexpr void Matrix4x4<T>::Rotate(float Radians, const Vector3<T>& ArbitraryAxis)
	{
		*this *= MakeRotation(Radians, ArbitraryAxis);
	}

	template <Numeric T>
	constexpr void Matrix4x4<T>::Rotate(float Pitch, float Yaw, float Roll)
	{
		*this *= MakeRotation(Pitch, Yaw, Roll);
	}

	template <Numeric T>
	constexpr void Matrix4x4<T>::Rotate(const Vector3<T>& Radians)
	{
		*this *= MakeRotation(Radians);
	}

	template <Numeric T>
	constexpr void Matrix4x4<T>::SetRotationMatrix(Matrix4x4 RotationMatrix)
	{
		const Vector3<T> scale = GetScale();

		M[0] = RotationMatrix[0] * scale.X;
		M[1] = RotationMatrix[1] * scale.Y;
		M[2] = RotationMatrix[2] * scale.Z;
	}

	template <Numeric T>
	constexpr void Matrix4x4<T>::RotateSelfX(float Radians)
	{
		Matrix4x4<T> rotationMatrix = GetRotationMatrix();
		rotationMatrix *= MakeRotationX(Radians);
		SetRotationMatrix(rotationMatrix);
	}

	template <Numeric T>
	constexpr void Matrix4x4<T>::RotateSelfY(float Radians)
	{
		Matrix4x4<T> rotationMatrix = GetRotationMatrix();
		rotationMatrix *= MakeRotationY(Radians);
		SetRotationMatrix(rotationMatrix);
	}

	template <Numeric T>
	constexpr void Matrix4x4<T>::RotateSelfZ(float Radians)
	{
		Matrix4x4<T> rotationMatrix = GetRotationMatrix();
		rotationMatrix *= MakeRotationZ(Radians);
		SetRotationMatrix(rotationMatrix);
	}

	template <Numeric T>
	constexpr Vector3<T> Matrix4x4<T>::GetRotation() const
	{
		const Vector3<T> scale = GetScale();
		Matrix4x4<T> rotation = *this;
		rotation[0] = Vector4<T>(M[0][0] / scale.x, M[0][1] / scale.x, M[0][2] / scale.x, 0);
		rotation[1] = Vector4<T>(M[1][0] / scale.y, M[1][1] / scale.y, M[1][2] / scale.y, 0);
		rotation[2] = Vector4<T>(M[2][0] / scale.z, M[2][1] / scale.z, M[2][2] / scale.z, 0);
		rotation[3] = Vector4<T>(0, 0, 0, 1);

		const T sy = Sqrt(rotation[0][0] * rotation[0][0] + rotation[1][0] * rotation[1][0]);
		if (sy > Constants<T>::CEpsilon)
		{
			const T x = -Atan2(rotation[2][1], rotation[2][2]);
			const T y = -Atan2(-rotation[2][0], sy);
			const T z = -Atan2(rotation[1][0], rotation[0][0]);
			return Vector3<T>(x, y, z);
		}
		else
		{
			LE_ASSERT_DESC(false, "Gimbal Lock");
			return Vector3<T>(0);
		}
	}

	template <Numeric T>
	constexpr Matrix4x4<T> Matrix4x4<T>::GetRotationMatrix() const
	{
		const Vector3<T> scale = GetScale();
		Matrix4x4<T> result = Identity();
		result[0] = M[0] / scale.X;
		result[1] = M[1] / scale.Y;
		result[2] = M[2] / scale.Z;

		return result;
	}

	template <Numeric T>
	constexpr Matrix4x4<T> Matrix4x4<T>::MakeReflection(const Vector3<T>& Vector)
	{
		const T x = Vector.X * static_cast<T>(-2);
		const T y = Vector.Y * static_cast<T>(-2);
		const T z = Vector.Z * static_cast<T>(-2);

		const T axay = x * Vector.Y;
		const T axaz = x * Vector.Z;
		const T ayaz = y * Vector.Z;

		Matrix4x4 result(x * Vector.X + static_cast<T>(1), axay, axaz, static_cast<T>(0),
		                 axay, y * Vector.Y + static_cast<T>(1), ayaz, static_cast<T>(0),
		                 axaz, ayaz, z * Vector.Z + static_cast<T>(1), static_cast<T>(0),
		                 static_cast<T>(0), static_cast<T>(0), static_cast<T>(0), static_cast<T>(1));
		result.Transpose();
		return result;
	}

	template <Numeric T>
	constexpr Matrix4x4<T> Matrix4x4<T>::MakeInvolution(const Vector3<T>& Vector)
	{
		const T x = Vector.X * static_cast<T>(2);
		const T y = Vector.Y * static_cast<T>(2);
		const T z = Vector.Z * static_cast<T>(2);

		const T axay = x * Vector.Y;
		const T axaz = x * Vector.Z;
		const T ayaz = y * Vector.Z;

		Matrix4x4 result(x * Vector.X - static_cast<T>(1), axay, axaz, static_cast<T>(0),
		                 axay, y * Vector.Y - static_cast<T>(1), ayaz, static_cast<T>(0),
		                 axaz, ayaz, z * Vector.Z - static_cast<T>(1), static_cast<T>(0),
		                 static_cast<T>(0), static_cast<T>(0), static_cast<T>(0), static_cast<T>(1));

		result.Transpose();
		return result;
	}

	template <Numeric T>
	constexpr void Matrix4x4<T>::Reflect(const Vector3<T>& Vector)
	{
		*this *= MakeReflection(Vector);
	}

	template <Numeric T>
	constexpr void Matrix4x4<T>::Involute(const Vector3<T>& Vector)
	{
		*this *= MakeInvolution(Vector);
	}

	template <Numeric T>
	constexpr Matrix4x4<T> Matrix4x4<T>::MakeScale(float ScaleX, float ScaleY, float ScaleZ)
	{
		Matrix4x4 result = Matrix4x4::Identity();
		result[0] *= ScaleX;
		result[1] *= ScaleY;
		result[2] *= ScaleZ;

		return result;
	}

	template <Numeric T>
	constexpr Matrix4x4<T> Matrix4x4<T>::MakeScale(const Vector3<T>& Scale)
	{
		Matrix4x4 result = Matrix4x4::Identity();
		result[0] *= Scale.X;
		result[1] *= Scale.Y;
		result[2] *= Scale.Z;

		return result;
	}

	template <Numeric T>
	constexpr Matrix4x4<T> Matrix4x4<T>::MakeScale(float Scale, const Vector3<T>& Direction)
	{
		const T s = static_cast<T>(Scale - 1.0f);

		const T x = Direction.X * s;
		const T y = Direction.Y * s;
		const T z = Direction.Z * s;

		const T axay = x * Direction.Y;
		const T axaz = x * Direction.Z;
		const T ayaz = y * Direction.Z;

		Matrix4x4 result(x * Direction.X + static_cast<T>(1), axay, axaz, static_cast<T>(0),
		                 axay, y * Direction.Y + static_cast<T>(1), ayaz, static_cast<T>(0),
		                 axaz, ayaz, z * Direction.Z + static_cast<T>(1), static_cast<T>(0),
		                 static_cast<T>(0), static_cast<T>(0), static_cast<T>(0), static_cast<T>(1));

		result.Transpose();
		return result;
	}

	template <Numeric T>
	constexpr Vector3<T> Matrix4x4<T>::GetScale() const
	{
		const T x = Vector3<T>(M[0][0], M[0][1], M[0][2]).Length();
		const T y = Vector3<T>(M[1][0], M[1][1], M[1][2]).Length();
		const T z = Vector3<T>(M[2][0], M[2][1], M[2][2]).Length();
		return Vector3<T>(x, y, z);
	}

	template <Numeric T>
	constexpr void Matrix4x4<T>::Scale(float ScaleX, float ScaleY, float ScaleZ)
	{
		*this *= MakeScale(ScaleX, ScaleY, ScaleZ);
	}

	template <Numeric T>
	constexpr void Matrix4x4<T>::Scale(const Vector3<T>& Scale)
	{
		*this *= MakeScale(Scale);
	}

	template <Numeric T>
	constexpr void Matrix4x4<T>::Scale(float Scale, const Vector3<T>& Direction)
	{
		*this *= MakeScale(Scale, Direction);
	}

	template <Numeric T>
	constexpr Matrix4x4<T> Matrix4x4<T>::MakeSkew(float Value, const Vector3<T>& DirectionA, const Vector3<T>& DirectionB)
	{
		const T tan = static_cast<T>(Tan(Value));
		const T x = DirectionA.X * tan;
		const T y = DirectionA.Y * tan;
		const T z = DirectionA.Z * tan;

		Matrix4x4 result(x * DirectionB.X + static_cast<T>(1), x * DirectionB.Y, x * DirectionB.Z, static_cast<T>(0),
		                 y * DirectionB.X, y * DirectionB.Y + static_cast<T>(1), y * DirectionB.Z, static_cast<T>(0),
		                 z * DirectionB.X, z * DirectionB.Y, z * DirectionB.Z + static_cast<T>(1), static_cast<T>(0),
		                 static_cast<T>(0), static_cast<T>(0), static_cast<T>(0), static_cast<T>(1));
		result.Transpose();
		return result;
	}

	template <Numeric T>
	constexpr void Matrix4x4<T>::Skew(float Value, const Vector3<T>& DirectionA, const Vector3<T>& DirectionB)
	{
		*this *= MakeSkew(Value, DirectionA, DirectionB);
	}


	template <Numeric T>
	constexpr bool operator==(const Matrix4x4<T>& Left, const Matrix4x4<T>& Right)
	{
		return Left.M[0] == Right.M[0] && Left.M[1] == Right.M[1] && Left.M[2] == Right.M[2];
	}

	using Matrix4x4F = Matrix4x4<float>;
	using Matrix4x4I = Matrix4x4<int32_t>;
	using Matrix4x4U = Matrix4x4<uint32_t>;
}
