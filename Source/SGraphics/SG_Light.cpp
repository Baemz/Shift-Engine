#include "SGraphics_Precompiled.h"
#include "SG_Light.h"

namespace Shift 
{
	SG_Light::SG_Light(const SG_LightType& aType)
		: myType(aType)
		, myColor(1.0f, 1.0f, 1.0f)
		, myIntensity(1.0f)
	{
	}

	SG_Light::~SG_Light()
	{
	}

	SG_LightType SG_Light::GetType() const
	{
		return myType;
	}

	SC_Vector3f SG_Light::GetColor() const
	{
		return myColor;
	}

	void SG_Light::SetColor(const SC_Vector3f& aColor)
	{
		myColor = aColor;
	}

	float SG_Light::GetIntensity() const
	{
		return myIntensity;
	}

	void SG_Light::SetIntensity(float aIntensity)
	{
		myIntensity = aIntensity;
	}

}
