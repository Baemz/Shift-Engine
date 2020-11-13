#pragma once


namespace Shift
{
	enum SR_GBufferPart
	{
		SR_GBufferPart_Color, // RGBA8_sRGB
		SR_GBufferPart_Normal, // RG11B10
		SR_GBufferPart_Material, // RGBA8

		SR_GBufferPart_COUNT,

		SR_GBufferPart_Optional_Emissive = SR_GBufferPart_COUNT,
		SR_GBufferPart_Optional_MotionVector,

		SR_GBufferPart_FULLCOUNT
	};

	enum SR_GBufferPermutation
	{
		SR_GBufferPermutation_Emissive = 0x1,
		SR_GBufferPermutation_MotionVector = 0x2,
		SR_GBufferPermutation_COUNT = 0x4,

	};
}