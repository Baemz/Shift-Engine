#include "SRender_Precompiled.h"
#include "SR_CommandListPool_DX12.h"
#if ENABLE_DX12
#include "SR_CommandAllocator_DX12.h"

namespace Shift
{
	SR_CommandListPool_DX12::SR_CommandListPool_DX12(D3D12_COMMAND_LIST_TYPE aType, const char* aDebugName)
		: myType(aType)
		, myAllocatorCount(0)
		, myCmdListCount(0)
		, myDebugName(aDebugName)
	{
	}

	SR_CommandListPool_DX12::~SR_CommandListPool_DX12()
	{
		for (SR_CommandAllocator_DX12* allocator : myActiveAllocators)
		{
			assert(allocator->GetRefCount() == 1);
			allocator->myPool = nullptr;
		}
		SC_Atomic_Sub(myAllocatorCount, myActiveAllocators.Count());
		myActiveAllocators.RemoveAll();

		for (SR_CommandAllocator_DX12* allocator : myIdleAllocators)
		{
			assert(allocator->GetRefCount() == 1);
			allocator->myPool = nullptr;
		}
		SC_Atomic_Sub(myAllocatorCount, myIdleAllocators.Count());
		myIdleAllocators.RemoveAll();

		while (!myInFlightAllocators.Empty())
		{
			SR_CommandAllocator_DX12* allocator = myInFlightAllocators.Peek();
			assert(allocator->GetRefCount() == 1);
			allocator->myPool = nullptr;
			SC_Atomic_Decrement(myAllocatorCount);
			myInFlightAllocators.Remove();
		}

		for (SR_CommandList_DX12* cmdList : myIdleCmdLists)
		{
			assert(cmdList->GetRefCount() == 1);
			assert(!cmdList->myAllocator);
			cmdList->myPool = nullptr;
		}

		SC_Atomic_Sub(myCmdListCount, myIdleCmdLists.Count());
		myIdleCmdLists.RemoveAll();

		assert(!myAllocatorCount);
		assert(!myCmdListCount);
	}

	SC_Ref<SR_CommandList_DX12> SR_CommandListPool_DX12::GetCommandList(SR_ContextType aType)
	{
		SC_Ref<SR_CommandList_DX12> list;

		if (myIdleCmdLists.Count())
		{
			SC_MutexLock lock(myCmdListsMutex);
			if (myIdleCmdLists.Count())
			{
				list = myIdleCmdLists.GetFirst();
				myIdleCmdLists.RemoveByIndexCyclic(0);
			}
		}

		SC_Ref<SR_CommandAllocator_DX12> allocator = GetAllocator();
		if (list)
		{
			list->BeginRecording(aType);
			list->Open(allocator);
		}
		else
		{
			list = new SR_CommandList_DX12(myType, allocator, this, aType);
			list->myPoolId = myCmdListCount;
			SC_Atomic_Increment(myCmdListCount);
		}
		return list;
	}

	void SR_CommandListPool_DX12::OpenCommandList(SR_CommandList_DX12* aCmdList)
	{
		SC_Ref<SR_CommandAllocator_DX12> allocator = GetAllocator();
		aCmdList->Open(allocator);
	}

	void SR_CommandListPool_DX12::Update()
	{
		if (!myAllocatorCount && !myIdleCmdLists.Count())
			return;

		const uint64 currentFrame = SC_Timer::GetGlobalFrameIndex();
		const uint64 removeAfterFrames = 100;
		const float currentTime = SC_Timer::GetGlobalTotalTime();
		const float removeAfterTime = 240;

		SC_HybridArray<SC_Ref<SC_RefCounted>, 32> pendingRemoves;

		if (myAllocatorCount)
		{
			SC_MutexLock lock(myAllocatorsMutex);
			if (myIdleAllocators.Count())
			{
				SR_CommandAllocator_DX12* allocator = myIdleAllocators.GetFirst();
				if (allocator->myLastFrameOpened + removeAfterFrames < currentFrame ||
					allocator->myLastTimeOpened + removeAfterTime < currentTime)
				{
					assert(allocator->IsIdle());
					allocator->myPool = nullptr;
					SC_Atomic_Decrement(myAllocatorCount);
					pendingRemoves.Add(allocator);
					myIdleAllocators.RemoveByIndex(0);
				}
			}

			for (int i = int(myActiveAllocators.Count() - 1); i >= 0; --i)
			{
				SR_CommandAllocator_DX12* allocator = myActiveAllocators[i];
				if (allocator->IsIdle())
				{
					allocator->Reset();
					myIdleAllocators.Add(allocator);
					myActiveAllocators.RemoveByIndexCyclic(i);
				}
			}

			while (!myInFlightAllocators.Empty())
			{
				SR_CommandAllocator_DX12* allocator = myInFlightAllocators.Peek();
				if (allocator->IsIdle())
				{
					allocator->Reset();
					myIdleAllocators.Add(allocator);
					myInFlightAllocators.Remove();
				}
				else 
					break;
			}
		}

		if (myIdleCmdLists.Count())
		{
			SC_MutexLock lock(myCmdListsMutex);

			if (myIdleCmdLists.Count())
			{
				SR_CommandList_DX12* cmdList = myIdleCmdLists.GetFirst();
				if (cmdList->myLastFrameOpened + removeAfterFrames < currentFrame ||
					cmdList->myLastTimeOpened + removeAfterTime < currentTime)
				{
					SC_Atomic_Decrement_GetNew(myCmdListCount);
					pendingRemoves.Add(cmdList);
					myIdleCmdLists.RemoveFirst();
				}
			}
		}
	}

	SC_Ref<SR_CommandAllocator_DX12> SR_CommandListPool_DX12::GetAllocator()
	{
		{
			SC_MutexLock lock(myAllocatorsMutex);

			if (myActiveAllocators.Count())
			{
				SR_CommandAllocator_DX12* allocator = myActiveAllocators.GetLast();

				allocator->AddRef();
				allocator->BeginCommandList();
				myActiveAllocators.RemoveLast();
				return allocator;
			}

			if (myIdleAllocators.Count())
			{
				SR_CommandAllocator_DX12* allocator = myIdleAllocators.GetLast();

				allocator->AddRef();
				allocator->BeginCommandList();
				myIdleAllocators.RemoveLast();
				return allocator;
			}

			if (!myInFlightAllocators.Empty())
			{
				SR_CommandAllocator_DX12* allocator = myInFlightAllocators.Peek();
				if (allocator->IsIdle())
				{
					allocator->AddRef();
					myInFlightAllocators.Remove();
					lock.Unlock();
			
					allocator->Reset();
					allocator->BeginCommandList();
					return allocator;
				}
			}
		}

		SC_Ref<SR_CommandAllocator_DX12> allocator = new SR_CommandAllocator_DX12(myType, this);
		allocator->AddRef();
		allocator->BeginCommandList();
		return allocator;
	}
}
#endif