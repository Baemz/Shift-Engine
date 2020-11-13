#pragma once

#ifndef VECTOR_H
#error Vector.h should be included instead of Vector3.hpp
#endif

namespace Shift
{
	template<class T>
	class SC_Vector3
	{
	public:
		SC_Vector3(); 
		SC_Vector3(const T& aX, const T& aY, const T& aZ);
		SC_Vector3(const T& aValue);
		~SC_Vector3();

		template <class OtherType>
		inline SC_Vector3(const SC_Vector2<OtherType>& aVector) { x = (T)aVector.x; y = 0; z = (T)aVector.y; }

		template <class OtherType>
		inline SC_Vector3(const SC_Vector2<OtherType>& aVector, const T& aZ) { x = (T)aVector.x; y = (T)aVector.y; z = aZ; }

		template <class OtherType>
		inline SC_Vector3(const SC_Vector3<OtherType>& aVector) { x = (T)aVector.x; y = (T)aVector.y; z = (T)aVector.z; }

		template <class OtherType>
		inline SC_Vector3(const SC_Vector4<OtherType>& aVector) { x = (T)aVector.x; y = (T)aVector.y; z = (T)aVector.z; }

		//Negate operator
		SC_Vector3<T> operator-() const;

		// Subtraction operators
		SC_Vector3<T>& operator-=(const SC_Vector3<T>& aVector3);
		SC_Vector3<T>& operator-=(const T& aValue);
		SC_Vector3<T> operator-(const SC_Vector3<T>& aVector3) const;
		SC_Vector3<T> operator-(const T& aValue) const;

		//Multiplication operators
		SC_Vector3<T>& operator*=(const SC_Vector3<T>& aVector3);
		SC_Vector3<T>& operator*=(const T& aValue);
		SC_Vector3<T> operator*(const SC_Vector3<T>& aVector3) const;
		SC_Vector3<T> operator*(const T& aValue) const;

		//Addition operators
		SC_Vector3<T>& operator+=(const SC_Vector3<T>& aVector3);
		SC_Vector3<T>& operator+=(const T& aValue);
		SC_Vector3<T> operator+(const SC_Vector3<T>& aVector3) const;
		SC_Vector3<T> operator+(const T& aValue) const;

		//Division operators
		SC_Vector3<T>& operator/=(const SC_Vector3<T>& aVector3);
		SC_Vector3<T>& operator/=(const T& aValue);
		SC_Vector3<T> operator/(const SC_Vector3<T>& aVector3) const;
		SC_Vector3<T> operator/(const T& aValue) const;

		inline uint operator<(const SC_Vector3<T>& aVector) const
		{
			return (z < aVector.z ? 0x4 : 0) | (y < aVector.y ? 0x2 : 0) | (x < aVector.x ? 0x1 : 0);
		}
		inline uint operator<=(const SC_Vector3<T>& aVector) const
		{
			return (z <= aVector.z ? 0x4 : 0) | (y <= aVector.y ? 0x2 : 0) | (x <= aVector.x ? 0x1 : 0);
		}
		inline uint operator>(const SC_Vector3<T>& aVector) const
		{
			return (z > aVector.z ? 0x4 : 0) | (y > aVector.y ? 0x2 : 0) | (x > aVector.x ? 0x1 : 0);
		}
		inline uint operator>=(const SC_Vector3<T>& aVector) const
		{
			return (z >= aVector.z ? 0x4 : 0) | (y >= aVector.y ? 0x2 : 0) | (x >= aVector.x ? 0x1 : 0);
		}

		void Normalize();
		SC_Vector3<T> GetNormalized() const;
		SC_Vector3<T>& Scale(T aValue);
		T Length() const;
		T Length2() const;

		SC_Vector3<T> Cross(const SC_Vector3<T>& aVector) const;
		T Dot(const SC_Vector3<T>& aVector) const;

		inline T& X() { return x; }
		inline T& Y() { return y; }
		inline T& Z() { return z; }
		inline T X() const { return x; }
		inline T Y() const { return y; }
		inline T Z() const { return z; }

		inline SC_Vector2<T> XZ() const { return SC_Vector2<T>(x, z); }
		inline SC_Vector2<T> XY() const { return SC_Vector2<T>(x, y); }

	public:
		union { T x, r; };
		union { T y, g; };
		union { T z, b; };

		const static SC_Vector3<T> Zero;
		const static SC_Vector3<T> One;
		const static SC_Vector3<T> Forward;
		const static SC_Vector3<T> Right;
		const static SC_Vector3<T> Up;

		struct SC_AllowRawHashing_INTERNAL;
	};

	template <class T> const SC_Vector3<T> SC_Vector3<T>::Zero = SC_Vector3<T>{ 0 };
	template <class T> const SC_Vector3<T> SC_Vector3<T>::One = SC_Vector3<T>{ 1 };
	template <class T> const SC_Vector3<T> SC_Vector3<T>::Forward = SC_Vector3<T>{ 0, 0, 1 };
	template <class T> const SC_Vector3<T> SC_Vector3<T>::Right = SC_Vector3<T>{ 1, 0, 0 };
	template <class T> const SC_Vector3<T> SC_Vector3<T>::Up = SC_Vector3<T>{ 0, 1, 0 };

	template<class T>
	SC_Vector3<T>::SC_Vector3()
		: x(static_cast<T>(0))
		, y(static_cast<T>(0))
		, z(static_cast<T>(0))
	{
	}

	template<class T>
	SC_Vector3<T>::SC_Vector3(const T& aX, const T& aY, const T& aZ)
		: x(aX)
		, y(aY)
		, z(aZ)
	{
	}
	template<class T>
	SC_Vector3<T>::SC_Vector3(const T& aValue)
		: x(aValue)
		, y(aValue)
		, z(aValue)
	{
	}

	template<class T>
	SC_Vector3<T>::~SC_Vector3()
	{
	}

	template<class T> 
	inline T SC_Vector3<T>::Length2() const
	{
		return static_cast<T>((x * x) + (y * y) + (z * z));
	}

	template<class T>
	inline SC_Vector3<T>& SC_Vector3<T>::Scale(T aValue)
	{ 
		Normalize(); 
		operator *= (aValue); 
		return *this;
	}

	template<class T> 
	inline T SC_Vector3<T>::Length() const
	{
		return static_cast<T>(sqrt((x * x) + (y * y) + (z * z)));
	}

	template<class T>
	inline void SC_Vector3<T>::Normalize()
	{
		const T length2 = x * x + y * y + z * z;

		if (length2 > 0)
		{
			const T inversedLength = T(1) / (T)sqrt(length2);
			x *= inversedLength;
			y *= inversedLength;
			z *= inversedLength;
		}
	}

	template<class T>
	inline SC_Vector3<T> SC_Vector3<T>::GetNormalized() const
	{
		SC_Vector3<T> tmp(*this);
		tmp.Normalize();
		return tmp;
	}
	template<class T> 
	inline T SC_Vector3<T>::Dot(const SC_Vector3<T>& aVector) const
	{
		return ((x * aVector.x) + (y * aVector.y) + (z * aVector.z));
	}
	template<class T>
	inline SC_Vector3<T> SC_Vector3<T>::Cross(const SC_Vector3<T>& aVector) const
	{
		return SC_Vector3<T>(
			((y * aVector.z) - (z * aVector.y)),
			((z * aVector.x) - (x * aVector.z)),
			((x * aVector.y) - (y * aVector.x)));
	}

	template<class T>
	inline SC_Vector3<T> SC_Vector3<T>::operator-() const
	{
		return SC_Vector3<T>(-x, -y, -z);
	}

	template<class T>
	inline SC_Vector3<T>& SC_Vector3<T>::operator-=(const SC_Vector3<T>& aVector3)
	{
		x -= aVector3.x;
		y -= aVector3.y;
		z -= aVector3.z;
		return *this;
	}

	template<class T>
	inline SC_Vector3<T>& SC_Vector3<T>::operator-=(const T& aValue)
	{
		x -= aValue;
		y -= aValue;
		z -= aValue;
		return *this;
	}

	template<class T>
	inline SC_Vector3<T> SC_Vector3<T>::operator-(const SC_Vector3<T>& aVector3) const
	{
		return SC_Vector3<T>(x - aVector3.x, y - aVector3.y, z - aVector3.z);
	}

	template<class T>
	inline SC_Vector3<T> SC_Vector3<T>::operator-(const T& aValue) const
	{
		return SC_Vector3<T>(x - aValue, y - aValue, z - aValue);
	}

	template<class T>
	inline SC_Vector3<T>& SC_Vector3<T>::operator*=(const SC_Vector3<T>& aVector3)
	{
		x *= aVector3.x;
		y *= aVector3.y;
		z *= aVector3.z;
		return *this;
	}

	template<class T>
	inline SC_Vector3<T>& SC_Vector3<T>::operator*=(const T& aValue)
	{
		x *= aValue;
		y *= aValue;
		z *= aValue;
		return *this;
	}

	template<class T>
	inline SC_Vector3<T> SC_Vector3<T>::operator*(const SC_Vector3<T> & aVector3) const
	{
		return SC_Vector3<T>(x * aVector3.x, y * aVector3.y, z * aVector3.z);
	}

	template<class T>
	inline SC_Vector3<T> SC_Vector3<T>::operator*(const T& aValue) const
	{
		return SC_Vector3<T>(x * aValue, y * aValue, z * aValue);
	}

	template<class T>
	inline SC_Vector3<T>& SC_Vector3<T>::operator+=(const SC_Vector3<T>& aVector3)
	{
		x += aVector3.x;
		y += aVector3.y;
		z += aVector3.z;
		return *this;
	}

	template<class T>
	inline SC_Vector3<T>& SC_Vector3<T>::operator+=(const T& aValue)
	{
		x += aValue;
		y += aValue;
		z += aValue;
		return *this;
	}

	template<class T>
	inline SC_Vector3<T> SC_Vector3<T>::operator+(const SC_Vector3<T> & aVector3) const
	{
		return SC_Vector3<T>(x + aVector3.x, y + aVector3.y, z + aVector3.z);
	}

	template<class T>
	inline SC_Vector3<T> SC_Vector3<T>::operator+(const T& aValue) const
	{
		return SC_Vector3<T>(x + aValue, y + aValue, z + aValue);
	}

	template<class T>
	inline SC_Vector3<T>& SC_Vector3<T>::operator/=(const SC_Vector3<T>& aVector3)
	{
		x /= aVector3.x; 
		y /= aVector3.y; 
		z /= aVector3.z; 
		return *this;
	}

	template<class T>
	inline SC_Vector3<T>& SC_Vector3<T>::operator/=(const T& aValue)
	{
		x /= aValue;
		y /= aValue;
		z /= aValue;
		return *this;
	}

	template<class T>
	inline SC_Vector3<T> SC_Vector3<T>::operator/(const SC_Vector3<T>& aVector3) const
	{
		return SC_Vector3<T>(x / aVector3.x, y / aVector3.y, z / aVector3.z);
	}

	template<class T>
	inline SC_Vector3<T> SC_Vector3<T>::operator/(const T& aValue) const
	{
		return SC_Vector3<T>(x / aValue, y / aValue, z / aValue);
	}

	using SC_Vector3f = SC_Vector3<float>;
	using SC_Vector3i = SC_Vector3<int>;
	using SC_Vector3ui = SC_Vector3<unsigned int>;
}
