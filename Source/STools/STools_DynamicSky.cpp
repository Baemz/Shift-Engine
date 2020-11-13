#include "STools_Precompiled.h"
#include "STools_DynamicSky.h"

#include "SG_DynamicSky.h"
#include "SG_Shadows.h"

namespace Shift
{
	STools_DynamicSky::STools_DynamicSky(SG_DynamicSky* aSky)
		: myDynamicSky(aSky)
		, myOverridePCFFilter(false)
	{
	}

	STools_DynamicSky::~STools_DynamicSky()
	{
	}

	void STools_DynamicSky::Render()
	{
		ImGui::SliderFloat("Sun Intensity", &myDynamicSky->mySunIntensity, 0.0f, 20.f);
		ImGui::SliderFloat("Ambience Intensity", &myDynamicSky->myAmbientLightIntensity, 0.0f, 20.f);
		ImGui::SliderFloat("Time Of Day Speed", &myDynamicSky->myTimeOfDaySpeed, -10.0f, 10.0f);
		ImGui::Checkbox("Lock Shadow Camera", &SG_Shadows::lockCamera);
		ImGui::Checkbox("Override PCF Filter", &myOverridePCFFilter);
		if (myOverridePCFFilter)
			ImGui::SliderFloat("PCF Filter Scale", &SG_Shadows::ourPCFFilterScaleOverride, 0.001f, 1.f);
		else
			SG_Shadows::ourPCFFilterScaleOverride = -1.0f;
	}
}