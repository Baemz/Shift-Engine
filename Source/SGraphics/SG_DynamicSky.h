#pragma once
#include "SC_RefCounted.h"

namespace Shift
{
	class SG_View;
	class SG_Camera;
	struct SG_EnvironmentConstants;

	class SR_Texture;
	class SR_ShaderState;

	class SG_DynamicSky : public SC_RefCounted
	{
		friend class STools_DynamicSky;
	public:
		SG_DynamicSky();
		~SG_DynamicSky();

		void Init();

		void PrepareEnvironmentConstants(SG_EnvironmentConstants& aConstants);

		void Update(float aDeltaTime);
		void Render(SG_View* aView);
		void RenderSkyProbeFace(SG_Camera& aCamera, SR_Texture* aFaceRT, const uint aFaceIndex);
		void RenderSkyProbeIrradianceFace(SG_Camera& aCamera, SR_Texture* aFaceRT, SR_Texture* aSkyTexture, const uint aFaceIndex);

		void OverrideTimeOfDay();
		float GetTimeOfDay() const;

		SC_Vector3f GetToSunDirection() const;

		SC_Vector3f mySunLightColorConst;
		SC_Vector3f mySunLightColor;
		float mySunIntensity;
		float myTimeOfDaySpeed;
	private:
		SC_Matrix33 CalculateNorthAlignmentRotation();
		SC_Vector3f GetCelestialObjectDirection(float aLST, float aRA, float aDec);
		SC_Vector3f CalculateToSunDirection(float aTime, float& aLSTOut);

		SC_Vector3f myToSunDirection;
		float myTimeOfDay;

		float myZDirection;
		float myLatitudeN;
		float myLongitudeE;
		uint myDay;
		uint myMonth;
		uint myYear;
		float myUTCOffset;

		float mySunRadiusDivDistance;
		float mySunCosLow;
		float mySunCosHigh;
		float myDirectSunFactor;
		float myAmbientLightIntensity;

		SC_Ref<SR_Texture> mySkyTexture;
		SC_Ref<SR_ShaderState> mySkyShader;
		SC_Ref<SR_ShaderState> mySkyProbeShader;
		SC_Ref<SR_ShaderState> mySkyProbeIrradianceShader;
	};
}