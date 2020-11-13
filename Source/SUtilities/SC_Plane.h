#pragma once

#include "SC_Vector.h"

namespace Shift
{
	template <class T>
	class SC_Plane
	{
	public:
		SC_Vector3<T> myNormal;
		T myDistance;

		SC_Plane();
		SC_Plane(const SC_Plane& aPlane); 
		SC_Plane(const T& aFirst, const T& aSecond, const T& aThird, const T& aFourth);
		SC_Plane(const SC_Vector3<T>& aNormal, const SC_Vector3<T>& aPoint);

		T DistanceToPlane(const SC_Vector3<T>& aPoint) const;

		bool SphereIntersects(const SC_Vector3<T>& aCenter, const T& aRadius) const;

		void Normalize();

	private:
	};

	template<class T>
	inline SC_Plane<T>::SC_Plane()
		: myDistance((T)0.0)
	{
	}

	template<class T>
	inline SC_Plane<T>::SC_Plane(const SC_Plane& aPlane)
		: myNormal(aPlane.myNormal)
		, myDistance(aPlane.myDistance)
	{
	}

	template<class T>
	inline SC_Plane<T>::SC_Plane(const T& aFirst, const T& aSecond, const T& aThird, const T& aFourth)
		: myNormal(aFirst, aSecond, aThird)
		, myDistance(aFourth)
	{
	}

	template<class T>
	inline SC_Plane<T>::SC_Plane(const SC_Vector3<T>& aNormal, const SC_Vector3<T>& aPoint)
	{
		myNormal = aNormal;
		myDistance = -(myNormal.x * aPoint.x + myNormal.y * aPoint.y + myNormal.z * aPoint.z);
	}

	template<class T>
	inline T SC_Plane<T>::DistanceToPlane(const SC_Vector3<T>& aPoint) const
	{
		return myNormal.Dot(aPoint) + myDistance;
	}

	template<class T>
	inline bool SC_Plane<T>::SphereIntersects(const SC_Vector3<T>& aCenter, const T& aRadius) const
	{
		T absDistToPlane = abs(DistanceToPlane(aCenter));
		return (absDistToPlane < aRadius);
	}

	template<class T>
	inline void SC_Plane<T>::Normalize()
	{
		float l2 = myNormal.Length2();
		SC_ASSERT(l2 >= 0);
		float normalLength = sqrt(l2);
		float t = normalLength > 0.f ? (1.0f / normalLength) : 0.f;
		myNormal.x *= t;
		myNormal.y *= t;
		myNormal.z *= t;
		myDistance *= t;
	}

	using SC_Planef = SC_Plane<float>;
}