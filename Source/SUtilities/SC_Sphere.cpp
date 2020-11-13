#include "SC_Sphere.h"
#include "SC_CommonFuncs.h"

namespace Shift
{
	SC_Sphere::SC_Sphere()
		: myRadius(0.f)
	{
	}

	SC_Sphere::SC_Sphere(const SC_Vector3f& aPoint0, const SC_Vector3f& aPoint1)
		: myCenter((aPoint0 + aPoint1) * 0.5f)
		, myRadius((aPoint1 - aPoint0).Length() * 0.5f)
	{
	}

	SC_Sphere::~SC_Sphere()
	{
	}

	SC_Sphere SC_Sphere::FromAABB(const SC_AABB& anAABB)
	{
		return SC_Sphere(anAABB.myMin, anAABB.myMax);
	}

	bool SC_Sphere::IsInside(const SC_Vector3f& aPoint)
	{
		if ((myCenter - aPoint).Length2() < (myRadius * myRadius))
			return true;

		return false;
	}

	void SC_Sphere::Merge(const SC_Sphere& aSphere)
	{
		if (!aSphere.myRadius)
			return;

		if (!myRadius)
		{
			*this = aSphere;
			return;
		}

		SC_Vector3f centerDiff = aSphere.myCenter - myCenter;
		float centerDiffLen = centerDiff.Length();
		float objectRadius = aSphere.myRadius;

		if ((centerDiffLen + myRadius) <= objectRadius)
		{
			*this = aSphere;
			return;
		}

		float sum = centerDiffLen + objectRadius;
		if (sum <= myRadius)
			return;

		myCenter += centerDiff * ((sum - myRadius) / (2.f * centerDiffLen));
		myRadius = (sum + myRadius) * 0.5f;
	}
}