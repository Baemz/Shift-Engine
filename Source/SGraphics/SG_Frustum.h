#pragma once
#include "SC_Plane.h"

namespace Shift
{
	class SC_Sphere;
	class SG_Frustum
	{
	public:
		enum SG_FrustumPlane
		{
			SG_FrustumPlane_Near = 0,
			SG_FrustumPlane_Right,
			SG_FrustumPlane_Left,
			SG_FrustumPlane_Bottom,
			SG_FrustumPlane_Top,
			SG_FrustumPlane_Far,
			SG_FrustumPlane_Plane_Count,
		};

		SG_Frustum();
		~SG_Frustum();

		void Update(const SC_Matrix44& aView, const SC_Matrix44& aProj);

		bool Intersects(const SC_AABB& aAABB) const;
		bool Intersects(const SC_Sphere& aSphere) const;

	private:
		SC_Planef myPlanes[SG_FrustumPlane_Plane_Count];
	};
}

