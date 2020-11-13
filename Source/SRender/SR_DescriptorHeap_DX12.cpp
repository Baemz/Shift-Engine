#include "SRender_Precompiled.h"
#include "SR_DescriptorHeap_DX12.h"

#if ENABLE_DX12

#include "SR_GraphicsDevice_DX12.h"

namespace Shift
{
	static D3D12_DESCRIPTOR_HEAP_TYPE locGetHeapType(const SR_DescriptorHeapType& aType)
	{
		switch (aType)
		{
		case SR_DescriptorHeapType_CBV_SRV_UAV:
			return D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		case SR_DescriptorHeapType_DSV:
			return D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		case SR_DescriptorHeapType_RTV:
			return D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		}

		assert("Descriptor Heap Type not supported!");
		return D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	}

	SR_DescriptorHeap_DX12::SR_DescriptorHeap_DX12()
		: myDX12DescriptorHeap(nullptr)
		, myBeginCPU(0)
		, myBeginGPU(0)
		, myDescriptorSize(0)
		, myIsShaderVisible(false)
	{
	}

	SR_DescriptorHeap_DX12::~SR_DescriptorHeap_DX12()
	{
		if (myDX12DescriptorHeap)
			myDX12DescriptorHeap->Release();
	}

	bool SR_DescriptorHeap_DX12::Init(const SR_DescriptorHeapType& aType, uint aDescriptorCount, bool aIsShaderVisible, const char* aName)
	{
		SR_GraphicsDevice* device = SR_GraphicsDevice::GetDevice();
		SR_GraphicsDevice_DX12* deviceDX12 = static_cast<SR_GraphicsDevice_DX12*>(device);
		ID3D12Device* nativeDevice = deviceDX12->GetNativeDevice();

		D3D12_DESCRIPTOR_HEAP_TYPE type = locGetHeapType(aType);

		D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
		heapDesc.NodeMask = 0;
		heapDesc.NumDescriptors = aDescriptorCount;
		heapDesc.Type = type;
		heapDesc.Flags = (aIsShaderVisible) ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

		HRESULT result = nativeDevice->CreateDescriptorHeap(&heapDesc, SR_IID_PPV_ARGS(&myDX12DescriptorHeap));
		if (FAILED(result))
		{
			SC_ERROR_LOG("Failed to create descriptor heap");
			return false;
		}

		myTotalCount = aDescriptorCount;
		myBeginCPU = myDX12DescriptorHeap->GetCPUDescriptorHandleForHeapStart().ptr;
		myBeginGPU = myDX12DescriptorHeap->GetGPUDescriptorHandleForHeapStart().ptr;
		myDescriptorSize = nativeDevice->GetDescriptorHandleIncrementSize(type);
		myByteSize = myDescriptorSize * aDescriptorCount;
		myIsShaderVisible = aIsShaderVisible;

		myDX12DescriptorHeap->SetName(ToWString(aName).c_str());

		myFreeIndices.Init(aDescriptorCount);

		SC_LOG("%s created with size: %lluB", aName, myByteSize);
		return true;
	}

	SR_Descriptor_DX12 SR_DescriptorHeap_DX12::Allocate()
	{
		uint64 offset = GetOffset();

		SR_Descriptor_DX12 handle;
		handle.myCPUHandle.ptr = myBeginCPU + offset;
		handle.myGPUHandle.ptr = myBeginGPU ? myBeginGPU + offset : 0;

		return handle;
	}

	SR_DescriptorCPU_DX12 SR_DescriptorHeap_DX12::AllocateCPU()
	{
		assert(myBeginCPU);

		uint64 offset = GetOffset();
		SR_DescriptorCPU_DX12 handle;
		handle.ptr = myBeginCPU + offset;

		return handle;
	}

	SR_DescriptorGPU_DX12 SR_DescriptorHeap_DX12::AllocateGPU()
	{
		assert(myBeginGPU);

		uint64 offset = GetOffset();
		SR_DescriptorGPU_DX12 handle;
		handle.ptr = myBeginGPU + offset;

		return handle;
	}

	SR_DescriptorCPU_DX12 SR_DescriptorHeap_DX12::GetStartHandleCPU() const
	{
		SR_DescriptorCPU_DX12 handle;
		handle.ptr = myBeginCPU;
		return handle;
	}

	SR_DescriptorGPU_DX12 SR_DescriptorHeap_DX12::GetStartHandleGPU() const
	{
		SR_DescriptorGPU_DX12 handle;
		handle.ptr = myBeginGPU;
		return handle;
	}

	void SR_DescriptorHeap_DX12::Free(const SR_Descriptor_DX12& aHandle)
	{
		if (!aHandle.myCPUHandle.ptr && !aHandle.myGPUHandle.ptr)
			return;

		ReturnOffset(aHandle.myCPUHandle.ptr - myBeginCPU);
	}

	void SR_DescriptorHeap_DX12::Free(const SR_DescriptorCPU_DX12& aHandle)
	{
		if (!aHandle.ptr)
			return;

		ReturnOffset(aHandle.ptr - myBeginCPU);
	}

	void SR_DescriptorHeap_DX12::Free(const SR_DescriptorGPU_DX12& aHandle)
	{
		if (!aHandle.ptr)
			return;

		ReturnOffset(aHandle.ptr - myBeginGPU);
	}


	ID3D12DescriptorHeap* SR_DescriptorHeap_DX12::GetDescriptorHeap() const
	{
		return myDX12DescriptorHeap;
	}

	uint64 SR_DescriptorHeap_DX12::GetOffset()
	{
		uint offset = myFreeIndices.Allocate();
		return uint64(offset) * myDescriptorSize;
	}

	void SR_DescriptorHeap_DX12::ReturnOffset(uint64 aOffset)
	{
		SC_ASSERT((aOffset % myDescriptorSize) == 0 && aOffset < myByteSize);
		uint index = uint(aOffset / myDescriptorSize);
		myFreeIndices.Free(index);
	}
}
#endif