#pragma once
namespace Shift
{
	// Must reflect "Data/ShiftEngine/Shaders/SRender/SR_ViewConstants.shaderh"

	struct SR_ViewConstants
	{
		SC_Matrix44 myWorldToClip;
		SC_Matrix44 myClipToWorld;

		SC_Matrix44 myWorldToCamera;
		SC_Matrix44 myCameraToWorld;

		SC_Matrix44 myCameraToClip;
		SC_Matrix44 myClipToCamera;

		SC_Vector3f myCameraPosition;
		float myCameraFov;
		float myCameraNear;
		float myCameraFar;

		SC_Vector2f myResolution;
		uint myFrameIndex;
	};
}