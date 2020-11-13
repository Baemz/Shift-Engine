#include "SRender_Precompiled.h"
#include "SR_RingBuffer_DX12.h"

#if ENABLE_DX12

namespace Shift
{
	SR_RingBufferGPU_DX12::SR_RingBufferGPU_DX12(size_t aMaxSize, ID3D12Device * aGraphicsDevice, bool aAllowCPUAccess)
		: SR_RingBuffer(aMaxSize)
		, myCPUVirtualAddress(nullptr)
		, myGPUVirtualAddress(0)
		, myBuffer(nullptr)
	{
		D3D12_HEAP_PROPERTIES HeapProps;
		HeapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		HeapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		HeapProps.CreationNodeMask = 1;
		HeapProps.VisibleNodeMask = 1;

		D3D12_RESOURCE_DESC ResourceDesc;
		ResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		ResourceDesc.Alignment = 0;
		ResourceDesc.Height = 1;
		ResourceDesc.DepthOrArraySize = 1;
		ResourceDesc.MipLevels = 1;
		ResourceDesc.Format = DXGI_FORMAT_UNKNOWN;
		ResourceDesc.SampleDesc.Count = 1;
		ResourceDesc.SampleDesc.Quality = 0;
		ResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

		D3D12_RESOURCE_STATES DefaultUsage;
		if (aAllowCPUAccess)
		{
			HeapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
			ResourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
			DefaultUsage = D3D12_RESOURCE_STATE_GENERIC_READ;
		}
		else
		{
			HeapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
			ResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
			DefaultUsage = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
		}
		ResourceDesc.Width = aMaxSize;

		HRESULT hr = aGraphicsDevice->CreateCommittedResource(&HeapProps, D3D12_HEAP_FLAG_NONE, &ResourceDesc,
			DefaultUsage, nullptr, SR_IID_PPV_ARGS(&myBuffer));

		if (FAILED(hr))
		{
			SC_ERROR_LOG("Failed to create new upload ring buffer");
		}

		myBuffer->SetName(L"GPU Temp Memory Buffer");

		myGPUVirtualAddress = myBuffer->GetGPUVirtualAddress();

		if (aAllowCPUAccess)
		{
			myBuffer->Map(0, nullptr, &myCPUVirtualAddress);
		}
	}
	SR_RingBufferGPU_DX12::SR_RingBufferGPU_DX12(SR_RingBufferGPU_DX12&& rhs)
		: SR_RingBuffer(std::move(rhs))
		, myCPUVirtualAddress(rhs.myCPUVirtualAddress)
		, myGPUVirtualAddress(rhs.myGPUVirtualAddress)
		, myBuffer(std::move(rhs.myBuffer))
	{
		rhs.myCPUVirtualAddress = nullptr;
		rhs.myGPUVirtualAddress = 0;
		rhs.myBuffer->Release();
	}
	SR_RingBufferGPU_DX12 & SR_RingBufferGPU_DX12::operator=(SR_RingBufferGPU_DX12&& rhs)
	{
		Destroy();

		static_cast<SR_RingBuffer&>(*this) = std::move(rhs);
		myCPUVirtualAddress = rhs.myCPUVirtualAddress;
		myGPUVirtualAddress = rhs.myGPUVirtualAddress;
		myBuffer = std::move(rhs.myBuffer);
		rhs.myCPUVirtualAddress = 0;
		rhs.myGPUVirtualAddress = 0;

		return *this;
	}
	SR_RingBufferGPU_DX12::~SR_RingBufferGPU_DX12()
	{
		Destroy();
	}

	SR_TempAllocation SR_RingBufferGPU_DX12::Allocate(size_t aSizeInBytes)
	{
		uint64 Offset = SR_RingBuffer::Allocate(aSizeInBytes);
		if (Offset != SR_RingBuffer::InvalidOffset)
		{
			SR_TempAllocation tempMem(myBuffer, Offset, aSizeInBytes);
			tempMem.myGPUAddress = myGPUVirtualAddress + Offset;
			tempMem.myCPUAddress = myCPUVirtualAddress;

			if (tempMem.myCPUAddress)
				tempMem.myCPUAddress = reinterpret_cast<char*>(tempMem.myCPUAddress) + Offset;

			return tempMem;
		}
		else
			return SR_TempAllocation(nullptr, 0, 0);
	}
	void SR_RingBufferGPU_DX12::Destroy()
	{
		if (myBuffer)
		{
			if (myCPUVirtualAddress)
				myBuffer->Unmap(0, nullptr);

			myBuffer->Release();
			myBuffer = nullptr;
		}
		myCPUVirtualAddress = nullptr;
		myGPUVirtualAddress = 0;
		myUsedSize = 0;

	}
	SR_TempMemoryPool_DX12::SR_TempMemoryPool_DX12(bool aIsCPUAccessible, ID3D12Device * aDevice, size_t aInitialSize)
		: myGraphicsDeviceDX12(aDevice)
		, myIsCPUAccessible(aIsCPUAccessible)
	{
		myRingBuffers.Add(new SR_RingBufferGPU_DX12(aInitialSize, aDevice, myIsCPUAccessible));
	}
	SR_TempMemoryPool_DX12::~SR_TempMemoryPool_DX12()
	{
	}
	SR_TempAllocation SR_TempMemoryPool_DX12::Allocate(size_t aSizeInBytes, size_t aAlignment)
	{
		const size_t alignmentMask = aAlignment - 1;
		assert((alignmentMask & aAlignment) == 0);

		const size_t alignedSize = (aSizeInBytes + alignmentMask) & ~alignmentMask;
		SR_TempAllocation dynAlloc = myRingBuffers.GetLast()->Allocate(alignedSize);
		if (!dynAlloc.myBuffer)
		{
			SR_RingBuffer::OffsetType newMaxSize = myRingBuffers.GetLast()->GetMaxSize() * 2;
			while (newMaxSize < alignedSize)
			{
				newMaxSize *= 2;
			}
			myRingBuffers.Add(new SR_RingBufferGPU_DX12(newMaxSize, myGraphicsDeviceDX12, myIsCPUAccessible));
			dynAlloc = myRingBuffers.GetLast()->Allocate(alignedSize);
		}
		return dynAlloc;
	}
	void SR_TempMemoryPool_DX12::FinishFrame(uint64 aFenceValue, uint64 aLastCompletedFenceValue)
	{
		uint numBuffsToDelete = 0;
		for (uint index = 0; index < myRingBuffers.Count(); ++index)
		{
			auto& ringBuffer = myRingBuffers[index];
			ringBuffer->FinishCurrentFrame(aFenceValue);
			ringBuffer->ReleaseCompletedFrames(aLastCompletedFenceValue);
			if (numBuffsToDelete == index && index < myRingBuffers.Count() - 1 && ringBuffer->IsEmpty())
			{
				++numBuffsToDelete;
			}
		}

		if (numBuffsToDelete)
			myRingBuffers.RemoveFirstN(numBuffsToDelete);
	}
}
#endif