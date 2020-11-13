#include "SRender_Precompiled.h"
#include "SR_CommandAllocator_DX12.h"

#if ENABLE_DX12
#include "SR_CommandListPool_DX12.h"
#include "SR_GraphicsDevice_DX12.h"
#include "SR_QueueManager_DX12.h"

namespace Shift
{
	static volatile uint locNumAllocators = 0;

	SR_CommandAllocator_DX12::SR_CommandAllocator_DX12(D3D12_COMMAND_LIST_TYPE aType, SR_CommandListPool_DX12* aPool)
		: myType(aType)
		, myAllocator(nullptr)
		, myPool(aPool)
		, myCommandsSinceResetCount(0)
		, myCommandsPerResetLimit(20)
		, myCommandListsActive(0)
		, myCommandListsAlive(0)
		, myFence(0, SR_ContextType_Render)
	{
		if (aType == D3D12_COMMAND_LIST_TYPE_DIRECT)
			myCommandsPerResetLimit = 500;
		else if (aType == D3D12_COMMAND_LIST_TYPE_COMPUTE)
			myCommandsPerResetLimit = 100;

		SR_GraphicsDevice_DX12* device = SR_GraphicsDevice_DX12::GetDX12Device();
		ID3D12Device* nativeDevice = device->GetNativeDevice();

		HRESULT hr = S_OK;
		hr = nativeDevice->CreateCommandAllocator(myType, SR_IID_PPV_ARGS(&myAllocator));
		if (FAILED(hr))
		{
			SC_ERROR_LOG("Failed to create CommandAllocator");
			assert(false);
		}

		SC_Atomic_Increment(myPool->myAllocatorCount);

		uint num = SC_Atomic_Increment(locNumAllocators);

		std::wstring name(L"SR_CommandAllocator[");
		name += std::to_wstring(num);
		name += L']';

		myAllocator->SetName(name.c_str());

		myQueueManager = static_cast<SR_QueueManager_DX12*>(SR_QueueManager::Get());
	}
	SR_CommandAllocator_DX12::~SR_CommandAllocator_DX12()
	{
		if (myAllocator)
			myAllocator->Release();
	}
	bool SR_CommandAllocator_DX12::BeginCommandList()
	{
		if (IsFilled())
			return false;

		assert(SC_Atomic_CompareExchange(myCommandListsActive, 1, 0));
		SC_Atomic_Increment(myCommandListsAlive);

		return true;
	}

	void SR_CommandAllocator_DX12::Reset()
	{
		assert(myCommandsSinceResetCount > 0);
		assert(IsIdle());

		HRESULT hr = myAllocator->Reset();
		if (FAILED(hr))
		{
			SC_ERROR_LOG("Failed to reset CommandAllocator");
			assert(false);
		}

		myLastFrameOpened = SC_Timer::GetGlobalFrameIndex();
		myCommandsSinceResetCount = 0;
		myFence.myFenceValue = 0;
	}
	bool SR_CommandAllocator_DX12::IsFilled() const
	{
		return myCommandsPerResetLimit < myCommandsSinceResetCount;
	}
	bool SR_CommandAllocator_DX12::IsIdle()
	{
		if (myCommandListsActive || myCommandListsAlive)
			return false;

		SC_MutexLock lock(myMutex);
		if (myQueueManager->GetFenceManager().IsPending(myFence))
			return false;

		return true;
	}
}
#endif