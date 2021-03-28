#pragma once

#include <type_traits>
#include <math.h>

template <typename VectorType> class Vector2
{
	static_assert(std::is_arithmetic<VectorType>::value, "Vector type must be numeric");
public:
	Vector2(VectorType x, VectorType y) : x(x), y(y) {};

	template <typename ConvertTo> operator Vector2<ConvertTo>() { static_assert(std::is_arithmetic<ConvertTo>::value, "Cannot convert to a vector of this type"); return Vector2<ConvertTo>((ConvertTo)x, (ConvertTo)y); }
public:
	template <typename ScalarType> Vector2<VectorType> operator+(ScalarType value) { static_assert(std::is_arithmetic<ScalarType>::value, "Cannot add this type to this vector. It must be numeric."); return Vector2<VectorType>(x + (VectorType)value, y + (VectorType)value); }
	template <typename ScalarType> Vector2<VectorType> operator+=(ScalarType value) { static_assert(std::is_arithmetic<ScalarType>::value, "Cannot add this type to this vector. It must be numeric."); *this =  Vector2<VectorType>(x + (VectorType)value, y + (VectorType)value); return *this; }
	template <typename ScalarType> Vector2<VectorType> operator+(Vector2<ScalarType> vector) { static_assert(std::is_arithmetic<ScalarType>::value, "Vector type must be numeric."); return Vector2<VectorType>(x + (VectorType)vector.x, y + (VectorType)vector.y); }
	template <typename ScalarType> Vector2<VectorType> operator+=(Vector2<ScalarType> vector) { static_assert(std::is_arithmetic<ScalarType>::value, "Vector type must be numeric."); *this = Vector2<VectorType>(x + (VectorType)vector.x, y + (VectorType)vector.y); return *this; }
	Vector2<VectorType> operator++() { return Vector2<VectorType>(x++, y++); }
public:
	template <typename ScalarType> Vector2<VectorType> operator-(ScalarType value) { static_assert(std::is_arithmetic<ScalarType>::value, "Cannot subtract this type from this vector. It must be numeric."); return Vector2<VectorType>(x - (VectorType)value, y - (VectorType)value); }
	template <typename ScalarType> Vector2<VectorType> operator-=(ScalarType value) { static_assert(std::is_arithmetic<ScalarType>::value, "Cannot subtract this type from this vector. It must be numeric."); *this = Vector2<VectorType>(x - (VectorType)value, y - (VectorType)value); return *this; }
	template <typename ScalarType> Vector2<VectorType> operator-(Vector2<ScalarType> vector) { static_assert(std::is_arithmetic<ScalarType>::value, "Vector type must be numeric."); return Vector2<VectorType>(x - (VectorType)vector.x, y - (VectorType)vector.y); }
	template <typename ScalarType> Vector2<VectorType> operator-=(Vector2<ScalarType> vector) { static_assert(std::is_arithmetic<ScalarType>::value, "Vector type must be numeric."); *this = Vector2<VectorType>(x - (VectorType)vector.x, y - (VectorType)vector.y); return *this; }
	Vector2<VectorType> operator--() { return Vector2<VectorType>(x--, y--); }
public:
	template <typename ScalarType> Vector2<VectorType> operator*(ScalarType value) { static_assert(std::is_arithmetic<ScalarType>::value, "Cannot multiply this type with this vector. It must be numeric."); return Vector2<VectorType>(x * (VectorType)value, y * (VectorType)value); }
	template <typename ScalarType> Vector2<VectorType> operator*=(ScalarType value) { static_assert(std::is_arithmetic<ScalarType>::value, "Cannot multiply this type with this vector. It must be numeric."); *this = Vector2<VectorType>(x * (VectorType)value, y * (VectorType)value); return *this; }
	template <typename ScalarType> Vector2<VectorType> operator*(Vector2<ScalarType> vector) { static_assert(std::is_arithmetic<ScalarType>::value, "Vector type must be numeric."); return Vector2<VectorType>(x * (VectorType)vector.x, y * (VectorType)vector.y); }
	template <typename ScalarType> Vector2<VectorType> operator*=(Vector2<ScalarType> vector) { static_assert(std::is_arithmetic<ScalarType>::value, "Vector type must be numeric."); *this = Vector2<VectorType>(x * (VectorType)vector.x, y * (VectorType)vector.y); return *this; }
public:
	template <typename ScalarType> Vector2<VectorType> operator/(ScalarType value) { static_assert(std::is_arithmetic<ScalarType>::value, "Cannot divide this vector by this type. It must be numeric."); return Vector2<VectorType>(x / (VectorType)value, y / (VectorType)value); }
	template <typename ScalarType> Vector2<VectorType> operator/=(ScalarType value) { static_assert(std::is_arithmetic<ScalarType>::value, "Cannot divide this vector by this type. It must be numeric."); *this = Vector2<VectorType>(x / (VectorType)value, y / (VectorType)value); return *this; }
	template <typename ScalarType> Vector2<VectorType> operator/(Vector2<ScalarType> vector) { static_assert(std::is_arithmetic<ScalarType>::value, "Vector type must be numeric."); return Vector2<VectorType>(x / (VectorType)vector.x, y / (VectorType)vector.y); }
	template <typename ScalarType> Vector2<VectorType> operator/=(Vector2<ScalarType> vector) { static_assert(std::is_arithmetic<ScalarType>::value, "Vector type must be numeric."); *this = Vector2<VectorType>(x / (VectorType)vector.x, y / (VectorType)vector.y); return *this; }
public:
	Vector2<VectorType> Normalized() { return *this / sqrt((double)(x * x + y * y)); }
	void Normalize() { *this /= sqrt((double)(x * x + y * y)); }
	template <typename T> static double Dot(Vector2<T> vector1, Vector2<T> vector2) { return (double)vector1.x * (double)vector2.x + (double)vector1.y * (double)vector2.y; }
public:
	VectorType x, y;
};

template <typename VectorType> class Vector3
{
	static_assert(std::is_arithmetic<VectorType>::value, "Vector type must be numeric");
public:
	Vector3(VectorType x, VectorType y, VectorType z) : x(x), y(y), z(z) {};

	template <typename ConvertTo>
	operator Vector3<ConvertTo>()
	{
		static_assert(std::is_arithmetic<ConvertTo>::value, "Cannot convert to a vector of this type");
		return Vector3<ConvertTo>((ConvertTo)x, (ConvertTo)y, (ConvertTo)z);
	}
public:
	template <typename ScalarType> Vector3<VectorType> operator+(ScalarType value) { static_assert(std::is_arithmetic<ScalarType>::value, "Cannot add this type to this vector. It must be numeric."); return Vector3<VectorType>(x + (VectorType)value, y + (VectorType)value, z + (VectorType)value); }
	template <typename ScalarType> Vector3<VectorType> operator+(Vector3<ScalarType> vector) { static_assert(std::is_arithmetic<ScalarType>::value, "Vector type must be numeric."); return Vector3<VectorType>(x + (VectorType)vector.x, y + (VectorType)vector.y, z + (VectorType)vector.z); }
	template <typename ScalarType> Vector3<VectorType> operator+=(ScalarType value) { static_assert(std::is_arithmetic<ScalarType>::value, "Cannot add this type to this vector. It must be numeric."); *this = Vector3<VectorType>(x + (VectorType)value, y + (VectorType)value, z + (VectorType)value); return *this; }
	template <typename ScalarType> Vector3<VectorType> operator+=(Vector3<ScalarType> vector) { static_assert(std::is_arithmetic<ScalarType>::value, "Vector type must be numeric."); *this = Vector3<VectorType>(x + (VectorType)vector.x, y + (VectorType)vector.y, z + (VectorType)vector.z); return *this; }
	Vector3<VectorType> operator++() { return Vector3<VectorType>(x++, y++, z++); }
public:
	Vector3<VectorType> operator-() { return Vector3<VectorType>(-x, -y, -z); }
	template <typename ScalarType> Vector3<VectorType> operator-(ScalarType value) { static_assert(std::is_arithmetic<ScalarType>::value, "Cannot subtract this type from this vector. It must be numeric."); return Vector3<VectorType>(x - (VectorType)value, y - (VectorType)value, z - (VectorType)value); }
	template <typename ScalarType> Vector3<VectorType> operator-(Vector3<ScalarType> vector) { static_assert(std::is_arithmetic<ScalarType>::value, "Vector type must be numeric."); return Vector3<VectorType>(x - (VectorType)vector.x, y - (VectorType)vector.y, z - (VectorType)vector.z); }
	template <typename ScalarType> Vector3<VectorType> operator-=(ScalarType value) { static_assert(std::is_arithmetic<ScalarType>::value, "Cannot subtract this type from this vector. It must be numeric."); *this = Vector3<VectorType>(x - (VectorType)value, y - (VectorType)value, z - (VectorType)value); return *this; }
	template <typename ScalarType> Vector3<VectorType> operator-=(Vector3<ScalarType> vector) { static_assert(std::is_arithmetic<ScalarType>::value, "Vector type must be numeric."); *this = Vector3<VectorType>(x - (VectorType)vector.x, y - (VectorType)vector.y, z - (VectorType)vector.z); return *this; }
	Vector3<VectorType> operator--() { return Vector3(x--, y--, z--); }
public:
	template <typename ScalarType> Vector3<VectorType> operator*(ScalarType value) { static_assert(std::is_arithmetic<ScalarType>::value, "Cannot multiply this type with this vector. It must be numeric."); return Vector3<VectorType>(x * (VectorType)value, y * (VectorType)value, z * (VectorType)value); }
	template <typename ScalarType> Vector3<VectorType> operator*(Vector3<ScalarType> vector) { static_assert(std::is_arithmetic<ScalarType>::value, "Vector type must be numeric."); return Vector3<VectorType>(x * (VectorType)vector.x, y * (VectorType)vector.y, z * (VectorType)vector.z); }
	template <typename ScalarType> Vector3<VectorType> operator*=(ScalarType value) { static_assert(std::is_arithmetic<ScalarType>::value, "Cannot multiply this type with this vector. It must be numeric."); *this = Vector3<VectorType>(x * (VectorType)value, y * (VectorType)value, z * (VectorType)value); return *this; }
	template <typename ScalarType> Vector3<VectorType> operator*=(Vector3<ScalarType> vector) { static_assert(std::is_arithmetic<ScalarType>::value, "Vector type must be numeric."); *this = Vector3<VectorType>(x * (VectorType)vector.x, y * (VectorType)vector.y, z * (VectorType)vector.z); return *this; }
public:
	template <typename ScalarType> Vector3<VectorType> operator/(ScalarType value) { static_assert(std::is_arithmetic<ScalarType>::value, "Cannot divide this vector by this type. It must be numeric."); return Vector3<VectorType>(x / (VectorType)value, y / (VectorType)value, z / (VectorType)value); }
	template <typename ScalarType> Vector3<VectorType> operator/(Vector3<ScalarType> vector) { static_assert(std::is_arithmetic<ScalarType>::value, "Vector type must be numeric."); return Vector3<VectorType>(x / (VectorType)vector.x, y / (VectorType)vector.y, z / (VectorType)vector.z); }
	template <typename ScalarType> Vector3<VectorType> operator/=(ScalarType value) { static_assert(std::is_arithmetic<ScalarType>::value, "Cannot divide this vector by this type. It must be numeric."); *this = Vector3<VectorType>(x / (VectorType)value, y / (VectorType)value, z / (VectorType)value); return *this; }
	template <typename ScalarType> Vector3<VectorType> operator/=(Vector3<ScalarType> vector) { static_assert(std::is_arithmetic<ScalarType>::value, "Vector type must be numeric."); *this = Vector3<VectorType>(x / (VectorType)vector.x, y / (VectorType)vector.y, z / (VectorType)vector.z); return *this; }
public:
	Vector3 Normalized() { return *this / sqrt((double)(x * x + y * y + z * z)); }
	void Normalize() { *this /= sqrt((double)(x * x + y * y + z * z)); }
	template <typename T>static double Dot(Vector3<T> vector1, Vector3<T> vector2) { return (double)vector1.x * (double)vector2.x + (double)vector1.y * (double)vector2.y + (double)vector1.z * (double)vector2.z; }
	template <typename T> static Vector3<T> Cross(Vector3<T> u, Vector3<T> v)
	{
		return Vector3<T>(u.y * v.z - u.z * v.y, u.z * v.x - u.x * v.z, u.x * v.y - u.y * v.x);
	}
public:
	VectorType x, y, z;
};

template <typename VectorType> class Vector4
{
	static_assert(std::is_arithmetic<VectorType>::value, "Vector type must be numeric");
public:
	Vector4(VectorType x, VectorType y, VectorType z, VectorType w) : x(x), y(y), z(z), w(w) {};
	template <typename ConvertTo> operator Vector4<ConvertTo>() { static_assert(std::is_arithmetic<ConvertTo>::value, "Cannot convert to a vector of this type"); return Vector4<ConvertTo>((ConvertTo)x, (ConvertTo)y, (ConvertTo)z, (ConvertTo)w); }
public:
	template <typename ScalarType> Vector4<VectorType> operator+(ScalarType value) { static_assert(std::is_arithmetic<ScalarType>::value, "Scalar type must be numeric."); return Vector4<VectorType>(x + (VectorType)value, y + (VectorType)value, z + (VectorType)value, w + (VectorType)value); }
	template <typename ScalarType> Vector4<VectorType> operator+(Vector4<ScalarType> vector) { static_assert(std::is_arithmetic<ScalarType>::value, "Vector type must be numeric."); return Vector4<VectorType>(x + (VectorType)vector.x, y + (VectorType)vector.y, z + (VectorType)vector.z, w + (VectorType)vector.w); }
	template <typename ScalarType> Vector4<VectorType> operator+=(ScalarType value) { static_assert(std::is_arithmetic<ScalarType>::value, "Scalar type must be numeric."); *this = Vector4<VectorType>(x + (VectorType)value, y + (VectorType)value, z + (VectorType)value, w + (VectorType)value); return *this; }
	template <typename ScalarType> Vector4<VectorType> operator+=(Vector4<ScalarType> vector) { static_assert(std::is_arithmetic<ScalarType>::value, "Vector type must be numeric."); *this = Vector4<VectorType>(x + (VectorType)vector.x, y + (VectorType)vector.y, z + (VectorType)vector.z, w + (VectorType)vector.w); return *this; }
	Vector4<VectorType> operator++() { return Vector4(x++, y++, z++, w++); }
public:
	template <typename ScalarType> Vector4<VectorType> operator-(ScalarType value) { static_assert(std::is_arithmetic<ScalarType>::value, "Scalar type must be numeric."); return Vector4<VectorType>(x - (VectorType)value, y - (VectorType)value, z - (VectorType)value, w - (VectorType)value); }
	template <typename ScalarType> Vector4<VectorType> operator-(Vector4<ScalarType> vector) { static_assert(std::is_arithmetic<ScalarType>::value, "Vector type must be numeric."); return Vector4<VectorType>(x - (VectorType)vector.x, y - (VectorType)vector.y, z - (VectorType)vector.z, w - (VectorType)vector.w); }
	template <typename ScalarType> Vector4<VectorType> operator-=(ScalarType value) { static_assert(std::is_arithmetic<ScalarType>::value, "Scalar type must be numeric."); *this = Vector4<VectorType>(x - (VectorType)value, y - (VectorType)value, z - (VectorType)value, w - (VectorType)value); return *this; }
	template <typename ScalarType> Vector4<VectorType> operator-=(Vector4<ScalarType> vector) { static_assert(std::is_arithmetic<ScalarType>::value, "Vector type must be numeric."); *this = Vector4<VectorType>(x - (VectorType)vector.x, y - (VectorType)vector.y, z - (VectorType)vector.z, w - (VectorType)vector.w); return *this; }
	Vector4<VectorType> operator--() { return Vector4(x--, y--, z--, w--); }
public:
	template <typename ScalarType> Vector4<VectorType> operator*(ScalarType value) { static_assert(std::is_arithmetic<ScalarType>::value, "Scalar type must be numeric."); return Vector4<VectorType>(x * (VectorType)value, y * (VectorType)value, z * (VectorType)value, w * (VectorType)value); }
	template <typename ScalarType> Vector4<VectorType> operator*(Vector4<ScalarType> vector) { static_assert(std::is_arithmetic<ScalarType>::value, "Vector type must be numeric."); return Vector4<VectorType>(x * (VectorType)vector.x, y * (VectorType)vector.y, z * (VectorType)vector.z, w * (VectorType)vector.w); }
	template <typename ScalarType> Vector4<VectorType> operator*=(ScalarType value) { static_assert(std::is_arithmetic<ScalarType>::value, "Scalar type must be numeric."); *this = Vector4<VectorType>(x * (VectorType)value, y * (VectorType)value, z * (VectorType)value, w * (VectorType)value); return *this; }
	template <typename ScalarType> Vector4<VectorType> operator*=(Vector4<ScalarType> vector) { static_assert(std::is_arithmetic<ScalarType>::value, "Vector type must be numeric."); *this = Vector4<VectorType>(x * (VectorType)vector.x, y * (VectorType)vector.y, z * (VectorType)vector.z, w * (VectorType)vector.w); return *this; }
public:
	template <typename ScalarType> Vector4<VectorType> operator/(ScalarType value) { static_assert(std::is_arithmetic<ScalarType>::value, "Scalar type must be numeric."); return Vector4<VectorType>(x / (VectorType)value, y / (VectorType)value, z / (VectorType)value, w / (VectorType)value); }
	template <typename ScalarType> Vector4<VectorType> operator/(Vector4<ScalarType> vector) { static_assert(std::is_arithmetic<ScalarType>::value, "Vector type must be numeric."); return Vector4<VectorType>(x / (VectorType)vector.x, y / (VectorType)vector.y, z / (VectorType)vector.z, w / (VectorType)vector.w); }
	template <typename ScalarType> Vector4<VectorType> operator/=(ScalarType value) { static_assert(std::is_arithmetic<ScalarType>::value, "Scalar type must be numeric."); *this = Vector4<VectorType>(x / (VectorType)value, y / (VectorType)value, z / (VectorType)value, w / (VectorType)value); return *this; }
	template <typename ScalarType> Vector4<VectorType> operator/=(Vector4<ScalarType> vector) { static_assert(std::is_arithmetic<ScalarType>::value, "Vector type must be numeric."); *this = Vector4<VectorType>(x / (VectorType)vector.x, y / (VectorType)vector.y, z / (VectorType)vector.z, w / (VectorType)vector.w); return *this; }
public:
	Vector4<VectorType> Normalized() { return *this / sqrt((double)(x * x + y * y + z * z + w * w)); }
	void Normalize() { *this /= sqrt((double)(x * x + y * y + z * z + w * w)); }
	template <typename T> static double Dot(Vector4<T> vector1, Vector4<T> vector2) { return (double)vector1.x * (double)vector2.x + (double)vector1.y * (double)vector2.y + (double)vector1.z * (double)vector2.z + (double)vector1.w * (double)vector2.w; }
public:
	VectorType x, y, z, w;
};