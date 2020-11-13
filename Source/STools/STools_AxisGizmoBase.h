#pragma once
namespace Shift
{
	class SG_Camera;
	class STools_AxisGizmoBase
	{
	public:
		enum Axis
		{
			Right,
			Up,
			Forward,
			COUNT,
		};

		STools_AxisGizmoBase() {}
		virtual ~STools_AxisGizmoBase() {}

		virtual void Interact(SC_Vector3f& aPositionInOut, const SC_Vector2f& aCursorPos) = 0;
		virtual void Interact(SC_Vector3f& aPositionInOut) = 0;
		virtual void Render() = 0;

		SC_Vector3f myPosition;

	protected:
		static const SC_Vector4f ourHighlighColor;
		static const SC_Vector4f ourAxisColor[Axis::COUNT];
		static const SC_Vector3f ourAxisDirection[Axis::COUNT];
		static const SC_Vector3f ourUpVectors[Axis::COUNT];
	};
}

