#pragma once
#if ENABLE_DX12
#include "SR_VertexLayout.h"
#include "SR_Format.h"

namespace Shift
{
	inline DXGI_FORMAT SR_ConvertFormat_DX12(const SR_Format& value)
	{
		switch (value)
		{
		case SR_Format_RGBA32_Float:
			return DXGI_FORMAT_R32G32B32A32_FLOAT;
		case SR_Format_RGBA32_Uint:
			return DXGI_FORMAT_R32G32B32A32_UINT;
		case SR_Format_RGBA32_Sint:
			return DXGI_FORMAT_R32G32B32A32_SINT;
		case SR_Format_RGBA32_Typeless:
			return DXGI_FORMAT_R32G32B32A32_TYPELESS;

		case SR_Format_RGB32_Float:
			return DXGI_FORMAT_R32G32B32_FLOAT;
		case SR_Format_RGB32_Sint:
			return DXGI_FORMAT_R32G32B32_SINT;
		case SR_Format_RGB32_Typeless:
			return DXGI_FORMAT_R32G32B32_TYPELESS;
		case SR_Format_RGB32_Uint:
			return DXGI_FORMAT_R32G32B32_UINT;

		case SR_Format_RG32_Float:
			return DXGI_FORMAT_R32G32_FLOAT;
		case SR_Format_RG32_Sint:
			return DXGI_FORMAT_R32G32_SINT;
		case SR_Format_RG32_Uint:
			return DXGI_FORMAT_R32G32_UINT;
		case SR_Format_RG32_Typeless:
			return DXGI_FORMAT_R32G32_TYPELESS;

		case SR_Format_D32_Float:
			return DXGI_FORMAT_D32_FLOAT;
		case SR_Format_R32_Float:
			return DXGI_FORMAT_R32_FLOAT;
		case SR_Format_R32_Sint:
			return DXGI_FORMAT_R32_SINT;
		case SR_Format_R32_Uint:
			return DXGI_FORMAT_R32_UINT;
		case SR_Format_R32_Typeless:
			return DXGI_FORMAT_R32_TYPELESS;
			
		case SR_Format_RGBA16_Float:
			return DXGI_FORMAT_R16G16B16A16_FLOAT;
		case SR_Format_RGBA16_Uint:
			return DXGI_FORMAT_R16G16B16A16_UINT;
		case SR_Format_RGBA16_Sint:
			return DXGI_FORMAT_R16G16B16A16_SINT;
		case SR_Format_RGBA16_Typeless:
			return DXGI_FORMAT_R16G16B16A16_TYPELESS;
		case SR_Format_RGBA16_Snorm:
			return DXGI_FORMAT_R16G16B16A16_SNORM;
		case SR_Format_RGBA16_Unorm:
			return DXGI_FORMAT_R16G16B16A16_UNORM;

		case SR_Format_RG16_Float:
			return DXGI_FORMAT_R16G16_FLOAT;
		case SR_Format_RG16_Uint:
			return DXGI_FORMAT_R16G16_UINT;
		case SR_Format_RG16_Sint:
			return DXGI_FORMAT_R16G16_SINT;
		case SR_Format_RG16_Typeless:
			return DXGI_FORMAT_R16G16_TYPELESS;
		case SR_Format_RG16_Snorm:
			return DXGI_FORMAT_R16G16_SNORM;
		case SR_Format_RG16_Unorm:
			return DXGI_FORMAT_R16G16_UNORM;

		case SR_Format_RGBA8_Unorm:
			return DXGI_FORMAT_R8G8B8A8_UNORM;
		case SR_Format_RGBA8_Snorm:
			return DXGI_FORMAT_R8G8B8A8_SNORM;
		case SR_Format_RGBA8_Uint:
			return DXGI_FORMAT_R8G8B8A8_UINT;
		case SR_Format_RGBA8_Sint:
			return DXGI_FORMAT_R8G8B8A8_SINT;
		case SR_Format_RGBA8_Unorm_sRGB:
			return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		case SR_Format_RGBA8_Typeless:
			return DXGI_FORMAT_R8G8B8A8_TYPELESS;

		case SR_Format_RG11B10_Float:
			return DXGI_FORMAT_R11G11B10_FLOAT;
		case SR_Format_RGB10A2_Typeless:
			return DXGI_FORMAT_R10G10B10A2_TYPELESS;
		case SR_Format_RGB10A2_Uint:
			return DXGI_FORMAT_R10G10B10A2_UINT;
		case SR_Format_RGB10A2_Unorm:
			return DXGI_FORMAT_R10G10B10A2_UNORM;

		case SR_Format_DXT1:
			return DXGI_FORMAT_BC1_UNORM;
		case SR_Format_DXT1_sRGB:
			return DXGI_FORMAT_BC1_UNORM_SRGB;
		case SR_Format_DXT3:
			return DXGI_FORMAT_BC2_UNORM;
		case SR_Format_DXT3_sRGB:
			return DXGI_FORMAT_BC2_UNORM_SRGB;
		case SR_Format_DXT5:
			return DXGI_FORMAT_BC3_UNORM;
		case SR_Format_DXT5_sRGB:
			return DXGI_FORMAT_BC3_UNORM_SRGB;
		case SR_Format_BC5_Unorm:
			return DXGI_FORMAT_BC5_UNORM;
		case SR_Format_BC5_Snorm:
			return DXGI_FORMAT_BC5_SNORM;
		case SR_Format_BC7:
			return DXGI_FORMAT_BC7_UNORM;
		case SR_Format_BC7_sRGB:
			return DXGI_FORMAT_BC7_UNORM_SRGB;

		case SR_Format_Unknown:
		default:
			return DXGI_FORMAT_UNKNOWN;
		}
	}
	inline SR_Format SR_ConvertFormatFrom_DX12(const DXGI_FORMAT& value)
	{
		switch (value)
		{
		case DXGI_FORMAT_R32G32B32A32_FLOAT:
			return SR_Format_RGBA32_Float;
		case DXGI_FORMAT_R32G32B32A32_UINT:
			return SR_Format_RGBA32_Uint;
		case DXGI_FORMAT_R32G32B32A32_SINT:
			return SR_Format_RGBA32_Sint;
		case DXGI_FORMAT_R32G32B32A32_TYPELESS:
			return SR_Format_RGBA32_Typeless;

		case DXGI_FORMAT_R32G32B32_FLOAT:
			return SR_Format_RGB32_Float;
		case DXGI_FORMAT_R32G32B32_SINT:
			return SR_Format_RGB32_Sint;
		case DXGI_FORMAT_R32G32B32_TYPELESS:
			return SR_Format_RGB32_Typeless;
		case DXGI_FORMAT_R32G32B32_UINT:
			return SR_Format_RGB32_Uint;

		case DXGI_FORMAT_R32G32_FLOAT:
			return SR_Format_RG32_Float;
		case DXGI_FORMAT_R32G32_SINT:
			return SR_Format_RG32_Sint;
		case DXGI_FORMAT_R32G32_UINT:
			return SR_Format_RG32_Uint;
		case DXGI_FORMAT_R32G32_TYPELESS:
			return SR_Format_RG32_Typeless;

		case DXGI_FORMAT_D32_FLOAT:
			return SR_Format_D32_Float;
		case DXGI_FORMAT_R32_FLOAT:
			return SR_Format_R32_Float;
		case DXGI_FORMAT_R32_SINT:
			return SR_Format_R32_Sint;
		case DXGI_FORMAT_R32_UINT:
			return SR_Format_R32_Uint;
		case DXGI_FORMAT_R32_TYPELESS:
			return SR_Format_R32_Typeless;

		case DXGI_FORMAT_R16G16B16A16_FLOAT:
			return SR_Format_RGBA16_Float;
		case DXGI_FORMAT_R16G16B16A16_UINT:
			return SR_Format_RGBA16_Uint;
		case DXGI_FORMAT_R16G16B16A16_SINT:
			return SR_Format_RGBA16_Sint;
		case DXGI_FORMAT_R16G16B16A16_TYPELESS:
			return SR_Format_RGBA16_Typeless;
		case DXGI_FORMAT_R16G16B16A16_SNORM:
			return SR_Format_RGBA16_Snorm;
		case DXGI_FORMAT_R16G16B16A16_UNORM:
			return SR_Format_RGBA16_Unorm;

		case DXGI_FORMAT_R16G16_FLOAT:
			return SR_Format_RG16_Float;
		case DXGI_FORMAT_R16G16_UINT:
			return SR_Format_RG16_Uint;
		case DXGI_FORMAT_R16G16_SINT:
			return SR_Format_RG16_Sint;
		case DXGI_FORMAT_R16G16_TYPELESS:
			return SR_Format_RG16_Typeless;
		case DXGI_FORMAT_R16G16_SNORM:
			return SR_Format_RG16_Snorm;
		case DXGI_FORMAT_R16G16_UNORM:
			return SR_Format_RG16_Unorm;


		case DXGI_FORMAT_R8G8B8A8_UNORM:
			return SR_Format_RGBA8_Unorm;
		case DXGI_FORMAT_R8G8B8A8_SNORM:
			return SR_Format_RGBA8_Snorm;
		case DXGI_FORMAT_R8G8B8A8_UINT:
			return SR_Format_RGBA8_Uint;
		case DXGI_FORMAT_R8G8B8A8_SINT:
			return SR_Format_RGBA8_Sint;
		case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
			return SR_Format_RGBA8_Unorm_sRGB;
		case DXGI_FORMAT_R8G8B8A8_TYPELESS:
			return SR_Format_RGBA8_Typeless;


		case DXGI_FORMAT_R11G11B10_FLOAT:
			return SR_Format_RG11B10_Float;
		case DXGI_FORMAT_R10G10B10A2_TYPELESS:
			return SR_Format_RGB10A2_Typeless;
		case DXGI_FORMAT_R10G10B10A2_UINT:
			return SR_Format_RGB10A2_Uint;
		case DXGI_FORMAT_R10G10B10A2_UNORM:
			return SR_Format_RGB10A2_Unorm;


		case DXGI_FORMAT_BC1_UNORM:
			return SR_Format_DXT1;
		case DXGI_FORMAT_BC1_UNORM_SRGB:
			return SR_Format_DXT1_sRGB;
		case DXGI_FORMAT_BC2_UNORM:
			return SR_Format_DXT3;
		case DXGI_FORMAT_BC2_UNORM_SRGB:
			return SR_Format_DXT3_sRGB;
		case DXGI_FORMAT_BC3_UNORM:
			return SR_Format_DXT5;
		case DXGI_FORMAT_BC3_UNORM_SRGB:
			return SR_Format_DXT5_sRGB;
		case DXGI_FORMAT_BC5_UNORM:
			return SR_Format_BC5_Unorm;
		case DXGI_FORMAT_BC5_SNORM:
			return SR_Format_BC5_Snorm;
		case DXGI_FORMAT_BC7_UNORM:
			return SR_Format_BC7;
		case DXGI_FORMAT_BC7_UNORM_SRGB:
			return SR_Format_BC7_sRGB;

		case DXGI_FORMAT_UNKNOWN:
		default:
			return SR_Format_Unknown;
		}
	}

	inline DXGI_FORMAT GetDepthFormatFromTypelessDX(const SR_Format& value)
	{
		switch (value)
		{
		case SR_Format_R32_Typeless:
			return DXGI_FORMAT_D32_FLOAT;
		}
		return SR_ConvertFormat_DX12(value);
	}

	inline D3D12_INPUT_CLASSIFICATION _ConvertInputClassificationDX12(const SR_VertexLayoutElement::EInputClassification& value)
	{
		switch (value)
		{
		case SR_VertexLayoutElement::EInputClassification::INPUT_PER_VERTEX_DATA:
			return D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
		case SR_VertexLayoutElement::EInputClassification::INPUT_PER_INSTANCE_DATA:
			return D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA;
		default:
			break;
		}
		return D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
	}
} // END NAMESPACE SHIFT
#endif