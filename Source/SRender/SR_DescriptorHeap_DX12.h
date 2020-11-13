#pragma once

#if ENABLE_DX12
#include "SR_Descriptor_DX12.h"

#include "SC_IndexAllocator.h"

namespace Shift
{
	enum SR_DescriptorHeapType
	{
		SR_DescriptorHeapType_CBV_SRV_UAV,
		SR_DescriptorHeapType_RTV,
		SR_DescriptorHeapType_DSV,
		SR_DescriptorHeapType_Sampler,
	};

	class SR_DescriptorHeap_DX12
	{
		friend class SR_DescriptorRing_DX12;
	public:
		SR_DescriptorHeap_DX12();
		~SR_DescriptorHeap_DX12();

		bool Init(const SR_DescriptorHeapType& aType, uint aDescriptorCount, bool aIsShaderVisible = false, const char* aName = nullptr);

		SR_Descriptor_DX12 Allocate();
		SR_DescriptorCPU_DX12 AllocateCPU();
		SR_DescriptorGPU_DX12 AllocateGPU();

		SR_DescriptorCPU_DX12 GetStartHandleCPU() const;
		SR_DescriptorGPU_DX12 GetStartHandleGPU() const;

		void Free(const SR_Descriptor_DX12& aHandle);
		void Free(const SR_DescriptorCPU_DX12& aHandle);
		void Free(const SR_DescriptorGPU_DX12& aHandle);

		SR_Descriptor_DX12 GetDescriptorAtIndex(uint aIndex);

		uint GetUsedCount() const;

		ID3D12DescriptorHeap* GetDescriptorHeap() const;

	private:
		uint64 GetOffset();
		void ReturnOffset(uint64 aOffset);

		SC_IndexAllocator myFreeIndices;
		uint64 myBeginCPU;
		uint64 myBeginGPU;
		uint64 myByteSize;
		uint myDescriptorSize;
		uint myTotalCount;
		ID3D12DescriptorHeap* myDX12DescriptorHeap;

		bool myIsShaderVisible : 1;
	};

	SC_FORCEINLINE SR_Descriptor_DX12 SR_DescriptorHeap_DX12::GetDescriptorAtIndex(uint aIndex)
	{
		uint64 offset = uint64(aIndex) * myDescriptorSize;
		assert((offset + myDescriptorSize <= myByteSize) && "Out of bounds descriptor requested");

		SR_Descriptor_DX12 handle;
		handle.myCPUHandle.ptr = myBeginCPU + offset;
		handle.myGPUHandle.ptr = myBeginGPU ? myBeginGPU + offset : 0;
		return handle;
	}

	inline uint SR_DescriptorHeap_DX12::GetUsedCount() const
	{
		return myFreeIndices.GetUsedCount();
	}
}

#endif