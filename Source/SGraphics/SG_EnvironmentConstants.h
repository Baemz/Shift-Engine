#pragma once

namespace Shift
{
	// Must match "Data/ShiftEngine/Shaders/SRender/SR_EnvrionmentConstants.sshf"

	struct SG_EnvironmentConstants
	{
		SC_Vector3f myToSunDirection;
		float mySunIntensity;
		SC_Vector3f mySunColor;
		float mySunCosLow;
		float mySunCosHigh;
		float myDirectSunFactor;
		float myAmbientLightIntensity;
	};
}