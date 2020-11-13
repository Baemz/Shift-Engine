#pragma once

namespace Shift
{
	enum SR_Format : unsigned char
	{
		SR_Format_Unknown,
		SR_Format_RGBA32_Typeless,
		SR_Format_RGBA32_Float,
		SR_Format_RGBA32_Uint,
		SR_Format_RGBA32_Sint,
		SR_Format_RGB32_Typeless,
		SR_Format_RGB32_Float,
		SR_Format_RGB32_Uint,
		SR_Format_RGB32_Sint,
		SR_Format_RGBA16_Typeless,
		SR_Format_RGBA16_Float,
		SR_Format_RGBA16_Unorm,
		SR_Format_RGBA16_Uint,
		SR_Format_RGBA16_Snorm,
		SR_Format_RGBA16_Sint,
		SR_Format_RG16_Typeless,
		SR_Format_RG16_Float,
		SR_Format_RG16_Unorm,
		SR_Format_RG16_Uint,
		SR_Format_RG16_Snorm,
		SR_Format_RG16_Sint,
		SR_Format_RGBA8_Typeless,
		SR_Format_RGBA8_Unorm,
		SR_Format_RGBA8_Unorm_sRGB,
		SR_Format_BGRA8_Unorm,
		SR_Format_BGRA8_Unorm_sRGB,
		SR_Format_RGBA8_Uint,
		SR_Format_RGBA8_Snorm,
		SR_Format_RGBA8_Sint,
		SR_Format_RG32_Typeless,
		SR_Format_RG32_Float,
		SR_Format_RG32_Uint,
		SR_Format_RG32_Sint,
		SR_Format_RGB10A2_Typeless,
		SR_Format_RGB10A2_Unorm,
		SR_Format_RGB10A2_Uint,
		SR_Format_RG11B10_Float,
		SR_Format_R32_Typeless,
		SR_Format_D32_Float,
		SR_Format_R32_Float,
		SR_Format_R32_Uint,
		SR_Format_R32_Sint,
		SR_Format_DXT1,
		SR_Format_DXT1_sRGB,
		SR_Format_DXT3,
		SR_Format_DXT3_sRGB,
		SR_Format_DXT5,
		SR_Format_DXT5_sRGB,
		SR_Format_BC5_Unorm,
		SR_Format_BC5_Snorm,
		SR_Format_BC7,
		SR_Format_BC7_sRGB,

	};

	SC_FORCEINLINE bool IsDepthFormat(const SR_Format& aFormat)
	{
		switch (aFormat)
		{
		case SR_Format_D32_Float:
			return true;
		default:
			return false;
		}
	}
	
	SC_FORCEINLINE SR_Format GetNonDepthFormat(const SR_Format& aFormat)
	{
		switch (aFormat)
		{
		case SR_Format_D32_Float:
			return SR_Format_R32_Float;
		default:
			return SR_Format_Unknown;
		}
	}

	SC_FORCEINLINE SR_Format GetTypelessFormat(const SR_Format& aFormat)
	{
		switch (aFormat)
		{
		case SR_Format_D32_Float:
			return SR_Format_R32_Typeless;
		default:
			return SR_Format_Unknown;
		}
	}
}