#pragma once
#include "SG_Light.h"

namespace Shift
{
	class SG_Pointlight : public SG_Light
	{
	public:
		SG_Pointlight();
		~SG_Pointlight();

		SC_Vector3f GetPosition() const;
		void SetPosition(const SC_Vector3f& aPosition);

		float GetRange() const;
		void SetRange(float aRange);

		GPUData GetGPUData() const override;

	private:
		SC_Vector3f myPosition;
		float myRange;
	};

}