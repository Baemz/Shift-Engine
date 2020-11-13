#include "SRender_Precompiled.h"
#include "SR_RenderEnums_DX12.h"

#if ENABLE_DX12

namespace Shift
{
	D3D12_COMPARISON_FUNC GetComparisonFunc_DX12(SR_ComparisonFunc aComparisonFunc)
	{
		switch (aComparisonFunc)
		{
		case SR_ComparisonFunc_Never:
			return D3D12_COMPARISON_FUNC_NEVER;
		case SR_ComparisonFunc_Less:
			return D3D12_COMPARISON_FUNC_LESS;
		case SR_ComparisonFunc_Equal:
			return D3D12_COMPARISON_FUNC_EQUAL;
		case SR_ComparisonFunc_LessEqual:
			return D3D12_COMPARISON_FUNC_LESS_EQUAL;
		case SR_ComparisonFunc_Greater:
			return D3D12_COMPARISON_FUNC_GREATER;
		case SR_ComparisonFunc_NotEqual:
			return D3D12_COMPARISON_FUNC_NOT_EQUAL;
		case SR_ComparisonFunc_GreaterEqual:
			return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
		case SR_ComparisonFunc_Always:
			return D3D12_COMPARISON_FUNC_ALWAYS;
		default: 
			break;
		}
		assert(false && "Unknown comparison function DX12");
		return D3D12_COMPARISON_FUNC_NEVER;
	}

	D3D12_CULL_MODE GetRasterizerFaceCull_DX12(SR_RasterizerFaceCull aFaceCull)
	{
		switch (aFaceCull)
		{
		case SR_RasterizerFaceCull_None:
			return D3D12_CULL_MODE_NONE;
		case SR_RasterizerFaceCull_Front:
			return D3D12_CULL_MODE_FRONT;
		case SR_RasterizerFaceCull_Back:
			return D3D12_CULL_MODE_BACK;
		default: 
			break;
		}
		assert(false && "Unknown rasterizer face cull mode DX12");
		return D3D12_CULL_MODE_NONE;
	}
	D3D12_STENCIL_OP GetStencilOperator_DX12(SR_StencilOperator aStencilOperator)
	{
		switch (aStencilOperator)
		{
		case SR_StencilOperator_Keep:
			return D3D12_STENCIL_OP_KEEP;
		case SR_StencilOperator_Zero:
			return D3D12_STENCIL_OP_ZERO;
		case SR_StencilOperator_Replace:
			return D3D12_STENCIL_OP_REPLACE;
		case SR_StencilOperator_Increment_Saturate:
			return D3D12_STENCIL_OP_INCR_SAT;
		case SR_StencilOperator_Decrement_Saturate:
			return D3D12_STENCIL_OP_DECR_SAT;
		case SR_StencilOperator_Invert:
			return D3D12_STENCIL_OP_INVERT;
		case SR_StencilOperator_Increment:
			return D3D12_STENCIL_OP_INCR;
		case SR_StencilOperator_Decrement:
			return D3D12_STENCIL_OP_DECR;
		default: 
			break;
		}
		assert(false && "Unknown stencil operator DX12");
		return D3D12_STENCIL_OP_KEEP;
	}

	D3D12_BLEND GetBlendMode_DX12(SR_BlendMode aBlendMode)
	{
		switch (aBlendMode)
		{
		case SR_BlendMode_Zero:
			return D3D12_BLEND_ZERO;
		case SR_BlendMode_One:
			return D3D12_BLEND_ONE;
		case SR_BlendMode_Src_Color:
			return D3D12_BLEND_SRC_COLOR;
		case SR_BlendMode_One_Minus_Src_Color:
			return D3D12_BLEND_INV_SRC_COLOR;
		case SR_BlendMode_Src_Alpha:
			return D3D12_BLEND_SRC_ALPHA;
		case SR_BlendMode_One_Minus_Src_Alpha:
			return D3D12_BLEND_INV_SRC_ALPHA;
		case SR_BlendMode_Dst_Alpha:
			return D3D12_BLEND_DEST_ALPHA;
		case SR_BlendMode_One_Minus_Dst_Alpha:
			return D3D12_BLEND_INV_SRC_ALPHA;
		case SR_BlendMode_Dst_Color:
			return D3D12_BLEND_DEST_COLOR;
		case SR_BlendMode_One_Minus_Dst_Color:
			return D3D12_BLEND_INV_DEST_COLOR;
		case SR_BlendMode_Src_Alpha_Saturate:
			return D3D12_BLEND_SRC_ALPHA_SAT;
		case SR_BlendMode_Blend_Factor:
			return D3D12_BLEND_BLEND_FACTOR;
		case SR_BlendMode_One_Minus_Blend_Factor:
			return D3D12_BLEND_INV_BLEND_FACTOR;
		case SR_BlendMode_Src1_Color:
			return D3D12_BLEND_SRC1_COLOR;
		case SR_BlendMode_One_Minus_Src1_Color:
			return D3D12_BLEND_INV_SRC1_COLOR;
		case SR_BlendMode_Src1_Alpha:
			return D3D12_BLEND_SRC1_ALPHA;
		case SR_BlendMode_One_Minus_Src1_Alpha:
			return D3D12_BLEND_INV_SRC1_ALPHA;
		default:
			break;
		}
		assert(false && "Unknown blend mode DX12");
		return D3D12_BLEND_ZERO;
	}

	D3D12_BLEND_OP GetBlendFunc_DX12(SR_BlendFunc aBlendFunc)
	{
		switch (aBlendFunc)
		{
		case SR_BlendFunc_Add:
			return D3D12_BLEND_OP_ADD;
		case SR_BlendFunc_Subtract:
			return D3D12_BLEND_OP_SUBTRACT;
		case SR_BlendFunc_Reverse_Subtract:
			return D3D12_BLEND_OP_REV_SUBTRACT;
		case SR_BlendFunc_Min:
			return D3D12_BLEND_OP_MIN;
		case SR_BlendFunc_Max:
			return D3D12_BLEND_OP_MAX;
		default: 
			break;
		}
		assert(false && "Unknown blend func DX12");
		return D3D12_BLEND_OP_ADD;
	}

	UINT8 GetColorWriteMask_DX12(uint8 aWriteMask)
	{
		UINT8 mask = 0;
		if (aWriteMask & SR_ColorWriteMask_Red)
			mask |= D3D12_COLOR_WRITE_ENABLE_RED;
		if (aWriteMask & SR_ColorWriteMask_Green)
			mask |= D3D12_COLOR_WRITE_ENABLE_GREEN;
		if (aWriteMask & SR_ColorWriteMask_Blue)
			mask |= D3D12_COLOR_WRITE_ENABLE_BLUE;
		if (aWriteMask & SR_ColorWriteMask_Alpha)
			mask |= D3D12_COLOR_WRITE_ENABLE_ALPHA;
		return mask;
	}

	void GetResourceHeapDesc_DX12(D3D12_HEAP_PROPERTIES& aHeapDescOut, D3D12_RESOURCE_STATES& aInitialStateOut, const SR_AccessCPU aAccessCPU, const SR_MemoryAccess aMemoryAccess)
	{
		SC_UNUSED(aMemoryAccess);

		switch (aAccessCPU)
		{
		case SR_AccessCPU_None:
			aHeapDescOut.Type = D3D12_HEAP_TYPE_DEFAULT;
			return;

		case SR_AccessCPU_Map_Read:
			aHeapDescOut.Type = D3D12_HEAP_TYPE_READBACK;
			aInitialStateOut = D3D12_RESOURCE_STATE_COPY_DEST;
			return;

		case SR_AccessCPU_Map_Write:
			aHeapDescOut.Type = D3D12_HEAP_TYPE_UPLOAD;
			aInitialStateOut = D3D12_RESOURCE_STATE_GENERIC_READ;
			return;

		case SR_AccessCPU_Map_Read_Write:
			aHeapDescOut.Type = D3D12_HEAP_TYPE_CUSTOM;
			aHeapDescOut.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
			aHeapDescOut.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;
			return;
		}

		SC_ASSERT(false, "Unknown resource cpu access.");
	}

	uint GetFormatByteSize_DX12(DXGI_FORMAT aFormat)
	{
		switch (aFormat)
		{
			case DXGI_FORMAT_R32G32B32A32_TYPELESS:
			case DXGI_FORMAT_R32G32B32A32_FLOAT:
			case DXGI_FORMAT_R32G32B32A32_UINT:
			case DXGI_FORMAT_R32G32B32A32_SINT:
			case DXGI_FORMAT_R32G32B32_TYPELESS:
			case DXGI_FORMAT_R32G32B32_FLOAT:
			case DXGI_FORMAT_R32G32B32_UINT:
			case DXGI_FORMAT_R32G32B32_SINT:
				return 16;

			case DXGI_FORMAT_R16G16B16A16_TYPELESS:
			case DXGI_FORMAT_R16G16B16A16_FLOAT:
			case DXGI_FORMAT_R16G16B16A16_UNORM:
			case DXGI_FORMAT_R16G16B16A16_UINT:
			case DXGI_FORMAT_R16G16B16A16_SNORM:
			case DXGI_FORMAT_R16G16B16A16_SINT:
			case DXGI_FORMAT_R32G32_TYPELESS:
			case DXGI_FORMAT_R32G32_FLOAT:
			case DXGI_FORMAT_R32G32_UINT:
			case DXGI_FORMAT_R32G32_SINT:
				return 8;

			case DXGI_FORMAT_R10G10B10A2_TYPELESS:
			case DXGI_FORMAT_R10G10B10A2_UNORM:
			case DXGI_FORMAT_R10G10B10A2_UINT:
			case DXGI_FORMAT_R11G11B10_FLOAT:
			case DXGI_FORMAT_R8G8B8A8_TYPELESS:
			case DXGI_FORMAT_R8G8B8A8_UNORM:
			case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
			case DXGI_FORMAT_R8G8B8A8_UINT:
			case DXGI_FORMAT_R8G8B8A8_SNORM:
			case DXGI_FORMAT_R8G8B8A8_SINT:
			case DXGI_FORMAT_R16G16_TYPELESS:
			case DXGI_FORMAT_R16G16_FLOAT:
			case DXGI_FORMAT_R16G16_UNORM:
			case DXGI_FORMAT_R16G16_UINT:
			case DXGI_FORMAT_R16G16_SNORM:
			case DXGI_FORMAT_R16G16_SINT:
			case DXGI_FORMAT_R32_TYPELESS:
			case DXGI_FORMAT_D32_FLOAT:
			case DXGI_FORMAT_R32_FLOAT:
			case DXGI_FORMAT_R32_UINT:
			case DXGI_FORMAT_R32_SINT:
				return 4;

			case DXGI_FORMAT_R8G8_TYPELESS:
			case DXGI_FORMAT_R8G8_UNORM:
			case DXGI_FORMAT_R8G8_UINT:
			case DXGI_FORMAT_R8G8_SNORM:
			case DXGI_FORMAT_R8G8_SINT:
			case DXGI_FORMAT_R16_TYPELESS:
			case DXGI_FORMAT_R16_FLOAT:
			case DXGI_FORMAT_D16_UNORM:
			case DXGI_FORMAT_R16_UNORM:
			case DXGI_FORMAT_R16_UINT:
			case DXGI_FORMAT_R16_SNORM:
			case DXGI_FORMAT_R16_SINT:
				return 2;

			case DXGI_FORMAT_R8_TYPELESS:
			case DXGI_FORMAT_R8_UNORM:
			case DXGI_FORMAT_R8_UINT:
			case DXGI_FORMAT_R8_SNORM:
			case DXGI_FORMAT_R8_SINT:
			return 1;
		}
		return 0;
	}

	DXGI_FORMAT GetFormat_DX12(SR_Format aFormat)
	{
		switch (aFormat)
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
}

#endif