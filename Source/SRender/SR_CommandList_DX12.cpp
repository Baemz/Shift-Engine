#include "SRender_Precompiled.h"
#include "SR_CommandList_DX12.h"

#if ENABLE_DX12
#include "SR_GraphicsDevice_DX12.h"
#include "SR_CommandAllocator_DX12.h"
#include "SR_CommandListPool_DX12.h"
#include "SR_QueueManager_DX12.h"

namespace Shift
{
	static volatile uint locNumCommandLists = 0;

	SR_CommandList_DX12::SR_CommandList_DX12(D3D12_COMMAND_LIST_TYPE aType, SR_CommandAllocator_DX12* aCmdAllocator, SR_CommandListPool_DX12* aPool, const SR_ContextType& aContextType)
		: SR_CommandList(Idle|Recording, aContextType)
		, myType(aType)
		, myCurrentContextType(aContextType)
		, myCommandList(nullptr)
#if SR_ENABLE_RAYTRACING
		, myCommandList4(nullptr)
#endif
		, myAllocator(aCmdAllocator)
		, myPool(aPool)
		, myNumCommands(0)
		, myPoolId(SC_UINT32_MAX)
	{
		SR_GraphicsDevice_DX12* device = SR_GraphicsDevice_DX12::GetDX12Device();
		ID3D12Device* nativeDevice = device->GetNativeDevice();

		HRESULT hr = nativeDevice->CreateCommandList(1, myType, myAllocator->myAllocator, nullptr, SR_IID_PPV_ARGS(&myCommandList));
		if (FAILED(hr))
		{
			SC_ERROR_LOG("Failed to create CommandList");
			assert(false);
		}

#if SR_ENABLE_RAYTRACING
		if (myCommandList)
			myCommandList->QueryInterface(SR_IID_PPV_ARGS(&myCommandList4));
#endif

		uint num = SC_Atomic_Increment(locNumCommandLists);

		std::wstring name(L"SR_CommandList[");
		name += std::to_wstring(num);
		name += L']';

		myCommandList->SetName(name.c_str());


		myQueueManager = static_cast<SR_QueueManager_DX12*>(SR_QueueManager::Get());
	}
	SR_CommandList_DX12::~SR_CommandList_DX12()
	{
		if (myCommandList)
			myCommandList->Release();

#if SR_ENABLE_RAYTRACING
		if (myCommandList4)
			myCommandList4->Release();
#endif
	}
	ID3D12GraphicsCommandList* SR_CommandList_DX12::GetCommandList() const
	{
		return myCommandList;
	}

#if SR_ENABLE_RAYTRACING
	ID3D12GraphicsCommandList4* SR_CommandList_DX12::GetCommandList4() const
	{
		return myCommandList4;
	}
#endif

	void SR_CommandList_DX12::FinishRecording()
	{
		myNeedsExecute = myNumCommands > 0;
		for (;;)
		{
			uint8 state = myState;
			assert((state & (Idle|Recording)) == (Idle|Recording));

			if (uint8 queueType = state & BlockingExecute)
			{
				assert(SC_Atomic_CompareExchange(myState, Idle, state));
				SR_QueueManager::Get()->FinishBlockingCmdList(SR_ContextType(queueType));
				return;
			}
			else
			{
				assert(state == (Idle|Recording));
				if (SC_Atomic_CompareExchange(myState, Idle, Idle | Recording))
					return;
			}
		}
	}

	bool SR_CommandList_DX12::Close(bool aIsAllowedToFail)
{
		for (;;)
		{
			uint8 state = myState;
			if (!state)
				return true;
			else if (state & Closing)
			{
				if (aIsAllowedToFail)
					return false;
				assert(false);
			}

			if (SC_Atomic_CompareExchange(myState, state | Closing, state))
				break;
		}

		assert(myState & Idle);
		HRESULT hr = myCommandList->Close();
		if (FAILED(hr))
		{
			SC_ERROR_LOG("Failed to close CommandList");
			assert(false);
		}

		for (;;)
		{
			uint8 state = myState;
			assert(state & Idle);

			if (uint8 queueType = state & BlockingExecute)
			{
				assert(state & Recording);
				assert(SC_Atomic_CompareExchange(myState, 0, state));
				SR_QueueManager::Get()->FinishBlockingCmdList(SR_ContextType(queueType));
				break;
			}
			else
			{
				if (SC_Atomic_CompareExchange(myState, 0, state))
					break;
			}
		}

		//uint numCommands = myNumDraws + myNumBarriers + 1;
		//SC_Atomic_Add(myAllocator->myCommandsSinceResetCount, numCommands);

		SC_Atomic_Add(myAllocator->myCommandsSinceResetCount, myNumCommands);
		assert(SC_Atomic_Exchange_GetOld(myAllocator->myCommandListsActive, 0) == 1);

		SC_MutexLock lock(myPool->myAllocatorsMutex);

		//SC_LOG("Returning allocator (%p) fence: %llu, ctx: %i", myAllocator, myAllocator->myFence.myFenceValue, myAllocator->myFence.myContextType);

		if (myAllocator->IsFilled())
			myPool->myInFlightAllocators.Add(myAllocator);
		else
			myPool->myActiveAllocators.Add(myAllocator);

		myAllocator->RemoveRef();
		return true;
	}

	void SR_CommandList_DX12::FlushPendingTransitions()
	{
		SC_HybridArray<D3D12_RESOURCE_BARRIER, 128> barriers;

		for (auto it = myPendingTransitions.Begin(), e = myPendingTransitions.End(); it != e; ++it)
		{
			const SC_Pair<uint16, uint16>& transitionStates = it.Item();
			SR_TrackedResource* tracked = it.Key();

			if (tracked->myDX12Tracking.myState == transitionStates.myFirst)
				continue;

			D3D12_RESOURCE_BARRIER& barrier = barriers.Add();
			barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			barrier.Transition.pResource = tracked->myDX12Resource;
			barrier.Transition.StateBefore = static_cast<D3D12_RESOURCE_STATES>(tracked->myDX12Tracking.myState);
			barrier.Transition.StateAfter = static_cast<D3D12_RESOURCE_STATES>(transitionStates.myFirst);
			barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES; 

			tracked->myDX12Tracking.myFenceContext = myCurrentContextType;
		}

		if (barriers.Count())
		{
			myCommandList->ResourceBarrier(barriers.Count(), barriers.GetBuffer());
			myNumCommands++;
		}
	}
	void SR_CommandList_DX12::UpdateResourceGlobalStates()
	{
		for (auto it = myPendingTransitions.Begin(), e = myPendingTransitions.End(); it != e; ++it)
		{
			const SC_Pair<uint16, uint16>& transitionStates = it.Item();
			SR_TrackedResource* trackedResource = it.Key();

			trackedResource->myDX12Tracking.myState = transitionStates.mySecond;
		}
	}

	bool SR_CommandList_DX12::CanRelease(volatile uint& aRefCount)
	{
		if (!myPool)
			return SC_RefCounted::CanRelease(aRefCount);

		SC_MutexLock cmdListLock(myPool->myCmdListsMutex);
		for (;;)
		{
			int oldCount = aRefCount;
			int newCount = oldCount - 1;

			if (newCount == 0)
			{
				assert(!myNext);
				if (myState & Recording)
				{
					newCount = SC_Atomic_Decrement_GetNew(aRefCount);
					assert(newCount == 0);

					myPool = nullptr;
					myAllocator = nullptr;
					myTransitionCmdList = nullptr;
					return true;
				}
				else
				{
					SC_Ref<SR_CommandAllocator_DX12> allocatorKeepAlive;
					if (myState & Idle)
					{
						allocatorKeepAlive = myAllocator;
						if (SR_QueueManager_DX12* qm = static_cast<SR_QueueManager_DX12*>(SR_QueueManager::Get()))
						{
							qm->Close(this);
							continue;
						}
						else
							Close();
					}

					if (myAllocator)
					{
						if (myFence.myFenceValue && myQueueManager->GetFenceManager().IsPending(myFence))
						{
							SC_MutexLock lock(myAllocator->myMutex);

							SR_Fence& maxFence = myAllocator->myFence;
							if (myFence.myFenceValue > maxFence.myFenceValue)
							{
								assert(myFence.myFenceValue > 0);
								maxFence = myFence;
							}
						}

						//SC_LOG("Destroying CMD list (%p) with allocator (%p), fence: %llu, ctx: %i", this, myAllocator, myAllocator->myFence.myFenceValue, myAllocator->myFence.myContextType);
						SC_Atomic_Decrement_GetNew(myAllocator->myCommandListsAlive);
						myAllocator = nullptr;
					}

					myFence.myFenceValue = 0;

					assert(!myState);
					if (!myPool->myIdleCmdLists.AddUnique(this))
						assert(false && "Tried to idle cmd list that is already considered idle");

					newCount = SC_Atomic_Decrement_GetNew(aRefCount);
					assert(newCount == 1);

					return false;
				}
			}
			else if (SC_Atomic_CompareExchange(aRefCount, newCount, oldCount))
				return false;

		}
	}
	void SR_CommandList_DX12::Open(SR_CommandAllocator_DX12* aCmdAllocator)
	{
		assert((myState & (Idle|Recording)) == Recording);
		assert(!myAllocator);
		assert(aCmdAllocator->myType == myType);
		assert(aCmdAllocator->myCommandListsActive == 1);

		myAllocator = aCmdAllocator;
		HRESULT hr = myCommandList->Reset(myAllocator->myAllocator, nullptr);
		//SC_LOG("Opening cmd-list (%p) with allocator (%p)", this, myAllocator);
		if (FAILED(hr))
		{
			SC_ERROR_LOG("Failed to reset CommandList");
			assert(false);
		}

		uint8 oldState = SC_Atomic_Or(myState, Idle);
		assert((oldState & (Idle|Recording)) == Recording);
	}

	void SR_CommandList_DX12::BeginRecording(const SR_ContextType& aContextType)
	{
		assert(!myState);
		assert(!myAllocator);

		myPendingTransitions.Reset();
		myCurrentContextType = aContextType;
		myNumCommands = 0;
		myLastFrameOpened = SC_Timer::GetGlobalFrameIndex();
		myLastTimeOpened = SC_Timer::GetGlobalTotalTime();
		myNeedsExecute = true;

		SC_Atomic_CompareExchange(myState, Recording, 0);
	}
}
#endif