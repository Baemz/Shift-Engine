#pragma once
#include "SR_RingBuffer.h"

#if ENABLE_DX12

namespace Shift
{
	class SR_RingBufferGPU_DX12 : public SR_RingBuffer
	{
	public:
		SR_RingBufferGPU_DX12(size_t aMaxSize, ID3D12Device *aGraphicsDevice, bool aAllowCPUAccess);

		SR_RingBufferGPU_DX12(SR_RingBufferGPU_DX12&& rhs);
		SR_RingBufferGPU_DX12& operator=(SR_RingBufferGPU_DX12&& rhs);
		~SR_RingBufferGPU_DX12();

		SR_RingBufferGPU_DX12(const SR_RingBufferGPU_DX12&) = delete;
		SR_RingBufferGPU_DX12& operator=(const SR_RingBufferGPU_DX12&) = delete;

		SR_TempAllocation Allocate(size_t aSizeInBytes);

	private:
		void Destroy();

		void* myCPUVirtualAddress;
		D3D12_GPU_VIRTUAL_ADDRESS myGPUVirtualAddress;
		ID3D12Resource* myBuffer;
	};

	class SR_TempMemoryPool_DX12
	{
	public:
		SR_TempMemoryPool_DX12(bool aIsCPUAccessible, ID3D12Device* aDevice, size_t aInitialSize);

		SR_TempMemoryPool_DX12(const SR_TempMemoryPool_DX12&) = delete;
		SR_TempMemoryPool_DX12(SR_TempMemoryPool_DX12&&) = delete;
		SR_TempMemoryPool_DX12& operator=(const SR_TempMemoryPool_DX12&) = delete;
		SR_TempMemoryPool_DX12& operator=(SR_TempMemoryPool_DX12&&) = delete;
		~SR_TempMemoryPool_DX12();

		SR_TempAllocation Allocate(size_t aSizeInBytes, size_t aAlignment = DEFAULT_ALIGN);

		void FinishFrame(uint64 aFenceValue, uint64 aLastCompletedFenceValue);

	private:
		SC_GrowingArray<SC_Ref<SR_RingBufferGPU_DX12>> myRingBuffers;
		ID3D12Device* myGraphicsDeviceDX12;
		const bool myIsCPUAccessible;
	};
}
#endif