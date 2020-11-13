#pragma once
#include "SC_Vector.h"
#include "SC_AABB.h"

namespace Shift
{
	class SC_Sphere
	{
	public:
		SC_Sphere();
		SC_Sphere(const SC_Vector3f& aPoint0, const SC_Vector3f& aPoint1);
		~SC_Sphere();

		static SC_Sphere FromAABB(const SC_AABB& anAABB);

		bool IsInside(const SC_Vector3f& aPoint);

		void Merge(const SC_Sphere& aSphere);

		SC_Vector3f myCenter;
		float myRadius;
	};
}