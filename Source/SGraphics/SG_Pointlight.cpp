#include "SGraphics_Precompiled.h"
#include "SG_Pointlight.h"

namespace Shift
{
	SG_Pointlight::SG_Pointlight()
		: SG_Light(SG_LightType_Pointlight)
	{
	}

	SG_Pointlight::~SG_Pointlight()
	{
	}

	SC_Vector3f SG_Pointlight::GetPosition() const
	{
		return myPosition;
	}

	void SG_Pointlight::SetPosition(const SC_Vector3f& aPosition)
	{
		myPosition = aPosition;
	}

	float SG_Pointlight::GetRange() const
	{
		return myRange;
	}

	void SG_Pointlight::SetRange(float aRange)
	{
		myRange = aRange;
	}
	SG_Light::GPUData SG_Pointlight::GetGPUData() const
	{
		GPUData gpuData;

		gpuData.myColorAndIntensity = SC_Vector4f(myColor, myIntensity);
		gpuData.myPositionAndRange = SC_Vector4f(myPosition, myRange);
		gpuData.myType = (uint)myType;

		return gpuData;
	}
}
