#pragma once

#if ENABLE_DX12
#include SR_INCLUDE_FILE_D3D12

#include "SR_RenderEnums.h"

namespace Shift
{
	D3D12_COMPARISON_FUNC GetComparisonFunc_DX12(SR_ComparisonFunc aComparisonFunc);
	D3D12_CULL_MODE GetRasterizerFaceCull_DX12(SR_RasterizerFaceCull aFaceCull);
	D3D12_STENCIL_OP GetStencilOperator_DX12(SR_StencilOperator aStencilOperator);
	D3D12_BLEND GetBlendMode_DX12(SR_BlendMode aBlendMode);
	D3D12_BLEND_OP GetBlendFunc_DX12(SR_BlendFunc aBlendFunc);
	UINT8 GetColorWriteMask_DX12(uint8 aWriteMask);

	void GetResourceHeapDesc_DX12(D3D12_HEAP_PROPERTIES& aHeapDescOut, D3D12_RESOURCE_STATES& aInitialStateOut, const SR_AccessCPU aAccessCPU, const SR_MemoryAccess aMemoryAccess);

	uint GetFormatByteSize_DX12(DXGI_FORMAT aFormat);

	DXGI_FORMAT GetFormat_DX12(SR_Format aFormat);
}

#endif