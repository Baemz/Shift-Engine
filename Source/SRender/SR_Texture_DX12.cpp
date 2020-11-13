#include "SRender_Precompiled.h"
#include "SR_Texture_DX12.h"
#include "SR_DescriptorHeap_DX12.h"

#if ENABLE_DX12

namespace Shift
{

	SR_Texture_DX12::SR_Texture_DX12()
		: myHeap(nullptr)
	{
	}
	

	SR_Texture_DX12::SR_Texture_DX12(SR_DescriptorHeap_DX12* aHeap)
		: myHeap(aHeap)
	{
	}

	SR_Texture_DX12::~SR_Texture_DX12()
	{
		if (SR_GraphicsDevice::GetDevice() && myHeap)
			myHeap->Free(myDescriptorCPU);
	}

}

#endif