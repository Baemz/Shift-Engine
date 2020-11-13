#include "SGraphics_Precompiled.h"
#include "SG_Frustum.h"

namespace Shift
{
	SG_Frustum::SG_Frustum()
	{
	}
	SG_Frustum::~SG_Frustum()
	{
	}
	void SG_Frustum::Update(const SC_Matrix44& aView, const SC_Matrix44& aProj)
	{
		const SC_Matrix44 viewProj = aView * aProj; 

		myPlanes[SG_FrustumPlane_Right].myNormal.x = viewProj[3] - viewProj[0];
		myPlanes[SG_FrustumPlane_Right].myNormal.y = viewProj[7] - viewProj[4];
		myPlanes[SG_FrustumPlane_Right].myNormal.z = viewProj[11] - viewProj[8];
		myPlanes[SG_FrustumPlane_Right].myDistance = viewProj[15] - viewProj[12];
		myPlanes[SG_FrustumPlane_Right].Normalize();

		myPlanes[SG_FrustumPlane_Left].myNormal.x = viewProj[3] + viewProj[0];
		myPlanes[SG_FrustumPlane_Left].myNormal.y = viewProj[7] + viewProj[4];
		myPlanes[SG_FrustumPlane_Left].myNormal.z = viewProj[11] + viewProj[8];
		myPlanes[SG_FrustumPlane_Left].myDistance = viewProj[15] + viewProj[12];
		myPlanes[SG_FrustumPlane_Left].Normalize();

		myPlanes[SG_FrustumPlane_Top].myNormal.x = viewProj[3] - viewProj[1];
		myPlanes[SG_FrustumPlane_Top].myNormal.y = viewProj[7] - viewProj[5];
		myPlanes[SG_FrustumPlane_Top].myNormal.z = viewProj[11] - viewProj[9];
		myPlanes[SG_FrustumPlane_Top].myDistance = viewProj[15] - viewProj[13];
		myPlanes[SG_FrustumPlane_Top].Normalize();

		myPlanes[SG_FrustumPlane_Bottom].myNormal.x = viewProj[3] + viewProj[1];
		myPlanes[SG_FrustumPlane_Bottom].myNormal.y = viewProj[7] + viewProj[5];
		myPlanes[SG_FrustumPlane_Bottom].myNormal.z = viewProj[11] + viewProj[9];
		myPlanes[SG_FrustumPlane_Bottom].myDistance = viewProj[15] + viewProj[13];
		myPlanes[SG_FrustumPlane_Bottom].Normalize();

		myPlanes[SG_FrustumPlane_Near].myNormal.x = viewProj[3] - viewProj[2];
		myPlanes[SG_FrustumPlane_Near].myNormal.y = viewProj[7] - viewProj[6];
		myPlanes[SG_FrustumPlane_Near].myNormal.z = viewProj[11] - viewProj[10];
		myPlanes[SG_FrustumPlane_Near].myDistance = viewProj[15] - viewProj[14];
		myPlanes[SG_FrustumPlane_Near].Normalize();

		myPlanes[SG_FrustumPlane_Far].myNormal.x = viewProj[2];
		myPlanes[SG_FrustumPlane_Far].myNormal.y = viewProj[6];
		myPlanes[SG_FrustumPlane_Far].myNormal.z = viewProj[10];
		myPlanes[SG_FrustumPlane_Far].myDistance = viewProj[14];
		myPlanes[SG_FrustumPlane_Far].Normalize();
	}

	bool SG_Frustum::Intersects(const SC_AABB& aAABB) const
	{
		bool intersects = true;
		for (uint i = 0; i < SG_FrustumPlane_Plane_Count; ++i)
		{
			SC_Vector3f axisVert;
			if (myPlanes[i].myNormal.x < 0.0f)
				axisVert.x = aAABB.myMin.x;
			else
				axisVert.x = aAABB.myMax.x;

			if (myPlanes[i].myNormal.y < 0.0f)
				axisVert.y = aAABB.myMin.y;
			else
				axisVert.y = aAABB.myMax.y;

			if (myPlanes[i].myNormal.z < 0.0f)
				axisVert.z = aAABB.myMin.z;
			else
				axisVert.z = aAABB.myMax.z;

			if (myPlanes[i].DistanceToPlane(axisVert) < 0.0f)
			{
				intersects = false;
				break;
			}
		}

		return intersects;
	}

	bool SG_Frustum::Intersects(const SC_Sphere& aSphere) const
	{
		float minDist = 0.0f;
		for (int i = 0; i < SG_FrustumPlane_Plane_Count; ++i)
		{
			float dist = myPlanes[i].DistanceToPlane(aSphere.myCenter);
			minDist = SC_Min(minDist, dist);
		}

		return minDist > (-aSphere.myRadius);
	}
}