#pragma once

namespace Shift
{
	// Must match "Data/ShiftEngine/Shaders/SRender/SR_ShadowConstants.sshf"

	struct SG_ShadowConstants
	{
		SC_Matrix44 myShadowWorldToClip[4]; // Match number with SG_Shadows::ourNumShadowCascades
		SC_Vector4f myShadowSampleRotation;
		float myPCFFilterOffset;
	};
}