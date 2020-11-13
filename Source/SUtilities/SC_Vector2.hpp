#pragma once

#ifndef VECTOR_H
#error Vector.h should be included instead of Vector2.hpp
#endif

namespace Shift
{
	template<class T>
	class SC_Vector2
	{
	public:
		SC_Vector2();
		SC_Vector2(const T& aX, const T& aY);
		SC_Vector2(const T& aValue);
		~SC_Vector2();

		template<class T2>
		inline SC_Vector2::SC_Vector2(const T2& aX, const T2& aY)
			: x(static_cast<T>(aX))
			, y(static_cast<T>(aY))
		{
		}
		template<class T2>
		inline SC_Vector2::SC_Vector2(const SC_Vector2<T2>& aOther)
			: x(static_cast<T>(aOther.x))
			, y(static_cast<T>(aOther.y))
		{
		}

		// MODIFYING OPERATIONS
		inline SC_Vector2<T>& operator+=(const SC_Vector2<T>& aVector) { x += aVector.x; y += aVector.y; return *this; }
		inline SC_Vector2<T>& operator-=(const SC_Vector2<T>& aVector) { x -= aVector.x; y -= aVector.y; return *this; }
		inline SC_Vector2<T>& operator*=(const SC_Vector2<T>& aVector) { x *= aVector.x; y *= aVector.y; return *this; }
		inline SC_Vector2<T>& operator/=(const SC_Vector2<T>& aVector) { x /= aVector.x; y /= aVector.y; return *this; }

		inline SC_Vector2<T>& operator*=(T aValue) { x *= aValue; y *= aValue; return *this; }
		inline SC_Vector2<T>& operator/=(T aValue) { x /= aValue; y /= aValue; return *this; }

		// CREATIVE OPERATORS WITH VECTORS
		inline SC_Vector2<T> operator+(const SC_Vector2<T>& aVector) const { return SC_Vector2<T>(x + aVector.x, y + aVector.y); }
		inline SC_Vector2<T> operator-(const SC_Vector2<T>& aVector) const { return SC_Vector2<T>(x - aVector.x, y - aVector.y); }
		inline SC_Vector2<T> operator*(const SC_Vector2<T>& aVector) const { return SC_Vector2<T>(x * aVector.x, y * aVector.y); }
		inline SC_Vector2<T> operator/(const SC_Vector2<T>& aVector) const { return SC_Vector2<T>(x / aVector.x, y / aVector.y); }

		// CREATIVE OPERATORS WITH SCALARS
		inline SC_Vector2<T> operator*(T aValue) const { return SC_Vector2<T>(x * aValue, y * aValue); }
		inline SC_Vector2<T> operator/(T aValue) const { return SC_Vector2<T>(x / aValue, y / aValue); }
		inline SC_Vector2<T> operator>>(uint aValue) const { return SC_Vector2<T>(x >> aValue, y >> aValue); }
		inline SC_Vector2<T> operator<<(uint aValue) const { return SC_Vector2<T>(x << aValue, y << aValue); }



		inline T Length() const;
		inline T Length2() const { return (x * x + y * y); }

		inline SC_Vector2<T>& Normalize();
		//float Length();
		//float Length2();

	public:
		union { T x, r; };
		union { T y, z, g; };

		struct SC_AllowRawHashing_INTERNAL;
	};

	template<class T>
	inline SC_Vector2<T>::SC_Vector2()
		: x(static_cast<T>(0))
		, y(static_cast<T>(0))
	{
	}

	template<class T>
	inline SC_Vector2<T>::SC_Vector2(const T& aX, const T& aY)
		: x(static_cast<T>(aX))
		, y(static_cast<T>(aY))
	{
	}

	template<class T>
	inline SC_Vector2<T>::SC_Vector2(const T& aValue)
		: x(static_cast<T>(aValue))
		, y(static_cast<T>(aValue))
	{
	}

	template<class T>
	inline SC_Vector2<T>::~SC_Vector2()
	{
	}

	template<class T>
	inline T SC_Vector2<T>::Length() const
	{
		return (T)sqrt(x * x + y * y);
	}

	template<class T>
	inline SC_Vector2<T>& SC_Vector2<T>::Normalize()
	{
		const T length2 = x * x + y * y;

		if (length2 > 0)
		{
			const T inversedLength = T(1) / (T)sqrt(length2);
			x *= inversedLength;
			y *= inversedLength;
		}

		return *this;
	}

	template<class T> using SC_Vec2 = SC_Vector2<T>;
	using SC_Vector2f = SC_Vector2<float>;
	using SC_Vector2i = SC_Vector2<int>;
	using SC_Vector2ui = SC_Vector2<unsigned int>;
	using SC_Float2 = SC_Vector2<float>;
	using SC_Int2 = SC_Vector2<int>;
	using SC_Uint2 = SC_Vector2<unsigned int>;
}
