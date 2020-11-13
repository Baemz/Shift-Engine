#pragma once
#include "SR_GBufferDefines.h"
#include "SR_TextureResource.h"

namespace Shift
{
	class SR_GBuffer
	{
	public:
		SR_GBuffer();
		~SR_GBuffer();

		void Update();

		SR_TextureResource myColors;
		SR_TextureResource myNormals;
		SR_TextureResource myMaterials;

		SR_TextureResource myEmissive;
		SR_TextureResource myMotionVectors;

	private:
	};
}
