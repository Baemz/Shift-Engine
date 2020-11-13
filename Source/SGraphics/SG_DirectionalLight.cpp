#include "SGraphics_Precompiled.h"
#include "SG_DirectionalLight.h"

namespace Shift
{
	SG_DirectionalLight::SG_DirectionalLight()
		: SG_Light(SG_LightType_Directional)
	{
	}

	SG_DirectionalLight::~SG_DirectionalLight()
	{
	}

	void SG_DirectionalLight::SetDirection(const SC_Vector3f& aDirection)
	{
		myDirection = aDirection;
	}

	const SC_Vector3f& SG_DirectionalLight::GetDirection() const
	{
		return myDirection;
	}

	const SC_Vector3f SG_DirectionalLight::GetToDirection() const
	{
		return -(myDirection);
	}

	void SG_DirectionalLight::SetColor(const SC_Color& aColor)
	{
		myColor = aColor;
	}

	const SC_Color& SG_DirectionalLight::GetColor() const
	{
		return myColor;
	}

	SG_Light::GPUData SG_DirectionalLight::GetGPUData() const
	{
		return GPUData();
	}
}