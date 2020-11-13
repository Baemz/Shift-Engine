#pragma once
#include "SR_Texture.h"

#if ENABLE_DX12
#include "SR_Descriptor_DX12.h"

namespace Shift
{
	class SR_DescriptorHeap_DX12;
	class SR_Texture_DX12 final : public SR_Texture
	{
		friend class SR_GraphicsDevice_DX12;
		friend class SR_GraphicsContext_DX12;
	public:
		SR_Texture_DX12();
		SR_Texture_DX12(SR_DescriptorHeap_DX12* aHeap);
		~SR_Texture_DX12();

		SR_DescriptorCPU_DX12 GetDescriptor() const { return myDescriptorCPU; }

	private:
		SR_DescriptorCPU_DX12 myDescriptorCPU; 
		SR_DescriptorHeap_DX12* myHeap;
	};

}

#endif
