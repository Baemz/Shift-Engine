#pragma once

#if ENABLE_DX12
#include "SR_RenderDefines.h"
#include SR_INCLUDE_FILE_D3D12

namespace Shift
{
	// Wrapped D3D12 Descriptor
	template<class DescriptorHandle>
	struct SR_DescriptorWrapper_DX12 : DescriptorHandle
	{
		SR_DescriptorWrapper_DX12() { this->ptr = 0; }
		SR_DescriptorWrapper_DX12(nullptr_t) { this->ptr = 0; }

		operator bool() const { return (this->ptr != 0); }
		bool operator==(const SR_DescriptorWrapper_DX12& aOther) const { return this->ptr == aOther.ptr; }
		bool operator!=(const SR_DescriptorWrapper_DX12& aOther) const { return this->ptr != aOther.ptr; }
	};

	template<class DescriptorHandle>
	SR_DescriptorWrapper_DX12<DescriptorHandle> operator+(SR_DescriptorWrapper_DX12<DescriptorHandle> aHandle, uint64 aOffset)
	{
		aHandle.ptr += aOffset;
		return aHandle;
	}

	using SR_DescriptorCPU_DX12 = SR_DescriptorWrapper_DX12<D3D12_CPU_DESCRIPTOR_HANDLE>;
	using SR_DescriptorGPU_DX12 = SR_DescriptorWrapper_DX12<D3D12_GPU_DESCRIPTOR_HANDLE>;

	// Combined Descriptor
	struct SR_Descriptor_DX12
	{
		SR_DescriptorCPU_DX12 myCPUHandle;
		SR_DescriptorGPU_DX12 myGPUHandle;

		operator const SR_DescriptorCPU_DX12& () const { return myCPUHandle; }
		operator const SR_DescriptorGPU_DX12& () const { return myGPUHandle; }
	};

	struct SR_DescriptorRange_DX12
	{
		SR_Descriptor_DX12 myDescriptorStart;
		uint myCount;

		operator const SR_Descriptor_DX12& () const { return myDescriptorStart; }
	};

	SC_FORCEINLINE D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptor(const D3D12_CPU_DESCRIPTOR_HANDLE& aHandle) { return aHandle; }
	SC_FORCEINLINE D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptor(const SR_DescriptorCPU_DX12& aHandle) { return aHandle; }
	SC_FORCEINLINE D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptor(const SR_Descriptor_DX12& aHandle) { return aHandle.myCPUHandle; }

	SC_FORCEINLINE SR_Descriptor_DX12 operator+(SR_Descriptor_DX12 aHandle, uint64 aOffset)
	{
		if (aHandle.myCPUHandle.ptr)
			aHandle.myCPUHandle.ptr += aOffset;

		if (aHandle.myGPUHandle.ptr)
			aHandle.myGPUHandle.ptr += aOffset;

		return aHandle;
	}
}

#endif