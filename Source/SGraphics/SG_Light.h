#pragma once
#include "SC_Resource.h"

namespace Shift
{
	enum SG_LightType
	{
		SG_LightType_Unknown = 0,
		SG_LightType_Directional,
		SG_LightType_Pointlight,
		SG_LightType_Spotlight,
	};

	class SG_Light : public SC_Resource
	{
	public:
		SG_Light() = delete;
		SG_Light(const SG_LightType& aType = SG_LightType_Unknown);
		virtual ~SG_Light();


		struct GPUData
		{
			SC_Vector4f myPositionAndRange;
			SC_Vector4f myColorAndIntensity;
			SC_Vector4f myDirectionAndAngle;
			uint myType;
			SC_Vector3ui _notUsed;
		};

		virtual GPUData GetGPUData() const = 0;

		SG_LightType GetType() const;

		SC_Vector3f GetColor() const;
		void SetColor(const SC_Vector3f& aColor);

		float GetIntensity() const;
		void SetIntensity(float aIntensity);

	protected:
		SC_Vector3f myColor;
		float myIntensity;
		SG_LightType myType;
	};
}