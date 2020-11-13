#pragma once
#include "STools_AxisGizmoBase.h"

namespace Shift
{
	class SG_Camera;
	class STools_TransformGizmo : public STools_AxisGizmoBase
	{
	public:

		STools_TransformGizmo(SG_Camera& aCamera);
		~STools_TransformGizmo();

		void Interact(SC_Vector3f& aPositionInOut, const SC_Vector2f& aCursorPos) override;
		void Interact(SC_Vector3f& aPositionInOut) override;
		void Render() override;

	private:
		void DrawAxisArrow(uint aAxis);
		void DrawArrowTriangle(const SC_Matrix44& aTransform, const SC_Vector4f& aColor);

		SG_Camera& myCamera;
		SC_Vector3f myStartDrag;
		SC_AABB myAABBs[Axis::COUNT];
		SC_Vector2f myMousePosLastFrame;
		bool myFocused[Axis::COUNT];
		bool myDidStartDrag;
	};
}
