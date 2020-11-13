#include "SGraphics_Precompiled.h"
#include "SG_Camera.h"

namespace Shift
{
	SG_Camera::SG_Camera()
		: myIsOrho(false)
	{
	}

	SG_Camera::~SG_Camera()
	{
	}

	void SG_Camera::InitAsPerspective(const SC_Vector2f& aResolution, float aFOV, float aNear, float aFar, bool aReverseZ)
	{
		myProjection = SC_Matrix44();

		float farPlane = aFar;
		float nearPlane = aNear;
		if (aReverseZ)
		{
			nearPlane = aFar;
			farPlane = aNear;
		}

		float aspectRatio = (aResolution.x / aResolution.y);
		float xScale = 1.0f / tanf(SC_DegreesToRadians(aFOV) * 0.5f);
		float yScale = xScale * aspectRatio;
		float zScale = nearPlane / (nearPlane - farPlane);


		myProjection.my11 = xScale;
		myProjection.my22 = yScale;
		myProjection.my33 = zScale;
		myProjection.my34 = 1.0f;
		myProjection.my43 = -zScale * farPlane;
		myProjection.my44 = 0.f;

		myProjection.GetInverse(myInverseProjection);
		myFov = aFOV;
		myAspectRatio = aspectRatio;
		myNear = nearPlane;
		myFar = farPlane;
		myIsOrho = false;
		assert(myDirtyFlag.myValue != 2);
		myDirtyFlag.myValue = 1;
	}

	void SG_Camera::InitAsOrthogonal(const SC_Vector2f& aResolution, float aNear, float aFar, bool aReverseZ)
	{
		myProjection = SC_Matrix44();

		float farPlane = aFar;
		float nearPlane = aNear;
		
		if (aReverseZ)
		{
			nearPlane = aFar;
			farPlane = aNear;
		}
		
		float xScale = 2.0f / aResolution.x;
		float yScale = 2.0f / aResolution.y;
		const float zScale = 1.0f / (nearPlane - farPlane);

		myProjection.my11 = xScale;
		myProjection.my22 = yScale;
		myProjection.my33 = zScale;
		myProjection.my43 = -zScale * farPlane;
		myProjection.my44 = 1.0f;
		
		myProjection.GetInverse(myInverseProjection);
		myNear = nearPlane;
		myFar = farPlane;
		myIsOrho = true;
		myResolution = aResolution;
		assert(myDirtyFlag.myValue != 2);
		myDirtyFlag.myValue = 1;
	}

	void SG_Camera::InitAsOrthogonal(float aLeft, float aRight, float aBottom, float aTop, float aNear, float aFar)
	{
		myProjection = SC_Matrix44();
		myProjection.my11 = 2 / (aRight - aLeft);
		myProjection.my22 = 2 / (aTop - aBottom);
		myProjection.my33 = 1 / (aFar - aNear);
		myProjection.my41 = (aLeft + aRight) / (aLeft - aRight);
		myProjection.my42 = (aTop + aBottom) / (aBottom - aTop);
		myProjection.my43 = aNear / (aNear - aFar);
		myProjection.my44 = 1.0f;
		myNear = aNear;
		myFar = aFar;
		myProjection.GetInverse(myInverseProjection);
		myIsOrho = true;
		assert(myDirtyFlag.myValue != 2);
		myDirtyFlag.myValue = 1;
	}

	const SC_Vector3f SG_Camera::GetPosition() const
	{
		UpdateIfDirty();
		return myTransform.GetPosition();
	}

	void SG_Camera::SetPosition(const SC_Vector3f& aPosition)
	{
		myTransform.SetPosition(aPosition);
		assert(myDirtyFlag.myValue != 2);
		myDirtyFlag.myValue = 1;
	}

	SC_Matrix44 SG_Camera::GetView() const
	{
		UpdateIfDirty();
		return myView;
	}

	SC_Matrix44 SG_Camera::GetInverseView() const
	{
		UpdateIfDirty();
		return myTransform;
	}

	SC_Matrix44 SG_Camera::GetProjection() const
	{
		UpdateIfDirty();
		return myProjection;
	}

	SC_Matrix44 SG_Camera::GetInverseProjection() const
	{
		UpdateIfDirty();
		return myInverseProjection;
	}

	SC_Matrix44 SG_Camera::GetTransform() const
	{
		UpdateIfDirty();
		return myTransform;
	}

	void SG_Camera::SetTransform(const SC_Matrix44& aTransform)
	{
		myTransform = aTransform;
		assert(myDirtyFlag.myValue != 2);
		myDirtyFlag.myValue = 1;
	}

	void SG_Camera::GenerateVerticesOnPlane(float aZDistance, SC_Vector3f* aOutVertices) const
	{
		assert(aOutVertices != NULL);
		UpdateIfDirty();

		const SC_Vector3f axisX(1.0f, 0.0f, 0.0f);
		const SC_Vector3f axisY(0.0f, 1.0f, 0.0f);
		const SC_Vector3f axisZ(0.0f, 0.0f, 1.0f);
		if (myIsOrho)
		{
			const float xScale = myResolution.x * 0.5f;
			const float yScale = myResolution.y * 0.5f;
			aOutVertices[0] = (axisZ * aZDistance + axisY * yScale + axisX * xScale) * myTransform;
			aOutVertices[1] = (axisZ * aZDistance + axisY * yScale - axisX * xScale) * myTransform;
			aOutVertices[2] = (axisZ * aZDistance - axisY * yScale + axisX * xScale) * myTransform;
			aOutVertices[3] = (axisZ * aZDistance - axisY * yScale - axisX * xScale) * myTransform;
		}
		else
		{
			float xScale = aZDistance * tanf(SC_DegreesToRadians(myFov) * 0.5f);
			float yScale = xScale / myAspectRatio;
			aOutVertices[0] = (axisZ * aZDistance + axisY * yScale + axisX * xScale) * myTransform;
			aOutVertices[1] = (axisZ * aZDistance + axisY * yScale - axisX * xScale) * myTransform;
			aOutVertices[2] = (axisZ * aZDistance - axisY * yScale + axisX * xScale) * myTransform;
			aOutVertices[3] = (axisZ * aZDistance - axisY * yScale - axisX * xScale) * myTransform;
		}
	}

	SC_Vector3f SG_Camera::Project(const SC_Vector3f& aPos) const
	{
		UpdateIfDirty();
		SC_Vector4f pos = SC_Vector4f(aPos, 1.0f);
		pos *= myView;
		pos *= myProjection;
		return pos.XYZ() / pos.w;
	}

	SC_Vector3f SG_Camera::Unproject(const SC_Vector3f& aClipPos) const
	{
		UpdateIfDirty();
		SC_Vector4f pos(aClipPos, 1.0f);
		pos *= myInverseProjection;
		pos *= myTransform;
		return pos;
	}

	SC_Ray SG_Camera::GetRayFrom2D(const SC_Vector2f& aPos2D, float aDepth)
	{
		UpdateIfDirty();
		float x = (aPos2D.x * 2.f) - 1.f;
		float y = -((aPos2D.y * 2.f) - 1.f);

		x = x / myProjection.my11;
		y = y / myProjection.my22;

		SC_Matrix44 inverseView = GetInverseView();
		SC_Vector3f dir;
		dir.x = (x * inverseView.my11) + (y * inverseView.my21) + (aDepth * inverseView.my31);
		dir.y = (x * inverseView.my12) + (y * inverseView.my22) + (aDepth * inverseView.my32);
		dir.z = (x * inverseView.my13) + (y * inverseView.my23) + (aDepth * inverseView.my33);
		dir.Normalize();

		SC_Ray ray;
		ray.myOrigin = inverseView.GetPosition();
		ray.myDirection = dir;

		return ray;
	}

	SC_Vector3f SG_Camera::GetPosFrom2D(const SC_Vector2f& aPos2D, float aDepth)
	{
		SC_Vector4f pos(aPos2D, aDepth);
		pos *= myInverseProjection;
		pos *= myTransform;
		return pos;
	}

	void SG_Camera::Move(const SC_Vector3f& aAxis, float aSpeed)
	{
		myTransform.SetPosition(myTransform.GetPosition() + (aAxis * aSpeed));
		assert(myDirtyFlag.myValue != 2);
		myDirtyFlag.myValue = 1;
	}

	void SG_Camera::LookAt(const SC_Vector3f& aTarget)
	{
		LookAt(GetPosition(), aTarget);
	}

	void SG_Camera::LookAt(const SC_Vector3f& aStartPos, const SC_Vector3f& aTarget)
	{
		static const SC_Vector3f UpVec = SC_Vector3f(0.0f, 1.0f, 0.0f);
		LookAt(aStartPos, aTarget, UpVec);
	}

	void SG_Camera::LookAt(const SC_Vector3f& aStartPos, const SC_Vector3f& aTarget, const SC_Vector3f& aUp)
	{
		assert(fabs(aStartPos.x) < 1.6e5f);
		assert(fabs(aStartPos.y) < 1.1e5f);
		assert(fabs(aStartPos.z) < 1.6e5f);

		SC_Vector3f direction = aTarget - aStartPos;

		SC_Matrix33 lookAt;
		lookAt.MakeLookAt(direction, aUp);
		myTransform.Set33(lookAt);
		myTransform.SetPosition(aStartPos);

		assert(myDirtyFlag.myValue != 2);
		myDirtyFlag.myValue = 1;
	}

	float SG_Camera::GetNear() const
	{
		UpdateIfDirty();
		return myNear;
	}

	float SG_Camera::GetFar() const
	{
		UpdateIfDirty();
		return myFar;
	}

	float SG_Camera::GetFov() const
	{
		UpdateIfDirty();
		return myFov;
	}

	float SG_Camera::GetFovInRadians() const
	{
		UpdateIfDirty();
		return SC_DegreesToRadians(myFov);
	}

	float SG_Camera::GetAspectRatio() const
	{
		UpdateIfDirty();
		return myAspectRatio;
	}

	void SG_Camera::UpdateIfDirty() const
	{
		if (!myDirtyFlag.IsDirty())
			return;

		UpdateVectors();
		myTransform.GetInverse(myView);
		myFrustum.Update(myView, myProjection);
		myDirtyFlag.myValue = 0;
	}

	void SG_Camera::UpdateVectors() const
	{
		//Local basis vectors
		myRight.x = myTransform[0];
		myRight.y = myTransform[1];
		myRight.z = myTransform[2];
		myRight.Normalize();
		myUp.x = myTransform[4];
		myUp.y = myTransform[5];
		myUp.z = myTransform[6];
		myUp.Normalize();
		myForward.x = myTransform[8];
		myForward.y = myTransform[9];
		myForward.z = myTransform[10];
		myForward.Normalize();
	}

	const SC_Vector3f& SG_Camera::GetForward() const
	{
		UpdateIfDirty();
		return myForward;
	}

	const SC_Vector3f& SG_Camera::GetRight() const
	{
		UpdateIfDirty();
		return myRight;
	}

	const SC_Vector3f& SG_Camera::GetUp() const
	{
		UpdateIfDirty();
		return myUp;
	}

	const SC_Vector3f SG_Camera::GetXAxis() const
	{
		UpdateIfDirty();
		return myTransform.GetXAxis();
	}

	const SC_Vector3f SG_Camera::GetYAxis() const
	{
		UpdateIfDirty();
		return myTransform.GetYAxis();
	}

	const SC_Vector3f SG_Camera::GetZAxis() const
	{
		UpdateIfDirty();
		return myTransform.GetZAxis();
	}

	const SG_Frustum& SG_Camera::GetFrustum() const
	{
		UpdateIfDirty();
		return myFrustum;
	}

	void SG_Camera::DebugDrawFrustum() const
	{
		SC_Vector3f corners[8];
		GenerateVerticesOnPlane(myNear, corners);
		GenerateVerticesOnPlane(myFar, corners + 4);

		SC_Color col(0.f, 1.f, 0.5f, 1.f);

		SG_PRIMITIVE_DRAW_LINE3D_COLORED(corners[0], corners[1], col);
		SG_PRIMITIVE_DRAW_LINE3D_COLORED(corners[1], corners[3], col);
		SG_PRIMITIVE_DRAW_LINE3D_COLORED(corners[3], corners[2], col);
		SG_PRIMITIVE_DRAW_LINE3D_COLORED(corners[2], corners[0], col);
		SG_PRIMITIVE_DRAW_LINE3D_COLORED(corners[0], corners[4], col);
		SG_PRIMITIVE_DRAW_LINE3D_COLORED(corners[4], corners[5], col);
		SG_PRIMITIVE_DRAW_LINE3D_COLORED(corners[5], corners[1], col);
		SG_PRIMITIVE_DRAW_LINE3D_COLORED(corners[1], corners[5], col);
		SG_PRIMITIVE_DRAW_LINE3D_COLORED(corners[5], corners[7], col);
		SG_PRIMITIVE_DRAW_LINE3D_COLORED(corners[7], corners[3], col);
		SG_PRIMITIVE_DRAW_LINE3D_COLORED(corners[3], corners[7], col);
		SG_PRIMITIVE_DRAW_LINE3D_COLORED(corners[7], corners[6], col);
		SG_PRIMITIVE_DRAW_LINE3D_COLORED(corners[6], corners[2], col);
		SG_PRIMITIVE_DRAW_LINE3D_COLORED(corners[2], corners[6], col);
		SG_PRIMITIVE_DRAW_LINE3D_COLORED(corners[6], corners[4], col);

	}
}