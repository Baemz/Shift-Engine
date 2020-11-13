#pragma once
#include "SG_Light.h"

namespace Shift
{
	class SG_DirectionalLight : public SG_Light
	{
	public:
		SG_DirectionalLight();
		~SG_DirectionalLight();

		void SetDirection(const SC_Vector3f& aDirection);
		const SC_Vector3f& GetDirection() const;
		const SC_Vector3f GetToDirection() const;

		void SetColor(const SC_Color& aColor);
		const SC_Color& GetColor() const;

		GPUData GetGPUData() const override;

	private:
		SC_Color myColor;
		SC_Vector3f myDirection;
	};
}