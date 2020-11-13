#pragma once
#include "SR_RenderTarget.h"

#if ENABLE_DX12

#include "SR_Descriptor_DX12.h"

namespace Shift
{
	class SR_DescriptorHeap_DX12;
	class SR_RenderTarget_DX12 final : public SR_RenderTarget
	{
		friend class SR_GraphicsDevice_DX12;
		friend class SR_GraphicsContext_DX12;
	public:
		SR_RenderTarget_DX12();
		SR_RenderTarget_DX12(SR_DescriptorHeap_DX12* aHeap);
		~SR_RenderTarget_DX12();

	private:
		SR_DescriptorCPU_DX12 myRenderTargetView;
		SR_DescriptorCPU_DX12 myDepthStencilView;
		SR_DescriptorHeap_DX12* myHeap;

	};
}

#endif