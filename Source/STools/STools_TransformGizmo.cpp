#include "STools_Precompiled.h"
#include "STools_TransformGizmo.h"
#include "SG_Camera.h"

namespace Shift
{
	bool locRayVSAABB(const SC_Ray& aRay, const SC_AABB& aAABB, float& t)
	{
		// r.dir is unit direction vector of ray
		SC_Vector3f dirfrac;
		dirfrac.x = 1.0f / aRay.myDirection.x;
		dirfrac.y = 1.0f / aRay.myDirection.y;
		dirfrac.z = 1.0f / aRay.myDirection.z;
		// lb is the corner of AABB with minimal coordinates - left bottom, rt is maximal corner
		// r.org is origin of ray
		float t1 = (aAABB.myMin.x - aRay.myOrigin.x) * dirfrac.x;
		float t2 = (aAABB.myMax.x - aRay.myOrigin.x) * dirfrac.x;
		float t3 = (aAABB.myMin.y - aRay.myOrigin.y) * dirfrac.y;
		float t4 = (aAABB.myMax.y - aRay.myOrigin.y) * dirfrac.y;
		float t5 = (aAABB.myMin.z - aRay.myOrigin.z) * dirfrac.z;
		float t6 = (aAABB.myMax.z - aRay.myOrigin.z) * dirfrac.z;

		float tmin = SC_Max(SC_Max(SC_Min(t1, t2), SC_Min(t3, t4)), SC_Min(t5, t6));
		float tmax = SC_Min(SC_Min(SC_Max(t1, t2), SC_Max(t3, t4)), SC_Max(t5, t6));

		// if tmax < 0, ray (line) is intersecting AABB, but the whole AABB is behind us
		if (tmax < 0)
		{
			t = tmax;
			return false;
		}

		// if tmin > tmax, ray doesn't intersect AABB
		if (tmin > tmax)
		{
			t = tmax;
			return false;
		}

		t = tmin;
		return true;
	}

	STools_TransformGizmo::STools_TransformGizmo(SG_Camera& aCamera)
		: myCamera(aCamera)
	{
	}

	STools_TransformGizmo::~STools_TransformGizmo()
	{
	}

	void STools_TransformGizmo::Interact(SC_Vector3f& aPositionInOut, const SC_Vector2f& aCursorPos)
	{
		SG_Camera* camera = &myCamera;
		if (!myDidStartDrag)
		{
			for (uint i = 0; i < Axis::COUNT; ++i)
				myFocused[i] = false;

			SC_Vector2f cursorPos = aCursorPos;
			SC_Ray ray = camera->GetRayFrom2D(cursorPos, 1.0f);

			float t;
			if (locRayVSAABB(ray, myAABBs[Axis::Right], t))
				myFocused[Axis::Right] = true;
			else if (locRayVSAABB(ray, myAABBs[Axis::Up], t))
				myFocused[Axis::Up] = true;
			else if (locRayVSAABB(ray, myAABBs[Axis::Forward], t))
				myFocused[Axis::Forward] = true;
		}
		SC_Vector2f mousePos = aCursorPos;
		SC_Ray wPos = camera->GetRayFrom2D(mousePos);
		if ((GetKeyState(VK_LBUTTON) & 0x100) != 0)
		{
			if (!myDidStartDrag)
			{
				myStartDrag = aPositionInOut;
				myDidStartDrag = true;
			}
			SC_Vector3f axis;
			bool found = false;
			for (uint i = 0; i < Axis::COUNT; ++i)
			{
				if (myFocused[i])
				{
					axis = ourAxisDirection[i];
					found = true;
					break;
				}
			}

			if (found)
			{
				SC_Ray lwPos = camera->GetRayFrom2D(myMousePosLastFrame);
				float factor = axis.Dot(wPos.myDirection - lwPos.myDirection);
				aPositionInOut = aPositionInOut + axis * factor;
				myPosition = aPositionInOut;
			}
		}
		else
			myDidStartDrag = false;
		myMousePosLastFrame = mousePos;
	}

	void STools_TransformGizmo::Interact(SC_Vector3f& aPositionInOut)
	{
		Interact(aPositionInOut, STools_Editor::GetInstance()->GetViewportCursorPos());
	}

	void STools_TransformGizmo::Render()
	{
		DrawAxisArrow(Right);
		DrawAxisArrow(Up);
		DrawAxisArrow(Forward);
	}

	void STools_TransformGizmo::DrawAxisArrow(uint aAxis)
	{
		const SC_Vector3f& upVector = ourUpVectors[aAxis];
		const SC_Vector3f& directionVector = ourAxisDirection[aAxis];
		const SC_Vector4f axisColor = (myFocused[aAxis]) ? ourHighlighColor : ourAxisColor[aAxis];

		SG_Camera& camera = myCamera;
		SC_Vector3f distanceVector = myPosition - camera.GetPosition();
		float distanceToCamera = distanceVector.Length();
		float radius = 1.0f * (distanceToCamera / 10.f);
		float scaleFactor = radius;

		float width = (radius * 0.02f);
		SC_Vector3f offset = upVector * (width * 0.5f);
		SC_Vector3f right = upVector.Cross(directionVector).GetNormalized();
		offset += right * (width * 0.5f);

		SC_Matrix44 scaleTransform;
		scaleTransform.Scale({ width, width, radius * 0.8f });

		SC_Matrix44 transform;
		transform.SetPosition(myPosition + directionVector * (radius * 0.6f) + offset);
		transform.SetDirection(directionVector, upVector);
		SG_PRIMITIVE_DRAW_BOX3D_COLORED(scaleTransform * transform, axisColor);

		scaleTransform = SC_Matrix44();
		scaleTransform.Scale({ width * 5, width * 5, radius * 1.f });
		myAABBs[aAxis].myMin = SC_Vector3f(-0.5f, -0.5f, -0.5f) * (scaleTransform * transform);
		myAABBs[aAxis].myMax = SC_Vector3f(0.5f, 0.5f, 0.5f) * (scaleTransform * transform);

		scaleTransform = SC_Matrix44();
		scaleTransform.Scale(SC_Vector3f(scaleFactor));
		transform = SC_Matrix44();
		transform.SetDirection(directionVector, upVector);
		transform.SetPosition(myPosition + directionVector * radius);
		transform = scaleTransform * transform;
		DrawArrowTriangle(transform, axisColor);
	}
	void STools_TransformGizmo::DrawArrowTriangle(const SC_Matrix44& aTransform, const SC_Vector4f& aColor)
	{
		SC_Vector3f t1c1 = (SC_Vector4f(0.0f, 0.0f, 0.0f, 1.0f) * aTransform).XYZ();
		SC_Vector3f t1c2 = (SC_Vector4f(0.08f, 0.0f, 0.0f, 1.0f) * aTransform).XYZ();
		SC_Vector3f t1c3 = (SC_Vector4f(0.0f, 0.08f, 0.0f, 1.0f) * aTransform).XYZ();

		SC_Vector3f t2c1 = (SC_Vector4f(0.0f, 0.0f, 0.0f, 1.0f) * aTransform).XYZ();
		SC_Vector3f t2c2 = (SC_Vector4f(0.0f, 0.0f, 0.24f, 1.0f) * aTransform).XYZ();
		SC_Vector3f t2c3 = (SC_Vector4f(0.08f, 0.0f, 0.0f, 1.0f) * aTransform).XYZ();

		SC_Vector3f t3c1 = (SC_Vector4f(0.0f, 0.0f, 0.0f, 1.0f) * aTransform).XYZ();
		SC_Vector3f t3c2 = (SC_Vector4f(0.0f, 0.08f, 0.0f, 1.0f) * aTransform).XYZ();
		SC_Vector3f t3c3 = (SC_Vector4f(0.0f, 0.0f, 0.024f, 1.0f) * aTransform).XYZ();

		SC_Vector3f t4c1 = (SC_Vector4f(0.0f, 0.08f, 0.0f, 1.0f) * aTransform).XYZ();
		SC_Vector3f t4c2 = (SC_Vector4f(0.08f, 0.0f, 0.0f, 1.0f) * aTransform).XYZ();
		SC_Vector3f t4c3 = (SC_Vector4f(0.0f, 0.0f, 0.024f, 1.0f) * aTransform).XYZ();

		SG_PRIMITIVE_DRAW_TRAINGLE3D_COLORED(t1c1, t1c2, t1c3, aColor);
		SG_PRIMITIVE_DRAW_TRAINGLE3D_COLORED(t2c1, t2c2, t2c3, aColor);
		SG_PRIMITIVE_DRAW_TRAINGLE3D_COLORED(t3c1, t3c2, t3c3, aColor);
		SG_PRIMITIVE_DRAW_TRAINGLE3D_COLORED(t4c1, t4c2, t4c3, aColor);
	}
}