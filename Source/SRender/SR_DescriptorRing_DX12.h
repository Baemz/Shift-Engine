#pragma once

#if ENABLE_DX12
#include "SR_RingBuffer.h"

namespace Shift
{
	class SR_DescriptorHeap_DX12;
	struct SR_Descriptor_DX12;
	class SR_DescriptorRing_DX12
	{
		friend class SR_GraphicsDevice_DX12;
	public:
		SR_DescriptorRing_DX12();

		void Init(SR_DescriptorHeap_DX12& aParentHeap, uint aMaxDescriptors);
		
		bool Allocate(SR_Descriptor_DX12& aDescriptorOut, uint aCount, SR_GraphicsContext* aCtx);

		uint GetDescriptorSize() const { return myDescriptorSize; }

	private:
		SR_RingBuffer2 myRingBuffer;
		uint64 myBeginCPU;
		uint64 myBeginGPU;
		uint myDescriptorSize;
	};

	SC_ALLOW_RELOCATE(SR_DescriptorRing_DX12);
}

#endif