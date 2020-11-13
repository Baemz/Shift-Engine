#pragma once

#ifndef VECTOR_H
#error Vector.h should be included instead of Vector4.hpp
#endif

namespace Shift
{
	template<class T>
	class SC_Vector4
	{
	public:
		SC_Vector4();
		SC_Vector4(const T& aX, const T& aY, const T& aZ, const T& aW);
		~SC_Vector4();

		bool operator==(const SC_Vector4<T>& aOther)
		{
			if (x == aOther.x &&
				y == aOther.y &&
				z == aOther.z &&
				w == aOther.w)
			{
				return true;
			}
			return false;
		}

		//void Normalize();
		//float Length();
		//float Length2();

	public:
		union { T x, r; };
		union { T y, g; };
		union { T z, b; };
		union { T w, a; };

		struct SC_AllowRawHashing_INTERNAL;
	};

	template<class T>
	inline SC_Vector4<T>::SC_Vector4()
		: x(static_cast<T>(0))
		, y(static_cast<T>(0))
		, z(static_cast<T>(0))
		, w(static_cast<T>(0))
	{
	}
	template<class T>
		inline SC_Vector4<T>::SC_Vector4(const T& aX, const T& aY, const T& aZ, const T& aW)
		: x(aX)
		, y(aY)
		, z(aZ)
		, w(aW)
	{
	}

	template<class T>
	inline SC_Vector4<T>::~SC_Vector4()
	{
	}


	template<class T> using SC_Vec4 = SC_Vector4<T>;
	using SC_Vector4f = SC_Vector4<float>;
	using SC_Vector4i = SC_Vector4<int>;
	using SC_Vector4ui = SC_Vector4<unsigned int>;
	using SC_Color = SC_Vector4<float>;
}
#include "SC_Vector4f.hpp"