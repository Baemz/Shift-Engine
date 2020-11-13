#include "SRender_Precompiled.h"
#include "SR_DescriptorRing_DX12.h"

#if ENABLE_DX12
#include "SR_DescriptorHeap_DX12.h"

namespace Shift
{

	SR_DescriptorRing_DX12::SR_DescriptorRing_DX12()
		: myBeginCPU(0)
		, myBeginGPU(0)
		, myDescriptorSize(0)
	{

	}

	void SR_DescriptorRing_DX12::Init(SR_DescriptorHeap_DX12& aParentHeap, uint aMaxDescriptors)
	{
		SC_ASSERT(!(aMaxDescriptors & 63));

		myRingBuffer.mySize = aMaxDescriptors;
		myRingBuffer.myMaxAllocSize = aMaxDescriptors / 4;
		myRingBuffer.myUsedOffset = aMaxDescriptors;

		uint firstIdx = aParentHeap.myFreeIndices.Reserve(aMaxDescriptors);
		myDescriptorSize = aParentHeap.myDescriptorSize;
		myBeginCPU = aParentHeap.myBeginCPU ? aParentHeap.myBeginCPU + firstIdx * myDescriptorSize : 0;
		myBeginGPU = aParentHeap.myBeginGPU ? aParentHeap.myBeginGPU + firstIdx * myDescriptorSize : 0;
	}

	bool SR_DescriptorRing_DX12::Allocate(SR_Descriptor_DX12& aDescriptorOut, uint aCount, SR_GraphicsContext* aCtx)
	{
		uint offset;
		if (!myRingBuffer.Allocate(offset, aCount, aCtx, SR_Fence()))
			return false;

		aDescriptorOut.myCPUHandle.ptr = myBeginCPU ? myBeginCPU + offset * myDescriptorSize : 0;
		aDescriptorOut.myGPUHandle.ptr = myBeginGPU ? myBeginGPU + offset * myDescriptorSize : 0;
		return true;
	}

}

#endif