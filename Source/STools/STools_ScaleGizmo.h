#pragma once
#include "STools_AxisGizmoBase.h"

namespace Shift
{
	class STools_ScaleGizmo : public STools_AxisGizmoBase
	{
	public:

		STools_ScaleGizmo();
		~STools_ScaleGizmo();

		void Interact(SC_Vector3f& aPositionInOut) override;
		void Render() override;
	private:
		void DrawAxis(uint aAxis);
		void DrawArrowBox(const SC_Matrix44& aTransform, uint aAxis);
	};
}
