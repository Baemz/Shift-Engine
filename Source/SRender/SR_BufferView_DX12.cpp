#include "SRender_Precompiled.h"
#include "SR_BufferView_DX12.h"

#if ENABLE_DX12
#include "SR_DescriptorHeap_DX12.h"

namespace Shift
{
	SR_BufferView_DX12::SR_BufferView_DX12()
		: myHeap(nullptr)
	{
	}

	SR_BufferView_DX12::SR_BufferView_DX12(SR_DescriptorHeap_DX12* aHeap)
		: myHeap(aHeap)
	{
	}

	SR_BufferView_DX12::~SR_BufferView_DX12()
	{
		if (SR_GraphicsDevice::GetDevice() && myHeap)
			myHeap->Free(myDescriptorCPU);
	}
}
#endif