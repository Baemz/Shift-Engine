#pragma once
#include "SG_Frustum.h"
#include "SC_DirtyFlag.h"

namespace Shift
{
	class SG_Camera
	{
	public:
		SG_Camera();
		~SG_Camera();

		void InitAsPerspective(const SC_Vector2f& aResolution, float aFOV = 75.f, float aNear = 0.01f, float aFar = 1000.f, bool aReverseZ = false);
		void InitAsOrthogonal(const SC_Vector2f& aResolution, float aNear = 1.0f, float aFar = 100.f, bool aReverseZ = false);
		void InitAsOrthogonal(float aLeft, float aRight, float aBottom, float aTop, float aNear, float aFar);

		const SC_Vector3f GetPosition() const;
		void SetPosition(const SC_Vector3f& aPosition);

		SC_Matrix44 GetView() const;
		SC_Matrix44 GetInverseView() const;
		SC_Matrix44 GetProjection() const;
		SC_Matrix44 GetInverseProjection() const;
		SC_Matrix44 GetTransform() const;

		void SetTransform(const SC_Matrix44& aTransform);

		void GenerateVerticesOnPlane(float aZDistance, SC_Vector3f* aOutVertices) const;

		SC_Vector3f Project(const SC_Vector3f& aPos) const;
		SC_Vector3f Unproject(const SC_Vector3f& aPos) const;
		SC_Ray GetRayFrom2D(const SC_Vector2f& aPos2D, float aDepth = 0.0f);
		SC_Vector3f GetPosFrom2D(const SC_Vector2f& aPos2D, float aDepth = 0.0f);


		void Move(const SC_Vector3f& aAxis, float aSpeed);
		void LookAt(const SC_Vector3f& aTarget);
		void LookAt(const SC_Vector3f& aStartPos, const SC_Vector3f& aTarget);
		void LookAt(const SC_Vector3f& aStartPos, const SC_Vector3f& aTarget, const SC_Vector3f& aUp);

		float GetNear() const;
		float GetFar() const;
		float GetFov() const;
		float GetFovInRadians() const;
		float GetAspectRatio() const;
		const SC_Vector3f& GetForward() const;
		const SC_Vector3f& GetRight() const;
		const SC_Vector3f& GetUp() const;
		const SC_Vector3f GetXAxis() const;
		const SC_Vector3f GetYAxis() const;
		const SC_Vector3f GetZAxis() const;

		const SG_Frustum& GetFrustum() const;

		void DebugDrawFrustum() const;

	private:
		void UpdateIfDirty() const;
		void UpdateVectors() const;

		mutable SG_Frustum myFrustum;
		mutable SC_Matrix44 myTransform;
		mutable SC_Matrix44 myProjection;
		mutable SC_Matrix44 myInverseProjection;
		mutable SC_Matrix44 myView;
		mutable SC_Vector3f myForward;
		mutable SC_Vector3f myUp;
		mutable SC_Vector3f myRight;
		mutable SC_DirtyFlag<1> myDirtyFlag;

		float myNear;
		float myFar;
		union
		{
			struct
			{
				float myFov;
				float myAspectRatio;
			};
			SC_Vector2f myResolution;
		};
		bool myIsOrho;
	};

}