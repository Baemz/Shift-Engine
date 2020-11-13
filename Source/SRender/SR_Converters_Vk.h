#pragma once

#if ENABLE_VULKAN
#include "SR_Format.h"

namespace Shift
{
	inline VkFormat SR_ConvertFormat_Vk(const SR_Format& aValue)
	{
		switch (aValue)
		{
		case SR_Format_RGBA32_Float:
			return VK_FORMAT_R32G32B32A32_SFLOAT;
		case SR_Format_RGBA32_Uint:
			return VK_FORMAT_R32G32B32A32_UINT;
		case SR_Format_RGBA32_Sint:
			return VK_FORMAT_R32G32B32A32_SINT;

		case SR_Format_RGB32_Float:
			return VK_FORMAT_R32G32B32_SFLOAT;
		case SR_Format_RGB32_Sint:
			return VK_FORMAT_R32G32B32_SINT;
		case SR_Format_RGB32_Uint:
			return VK_FORMAT_R32G32B32_UINT;

		case SR_Format_RG32_Float:
			return VK_FORMAT_R32G32_SFLOAT;
		case SR_Format_RG32_Sint:
			return VK_FORMAT_R32G32_SINT;
		case SR_Format_RG32_Uint:
			return VK_FORMAT_R32G32_UINT;

		case SR_Format_D32_Float:
			return VK_FORMAT_D32_SFLOAT;
		case SR_Format_R32_Float:
			return VK_FORMAT_R32_SFLOAT;
		case SR_Format_R32_Sint:
			return VK_FORMAT_R32_SINT;
		case SR_Format_R32_Uint:
			return VK_FORMAT_R32_UINT;

		case SR_Format_RGBA16_Float:
			return VK_FORMAT_R16G16B16A16_SFLOAT;
		case SR_Format_RGBA16_Uint:
			return VK_FORMAT_R16G16B16A16_UINT;
		case SR_Format_RGBA16_Sint:
			return VK_FORMAT_R16G16B16A16_SINT;
		case SR_Format_RGBA16_Snorm:
			return VK_FORMAT_R16G16B16A16_SNORM;
		case SR_Format_RGBA16_Unorm:
			return VK_FORMAT_R16G16B16A16_UNORM;

		case SR_Format_RG16_Float:
			return VK_FORMAT_R16G16_SFLOAT;
		case SR_Format_RG16_Uint:
			return VK_FORMAT_R16G16_UINT;
		case SR_Format_RG16_Sint:
			return VK_FORMAT_R16G16_SINT;
		case SR_Format_RG16_Snorm:
			return VK_FORMAT_R16G16_SNORM;
		case SR_Format_RG16_Unorm:
			return VK_FORMAT_R16G16_UNORM;

		case SR_Format_RGBA8_Unorm:
			return VK_FORMAT_R8G8B8A8_UNORM;
		case SR_Format_BGRA8_Unorm:
			return VK_FORMAT_B8G8R8A8_UNORM;
		case SR_Format_RGBA8_Snorm:
			return VK_FORMAT_R8G8B8A8_SNORM;
		case SR_Format_RGBA8_Uint:
			return VK_FORMAT_R8G8B8A8_UINT;
		case SR_Format_RGBA8_Sint:
			return VK_FORMAT_R8G8B8A8_SINT;
		case SR_Format_RGBA8_Unorm_sRGB:
			return VK_FORMAT_R8G8B8A8_SRGB;
		case SR_Format_BGRA8_Unorm_sRGB:
			return VK_FORMAT_B8G8R8A8_SRGB;

		case SR_Format_RG11B10_Float:
			return VK_FORMAT_B10G11R11_UFLOAT_PACK32;
		case SR_Format_RGB10A2_Uint:
			return VK_FORMAT_A2R10G10B10_UINT_PACK32;
		case SR_Format_RGB10A2_Unorm:
			return VK_FORMAT_A2R10G10B10_UNORM_PACK32;

		case SR_Format_DXT1:
			return VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
		case SR_Format_DXT1_sRGB:
			return VK_FORMAT_BC1_RGBA_SRGB_BLOCK;
		case SR_Format_DXT3:
			return VK_FORMAT_BC2_UNORM_BLOCK;
		case SR_Format_DXT3_sRGB:
			return VK_FORMAT_BC2_SRGB_BLOCK;
		case SR_Format_DXT5:
			return VK_FORMAT_BC3_UNORM_BLOCK;
		case SR_Format_DXT5_sRGB:
			return VK_FORMAT_BC3_SRGB_BLOCK;
		case SR_Format_BC5_Unorm:
			return VK_FORMAT_BC5_UNORM_BLOCK;
		case SR_Format_BC5_Snorm:
			return VK_FORMAT_BC5_SNORM_BLOCK;
		case SR_Format_BC7:
			return VK_FORMAT_BC7_UNORM_BLOCK;
		case SR_Format_BC7_sRGB:
			return VK_FORMAT_BC7_SRGB_BLOCK;

		case SR_Format_Unknown:
		default:
			return VK_FORMAT_UNDEFINED;
		}
	}
	inline SR_Format SR_ConvertFormatFrom_Vk(const VkFormat& aValue)
	{
		switch (aValue)
		{
		case VK_FORMAT_R32G32B32A32_SFLOAT:
			return SR_Format_RGBA32_Float;
		case VK_FORMAT_R32G32B32A32_UINT:
			return SR_Format_RGBA32_Uint;
		case VK_FORMAT_R32G32B32A32_SINT:
			return SR_Format_RGBA32_Sint;

		case VK_FORMAT_R32G32B32_SFLOAT:
			return SR_Format_RGB32_Float;
		case VK_FORMAT_R32G32B32_SINT:
			return SR_Format_RGB32_Sint;
		case VK_FORMAT_R32G32B32_UINT:
			return SR_Format_RGB32_Uint;

		case VK_FORMAT_R32G32_SFLOAT:
			return SR_Format_RG32_Float;
		case VK_FORMAT_R32G32_SINT:
			return SR_Format_RG32_Sint;
		case VK_FORMAT_R32G32_UINT:
			return SR_Format_RG32_Uint;

		case VK_FORMAT_D32_SFLOAT:
			return SR_Format_D32_Float;
		case VK_FORMAT_R32_SFLOAT:
			return SR_Format_R32_Float;
		case VK_FORMAT_R32_SINT:
			return SR_Format_R32_Sint;
		case VK_FORMAT_R32_UINT:
			return SR_Format_R32_Uint;

		case VK_FORMAT_R16G16B16A16_SFLOAT:
			return SR_Format_RGBA16_Float;
		case VK_FORMAT_R16G16B16A16_UINT:
			return SR_Format_RGBA16_Uint;
		case VK_FORMAT_R16G16B16A16_SINT:
			return SR_Format_RGBA16_Sint;
		case VK_FORMAT_R16G16B16A16_SNORM:
			return SR_Format_RGBA16_Snorm;
		case VK_FORMAT_R16G16B16A16_UNORM:
			return SR_Format_RGBA16_Unorm;

		case VK_FORMAT_R16G16_SFLOAT:
			return SR_Format_RG16_Float;
		case VK_FORMAT_R16G16_UINT:
			return SR_Format_RG16_Uint;
		case VK_FORMAT_R16G16_SINT:
			return SR_Format_RG16_Sint;
		case VK_FORMAT_R16G16_SNORM:
			return SR_Format_RG16_Snorm;
		case VK_FORMAT_R16G16_UNORM:
			return SR_Format_RG16_Unorm;


		case VK_FORMAT_R8G8B8A8_UNORM:
			return SR_Format_RGBA8_Unorm;
		case VK_FORMAT_B8G8R8A8_UNORM:
			return SR_Format_BGRA8_Unorm;
		case VK_FORMAT_R8G8B8A8_SNORM:
			return SR_Format_RGBA8_Snorm;
		case VK_FORMAT_R8G8B8A8_UINT:
			return SR_Format_RGBA8_Uint;
		case VK_FORMAT_R8G8B8A8_SINT:
			return SR_Format_RGBA8_Sint;
		case VK_FORMAT_R8G8B8A8_SRGB:
			return SR_Format_RGBA8_Unorm_sRGB;
		case VK_FORMAT_B8G8R8A8_SRGB:
			return SR_Format_BGRA8_Unorm_sRGB;

		case VK_FORMAT_B10G11R11_UFLOAT_PACK32:
			return SR_Format_RG11B10_Float;
		case VK_FORMAT_A2R10G10B10_UINT_PACK32:
			return SR_Format_RGB10A2_Uint;
		case VK_FORMAT_A2R10G10B10_UNORM_PACK32:
			return SR_Format_RGB10A2_Unorm;

		case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:
			return SR_Format_DXT1;
		case VK_FORMAT_BC1_RGBA_SRGB_BLOCK:
			return SR_Format_DXT1_sRGB;
		case VK_FORMAT_BC2_UNORM_BLOCK:
			return SR_Format_DXT3;
		case VK_FORMAT_BC2_SRGB_BLOCK:
			return SR_Format_DXT3_sRGB;
		case VK_FORMAT_BC3_UNORM_BLOCK:
			return SR_Format_DXT5;
		case VK_FORMAT_BC3_SRGB_BLOCK:
			return SR_Format_DXT5_sRGB;
		case VK_FORMAT_BC5_UNORM_BLOCK:
			return SR_Format_BC5_Unorm;
		case VK_FORMAT_BC5_SNORM_BLOCK:
			return SR_Format_BC5_Snorm;
		case VK_FORMAT_BC7_UNORM_BLOCK:
			return SR_Format_BC7;
		case VK_FORMAT_BC7_SRGB_BLOCK:
			return SR_Format_BC7_sRGB;

		case VK_FORMAT_UNDEFINED:
		default:
			return SR_Format_Unknown;
		}
	}
}
#endif