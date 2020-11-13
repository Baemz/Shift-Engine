#pragma once
#include "SC_Vector.h"
#include <float.h>

namespace Shift
{
	template<class T>
	class SC_AxisAlignedBoundingBox
	{
		using OurType = SC_AxisAlignedBoundingBox<T>;
	public:
		SC_Vector3<T> myMin;
		SC_Vector3<T> myMax;

		SC_AxisAlignedBoundingBox()
			: myMin(SC_FLT_LOWEST)
			, myMax(SC_FLT_MAX) 
		{}

		template <typename U>
		SC_AxisAlignedBoundingBox(const SC_AxisAlignedBoundingBox<U>& aOther)
			: myMin(static_cast<T>(aOther.myMin))
			, myMax(static_cast<T>(aOther.myMax))
		{}

		bool operator!() const
		{
			return (myMax < myMin) != 0;
		}

		OurType& operator=(const OurType& aOther)
		{
			myMin = aOther.myMin;
			myMax = aOther.myMax;

			return *this;
		}

		bool operator==(const OurType& aOther) const
		{
			return (myMin == aOther.myMin && myMax == aOther.myMax);
		}

		bool operator!=(const OurType& aOther) const
		{
			return (myMin != aOther.myMin || myMax != aOther.myMax);
		}

		template <typename U>
		bool IsInside(const SC_Vector3<U> & aPoint, const T aEpsilon = T(0)) const
		{
			return (aPoint < (myMax + SC_Vector3<T>(aEpsilon)) && aPoint > (myMin - SC_Vector3<T>(aEpsilon)));
		}

		void Merge(const OurType& aOther)
		{
			myMin.x = SC_Min(myMin.x, aOther.myMin.x);
			myMin.y = SC_Min(myMin.y, aOther.myMin.y);
			myMin.z = SC_Min(myMin.z, aOther.myMin.z);

			myMax.x = SC_Max(myMax.x, aOther.myMax.x);
			myMax.y = SC_Max(myMax.y, aOther.myMax.y);
			myMax.z = SC_Max(myMax.z, aOther.myMax.z);
		}

		template <typename U>
		void FromPoints(const SC_Vector3<U>* somePoints, uint aNumPoints)
		{
			assert(aNumPoints > 0);
			myMin = myMax = static_cast<SC_Vector3<U>>(somePoints[0]);
			for (uint i = 1; i < aNumPoints; ++i)
			{
				myMin = SC_Min(myMin, static_cast<SC_Vector3<U>>(somePoints[i]));
				myMax = SC_Max(myMax, static_cast<SC_Vector3<U>>(somePoints[i]));
			}
		}

	};

	using SC_AABBf = SC_AxisAlignedBoundingBox<float>;
	using SC_AABBd = SC_AxisAlignedBoundingBox<double>;
	using SC_AABB = SC_AABBf;
}