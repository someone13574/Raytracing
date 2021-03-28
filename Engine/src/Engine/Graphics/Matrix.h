#pragma once

#include "Vector.h"

template <typename MatrixType> class Matrix2x2
{
	static_assert(std::is_arithmetic<VectorType>::value, "Matrix type must be numeric.");
public:
	Matrix2x2(MatrixType x0, MatrixType y0, MatrixType x1, MatrixType y1) : x0(x0), y0(y0), x1(x1), y1(y1) {};
	template <typename Type> Matrix2x2(Vector2<Type> v0, Vector2<Type> v1) : x0(v0.x), y0(v0.y), x1(v1.x), y1(v1.y) {};
	template <typename Type> operator Matrix2x2<Type>() { static_assert(std::is_arithmetic<Type>::value, "Cannot convert to a matrix of this type"); return Matrix2x2<Type>((Type)x0, (Type)y0, (Type)x1, (Type)y1); }
public:
	template <typename Type> Matrix2x2<MatrixType> operator+(Matrix2x2<Type> matrix) { static_assert(std::is_arithmetic<Type>::value, "Matrix type must be numeric."); return Matrix2x2<MatrixType>(x0 + (MatrixType)matrix.x0, y0 + (MatrixType)matrix.y0, x1 + (MatrixType)matrix.x1, y1 + (MatrixType)matrix.y1); }
	template <typename Type> Matrix2x2<MatrixType> operator+=(Matrix2x2<Type> matrix) { static_assert(std::is_arithmetic<Type>::value, "Matrix type must be numeric."); *this = Matrix2x2<MatrixType>(x0 + (MatrixType)matrix.x0, y0 + (MatrixType)matrix.y0, x1 + (MatrixType)matrix.x1, y1 + (MatrixType)matrix.y1); return *this; }
public:
	template <typename Type> Matrix2x2<MatrixType> operator-(Matrix2x2<Type> matrix) { static_assert(std::is_arithmetic<Type>::value, "Matrix type must be numeric."); return Matrix2x2<MatrixType>(x0 - (MatrixType)matrix.x0, y0 - (MatrixType)matrix.y0, x1 - (MatrixType)matrix.x1, y1 - (MatrixType)matrix.y1); }
	template <typename Type> Matrix2x2<MatrixType> operator-=(Matrix2x2<Type> matrix) { static_assert(std::is_arithmetic<Type>::value, "Matrix type must be numeric."); *this = Matrix2x2<MatrixType>(x0 - (MatrixType)matrix.x0, y0 - (MatrixType)matrix.y0, x1 - (MatrixType)matrix.x1, y1 - (MatrixType)matrix.y1); return *this; }
public:
	template <typename Type> Matrix2x2<MatrixType> operator*(Type value) { static_assert(std::is_arithmetic<Type>::value, "Scalar type must be numeric."); return Matrix2x2<MatrixType>(x0 * (MatrixType)value, y0 * (MatrixType)value, x1 * (MatrixType)value, y1 * (MatrixType)value); }
	template <typename Type> Matrix2x2<MatrixType> operator*(Matrix2x2<Type> matrix) { static_assert(std::is_arithmetic<Type>::value, "Matrix type must be numeric."); return Matrix2x2<MatrixType>(x0 * (MatrixType)matrix.x0 + y0 * (MatrixType)matrix.x1, x0 * (MatrixType)matrix.y0 + y0 * (MatrixType)matrix.y1, x1 * (MatrixType)matrix.x0 + y1 * (MatrixType)matrix.x1, x1 * (MatrixType)matrix.y0 + y1 * (MatrixType)matrix.y1); }
	template <typename Type> Matrix2x2<MatrixType> operator*=(Type value) { static_assert(std::is_arithmetic<Type>::value, "Scalar type must be numeric."); *this = Matrix2x2<MatrixType>(x0 * value, y0 * value, x1 * value, y1 * value); return *this; }
	template <typename Type> Matrix2x2<MatrixType> operator*=(Matrix2x2<Type> matrix) { static_assert(std::is_arithmetic<Type>::value, "Matrix type must be numeric."); *this = Matrix2x2<MatrixType>(x0 * (MatrixType)matrix.x0 + y0 * (MatrixType)matrix.x1, x0 * (MatrixType)matrix.y0 + y0 * (MatrixType)matrix.y1, x1 * (MatrixType)matrix.x0 + y1 * (MatrixType)matrix.x1, x1 * (MatrixType)matrix.y0 + y1 * (MatrixType)matrix.y1); return *this; }
public:
	template <typename Type> Matrix2x2<MatrixType> operator/(Type value) { static_assert(std::is_arithmetic<Type>::value, "Scalar type must be numeric."); return Matrix2x2<MatrixType>(x0 / (MatrixType)value, y0 / (MatrixType)value, x1 / (MatrixType)value, y1 / (MatrixType)value); }
	template <typename Type> Matrix2x2<MatrixType> operator/=(Type value) { static_assert(std::is_arithmetic<Type>::value, "Scalar type must be numeric."); *this = Matrix2x2<MatrixType>(x0 / value, y0 / value, x1 / value, y1 / value); return *this; }
public:
	MatrixType x0, y0;
	MatrixType x1, y1;
public:
	struct GPUMatrix2x2
	{
		float x0, y0, x1, y1;
	};
	GPUMatrix2x2 GetGpuMatrix() { return { (float)x0, (float)y0, (float)x1, (float)y1 }; }
};

template <typename MatrixType> class Matrix3x3
{
	static_assert(std::is_arithmetic<MatrixType>::value, "Matrix type must be numeric.");
public:
	Matrix3x3(MatrixType x0, MatrixType y0, MatrixType z0, MatrixType x1, MatrixType y1, MatrixType z1, MatrixType x2, MatrixType y2, MatrixType z2) : x0(x0), y0(y0), z0(z0), x1(x1), y1(y1), z1(z1), x2(x2), y2(y2), z2(z2) {};
	template<typename Type> Matrix3x3(Vector3<Type> v0, Vector3<Type> v1, Vector3<Type> v2) : x0(v0.x), y0(v0.y), z0(v0.z), x1(v1.x), y1(v1.y), z1(v1.z), x2(v2.x), y2(v2.y), z2(v2.z) {};
	template <typename Type> operator Matrix3x3<Type>() { static_assert(std::is_arithmetic<Type>::value, "Cannot convert to a matrix of this type"); return Matrix3x3<Type>((Type)x0, (Type)y0, (Type)z0, (Type)x1, (Type)y1, (Type)z1, (Type)x2, (Type)y2, (Type)z2); }
public:
	template<typename Type> Matrix3x3<MatrixType> operator+(Matrix3x3<Type> matrix) { static_assert(std::is_arithmetic<Type>::value, "Matrix type must be numeric."); return Matrix3x3<MatrixType>(x0 + (MatrixType)matrix.x0, y0 + (MatrixType)matrix.y0, z0 + (MatrixType)matrix.z0, x1 + (MatrixType)matrix.x1, y1 + (MatrixType)matrix.y1, z1 + (MatrixType)matrix.z1, x2 + (MatrixType)matrix.x2, y2 + (MatrixType)matrix.y2, z2 + (MatrixType)matrix.z2); }
	template<typename Type> Matrix3x3<MatrixType> operator+=(Matrix3x3<Type> matrix) { static_assert(std::is_arithmetic<Type>::value, "Matrix type must be numeric."); *this = Matrix3x3<MatrixType>(x0 + (MatrixType)matrix.x0, y0 + (MatrixType)matrix.y0, z0 + (MatrixType)matrix.z0, x1 + (MatrixType)matrix.x1, y1 + (MatrixType)matrix.y1, z1 + (MatrixType)matrix.z1, x2 + (MatrixType)matrix.x2, y2 + (MatrixType)matrix.y2, z2 + (MatrixType)matrix.z2); return *this; }
public:
	template<typename Type> Matrix3x3<MatrixType> operator-(Matrix3x3<Type> matrix) { static_assert(std::is_arithmetic<Type>::value, "Matrix type must be numeric."); return Matrix3x3<MatrixType>(x0 - (MatrixType)matrix.x0, y0 - (MatrixType)matrix.y0, z0 - (MatrixType)matrix.z0, x1 - (MatrixType)matrix.x1, y1 - (MatrixType)matrix.y1, z1 - (MatrixType)matrix.z1, x2 - (MatrixType)matrix.x2, y2 - (MatrixType)matrix.y2, z2 - (MatrixType)matrix.z2); }
	template<typename Type> Matrix3x3<MatrixType> operator-=(Matrix3x3<Type> matrix) { static_assert(std::is_arithmetic<Type>::value, "Matrix type must be numeric."); *this = Matrix3x3<MatrixType>(x0 - (MatrixType)matrix.x0, y0 - (MatrixType)matrix.y0, z0 - (MatrixType)matrix.z0, x1 - (MatrixType)matrix.x1, y1 - (MatrixType)matrix.y1, z1 - (MatrixType)matrix.z1, x2 - (MatrixType)matrix.x2, y2 - (MatrixType)matrix.y2, z2 - (MatrixType)matrix.z2); return *this; }
public:
	template<typename Type> Matrix3x3<MatrixType> operator*(Type scalar) { static_assert(std::is_arithmetic<Type>::value, "Scalar type must be numeric."); return Matrix3x3<MatrixType>(x0 * scalar, y0 * scalar, z0 * scalar, x1 * scalar, y1 * scalar, z1 * scalar, x2 * scalar, y2 * scalar, z2 * scalar); }
	template<typename Type> Matrix3x3<MatrixType> operator*(Matrix3x3<Type> matrix) { static_assert(std::is_arithmetic<Type>::value, "Matrix type must be numeric."); return Matrix3x3<MatrixType>(x0 * (Type)matrix.x0 + y0 * (Type)matrix.x1 + z0 * (Type)matrix.x2, x0 * (Type)matrix.y0 + y0 * (Type)matrix.y1 + z0 * (Type)matrix.y2, x0 * (Type)matrix.z0 + y0 * (Type)matrix.z1 + z0 * (Type)matrix.z2, x1 * (Type)matrix.x0 + y1 * (Type)matrix.x1 + z1 * (Type)matrix.x2, x1 * (Type)matrix.y0 + y1 * (Type)matrix.y1 + z1 * (Type)matrix.y2, x1 * (Type)matrix.z0 + y1 * (Type)matrix.z1 + z1 * (Type)matrix.z2, x2 * (Type)matrix.x0 + y2 * (Type)matrix.x1 + z2 * (Type)matrix.x2, x2 * (Type)matrix.y0 + y2 * (Type)matrix.y1 + z2 * (Type)matrix.y2, x2 * (Type)matrix.z0 + y2 * (Type)matrix.z1 + z2 * (Type)matrix.z2); }
	template<typename Type> Matrix3x3<MatrixType> operator*=(Type scalar) { static_assert(std::is_arithmetic<Type>::value, "Scalar type must be numeric."); *this = Matrix3x3<MatrixType>(x0 * scalar, y0 * scalar, z0 * scalar, x1 * scalar, y1 * scalar, z1 * scalar, x2 * scalar, y2 * scalar, z2 * scalar); return *this; }
	template<typename Type> Matrix3x3<MatrixType> operator*=(Matrix3x3<Type> matrix) { static_assert(std::is_arithmetic<Type>::value, "Matrix type must be numeric."); *this = Matrix3x3<MatrixType>(x0 * (Type)matrix.x0 + y0 * (Type)matrix.x1 + z0 * (Type)matrix.x2, x0 * (Type)matrix.y0 + y0 * (Type)matrix.y1 + z0 * (Type)matrix.y2, x0 * (Type)matrix.z0 + y0 * (Type)matrix.z1 + z0 * (Type)matrix.z2, x1 * (Type)matrix.x0 + y1 * (Type)matrix.x1 + z1 * (Type)matrix.x2, x1 * (Type)matrix.y0 + y1 * (Type)matrix.y1 + z1 * (Type)matrix.y2, x1 * (Type)matrix.z0 + y1 * (Type)matrix.z1 + z1 * (Type)matrix.z2, x2 * (Type)matrix.x0 + y2 * (Type)matrix.x1 + z2 * (Type)matrix.x2, x2 * (Type)matrix.y0 + y2 * (Type)matrix.y1 + z2 * (Type)matrix.y2, x2 * (Type)matrix.z0 + y2 * (Type)matrix.z1 + z2 * (Type)matrix.z2); return *this; }
public:
	template<typename Type> Matrix3x3<MatrixType> operator/(Type scalar) { static_assert(std::is_arithmetic<Type>::value, "Scalar type must be numeric."); return Matrix3x3<MatrixType>(x0 / scalar, y0 / scalar, z0 / scalar, x1 / scalar, y1 / scalar, z1 / scalar, x2 / scalar, y2 / scalar, z2 / scalar); }
	template<typename Type> Matrix3x3<MatrixType> operator/=(Type scalar) { static_assert(std::is_arithmetic<Type>::value, "Scalar type must be numeric."); *this = Matrix3x3<MatrixType>(x0 / scalar, y0 / scalar, z0 / scalar, x1 / scalar, y1 / scalar, z1 / scalar, x2 / scalar, y2 / scalar, z2 / scalar); return *this; }
public:
	MatrixType x0, y0, z0;
	MatrixType x1, y1, z1;
	MatrixType x2, y2, z2;
public:
	struct GPUMatrix3x3
	{
		float x0, y0, z0, x1;
		float y1, z1, x2, y2;
		float z2;
	};
	GPUMatrix3x3 GetGpuMatrix() { return { (float)x0, (float)y0, (float)z0, (float)x1, (float)y1, (float)z1, (float)x2, (float)y2, (float)z2 }; }
};