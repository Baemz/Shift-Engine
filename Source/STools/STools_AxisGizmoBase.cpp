#include "STools_Precompiled.h"
#include "STools_AxisGizmoBase.h"

namespace Shift
{
	const SC_Vector4f STools_AxisGizmoBase::ourHighlighColor = { 1.0f, 0.89f, 0.019f, 1.0f };

	const SC_Vector4f STools_AxisGizmoBase::ourAxisColor[STools_AxisGizmoBase::Axis::COUNT] = {
		{1.0f, 0.0f, 0.0f, 1.0f},
		{0.0f, 1.0f, 0.0f, 1.0f},
		{0.0f, 0.0f, 1.0f, 1.0f},
	};

	const SC_Vector3f STools_AxisGizmoBase::ourAxisDirection[STools_AxisGizmoBase::Axis::COUNT] = {
		{1.0f, 0.0f, 0.0f},
		{0.0f, 1.0f, 0.0f},
		{0.0f, 0.0f, 1.0f},
	};

	const SC_Vector3f STools_AxisGizmoBase::ourUpVectors[STools_AxisGizmoBase::Axis::COUNT] = {
		{0.0f, 0.0f, 1.0f},
		{1.0f, 0.0f, 0.0f},
		{0.0f, 1.0f, 0.0f},
	};
}