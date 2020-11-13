#include "SRender_Precompiled.h"
#include "SR_QueueManager_DX12.h"

#if ENABLE_DX12
#include "SR_CommandListPool_DX12.h"
#include "SR_CommandList_DX12.h"
#include "SR_CommandAllocator_DX12.h"
#include "SR_GraphicsDevice_DX12.h"
#include "SR_GraphicsContext_DX12.h"

namespace Shift
{
	SR_QueueManager_DX12::SR_QueueManager_DX12()
	{

		SR_ContextType queueOrder[] = { SR_ContextType_Compute, SR_ContextType_Render, SR_ContextType_CopyFast, SR_ContextType_CopyStream, SR_ContextType_CopyInit };
		ourNumTypesProcessed = 5;
		SC_Memcpy(&ourTypeExecutionOrder, &queueOrder, sizeof(queueOrder));
		Init_Internal();
	}

	SR_QueueManager_DX12::~SR_QueueManager_DX12()
	{
		for (int i = 0; i < SR_ContextType_COUNT; ++i)
			myQueues[i]->Release();

		myPendingCloseCmd.Add(nullptr);
		myHasUpdateWork.Signal();
	}

	ID3D12CommandQueue* SR_QueueManager_DX12::GetQueue(const SR_ContextType& aType) const
	{
		return myQueues[aType];
	}

	void SR_QueueManager_DX12::Close(SR_CommandList* aCmdList)
	{
		static constexpr uint maxPendingCount = 2;

		SR_CommandList_DX12* cmdList = static_cast<SR_CommandList_DX12*>(aCmdList);
		if (myPendingCloseCmd.Count() < maxPendingCount)
		{
			{
				SC_MutexLock lock(myPendingCloseMutex);
				assert(cmdList->myFence.myFenceValue > 0);
				myPendingCloseCmd.Add(cmdList);
			}
			myHasUpdateWork.Signal();
		}
		else
			cmdList->Close();
	}

	void SR_QueueManager_DX12::Wait(const SR_Fence& aFence)
	{
		myFenceManager.Wait(aFence);
	}

	SR_Fence SR_QueueManager_DX12::GetNextExpectedFence(const SR_ContextType& aType) const
	{
		SR_Fence fence = myFenceManager.GetLastInsertedFence(aType);
		fence.myFenceValue += 1;

		return fence;
	}

	uint64 SR_QueueManager_DX12::GetTimestampFreq(const SR_ContextType& aType) const
	{
		uint64 returnVal = 1;
		HRESULT res = myQueues[aType]->GetTimestampFrequency(&returnVal);
		if (FAILED(res))
			SC_ERROR_LOG("Could not retrieve Timestamp Frequency.");

		return returnVal;
	}

	bool SR_QueueManager_DX12::IsPending(const SR_Fence& aFence)
	{
		return myFenceManager.IsPending(aFence);
	}

	void SR_QueueManager_DX12::UpdateCmdListPoolFunc()
	{
		for (;;)
		{
			bool quit = false;

			myHasUpdateWork.Wait();
			myHasUpdateWork.ResetSignal();

			for (;;)
			{
				bool signalUpper = mySignalUpperQueue;
				if (signalUpper)
					mySignalUpperQueue = false;

				bool hasPendingClose = myPendingCloseCmd.Count() != 0;

				if (!signalUpper && !hasPendingClose)
					break;

				if (hasPendingClose)
				{
					{
						SC_MutexLock lock(myPendingCloseMutex);
						myPendingCloseCmd.Swap(myPendingCloseCmdTemp);
					}
					for (SR_CommandList_DX12* cmdList : myPendingCloseCmdTemp)
					{
						if (cmdList)
							cmdList->Close(true);
						else
							quit = true;
					}

					if (signalUpper)
						myHasWork.Signal();

					if (hasPendingClose)
						myPendingCloseCmdTemp.RemoveAll();
				}
			}

			if (quit)
				return;

			UpdateCommandListPool_Internal();
		}
	}

	void SR_QueueManager_DX12::Init_Internal()
	{
		SR_GraphicsDevice_DX12* deviceDX12 = static_cast<SR_GraphicsDevice_DX12*>(SR_GraphicsDevice::GetDevice());
		ID3D12Device* device = deviceDX12->GetNativeDevice();

		D3D12_COMMAND_QUEUE_DESC cqDesc = {};

		HRESULT hr = S_OK;
		// Render Queue
		cqDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		hr = device->CreateCommandQueue(&cqDesc, SR_IID_PPV_ARGS(&myQueues[SR_ContextType_Render]));
		if (FAILED(hr))
			return;
		myQueues[SR_ContextType_Render]->SetName(L"Graphics Queue");
		myCmdListPools[SR_ContextType_Render] = new SR_CommandListPool_DX12(cqDesc.Type, "Command List Pool [Render]");

		// Compute queue
		cqDesc.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
		hr = device->CreateCommandQueue(&cqDesc, SR_IID_PPV_ARGS(&myQueues[SR_ContextType_Compute]));
		if (FAILED(hr))
			return;
		myQueues[SR_ContextType_Compute]->SetName(L"Compute Queue");
		myCmdListPools[SR_ContextType_Compute] = new SR_CommandListPool_DX12(cqDesc.Type, "Command List Pool [Compute]");

		// Copy Queues
		cqDesc.Type = D3D12_COMMAND_LIST_TYPE_COPY;
		hr = device->CreateCommandQueue(&cqDesc, SR_IID_PPV_ARGS(&myQueues[SR_ContextType_CopyInit]));
		if (FAILED(hr))
			return;
		myQueues[SR_ContextType_CopyInit]->SetName(L"CopyInit Queue");
		myCmdListPools[SR_ContextType_CopyInit] = new SR_CommandListPool_DX12(cqDesc.Type, "Command List Pool [CopyInit]");

		hr = device->CreateCommandQueue(&cqDesc, SR_IID_PPV_ARGS(&myQueues[SR_ContextType_CopyStream]));
		if (FAILED(hr))
			return;
		myQueues[SR_ContextType_CopyStream]->SetName(L"CopyStream Queue");
		myCmdListPools[SR_ContextType_CopyStream] = new SR_CommandListPool_DX12(cqDesc.Type, "Command List Pool [CopySteam]");

		hr = device->CreateCommandQueue(&cqDesc, SR_IID_PPV_ARGS(&myQueues[SR_ContextType_CopyFast]));
		if (FAILED(hr))
			return;
		myQueues[SR_ContextType_CopyFast]->SetName(L"CopyFast Queue");
		myCmdListPools[SR_ContextType_CopyFast] = new SR_CommandListPool_DX12(cqDesc.Type, "Command List Pool [CopyFast]");

		SC_ThreadProperties cmdPoolthreadProps;
		cmdPoolthreadProps.myName = "Queue Manager Update Pool";
		cmdPoolthreadProps.myAffinity = SC_ThreadTools::ourRenderThreadAffinity | SC_ThreadTools::ourFrameTaskThreadAffinity;
		cmdPoolthreadProps.myPriority = SC_ThreadPriority_Critical;
		myUpdateCmdListPoolThread = SC_CreateThread(cmdPoolthreadProps, this, &SR_QueueManager_DX12::UpdateCmdListPoolFunc);
	}

	bool SR_QueueManager_DX12::Execute_Internal(QueueData2& aTask)
	{
		ID3D12CommandQueue* cmdQueue = myQueues[aTask.myContextType];
		SC_HybridArray<SR_CommandList_DX12*, 16> lists;
		QueueDataPerType& queueData = myQueueDatasPerType[aTask.myContextType];

		for (SR_CommandList* list = aTask.myCmdList; list; list = list->myNext)
		{
			SR_CommandList_DX12* cmdList = static_cast<SR_CommandList_DX12*>(list);

			for (;;)
			{
				uint8 state = cmdList->myState;
				if (state & SR_CommandList_DX12::Recording)
				{
					assert(!(state & SR_CommandList_DX12::BlockingExecute));
					assert((uint8(aTask.myContextType) & uint8(SR_CommandList_DX12::BlockingExecute)) == aTask.myContextType && aTask.myContextType);

					uint8 newState = state | aTask.myContextType;
					if (SC_Atomic_CompareExchange(cmdList->myState, newState, state))
					{
						SC_Atomic_Increment_GetNew(queueData.myNumBlockingCmdLists);
						break;
					}
				}
				else if (state & SR_CommandList_DX12::Idle)
				{
					if (!CloseCommandList(cmdList))
					{
						//SC_LOG("Couldn't close cmdlist (%p), returning", cmdList);
						return false;
					}

					assert(!cmdList->myState);
					break;
				}
				else
				{
					assert(!state);
					break;
				}
			}

			lists.Add(cmdList);
		}

		assert(queueData.myNumBlockingCmdLists >= 0);
		if (queueData.myNumBlockingCmdLists)
		{
			//SC_LOG("Execute task has blocking cmdlist");
			return false;
		}

		SC_HybridArray<SC_Ref<SR_CommandList>, 16> transitionListsKeepAlive;
		SC_HybridArray<ID3D12CommandList*, 16> cmdLists;
		for (SR_CommandList_DX12* cmdList : lists)
		{
			if (!cmdList->myNeedsExecute)
				continue;

			if (cmdList->myPendingTransitions.Count() > 0)
			{
				SR_CommandList* transitionCmdList = transitionListsKeepAlive.Add(GetCommandList(aTask.myContextType));
				SR_CommandList_DX12* transitionCmdListDX12 = static_cast<SR_CommandList_DX12*>(transitionCmdList);

				transitionCmdListDX12->myPendingTransitions = SC_Move(cmdList->myPendingTransitions);
				transitionCmdListDX12->FlushPendingTransitions();
				transitionCmdListDX12->UpdateResourceGlobalStates();
				transitionCmdListDX12->SetLastFence(cmdList->myFence);
				transitionCmdListDX12->FinishRecording();
				transitionCmdListDX12->Close(false);

				cmdLists.Add(transitionCmdListDX12->GetCommandList());
			}

			cmdLists.Add(cmdList->GetCommandList());
		}
		 
		if (cmdLists.Count())
			cmdQueue->ExecuteCommandLists(cmdLists.Count(), cmdLists.GetBuffer());

		NativeSignal(aTask.myFence);
		assert(myLastFlushedSignals[aTask.myFence.myContextType] < aTask.myFence.myFenceValue);
		myLastFlushedSignals[aTask.myFence.myContextType] = aTask.myFence.myFenceValue;

		for (int i = lists.Count() - 1; i >= 0; --i)
		{
			SR_CommandList* list = lists[i];
			list->myNext = nullptr;
			list->RemoveRef();
		}

		return true;
	}

	void SR_QueueManager_DX12::UpdateCommandListPool_Internal()
	{
		uint numTypesToProcess = ourNumTypesProcessed;

		for (uint i = 0; i < numTypesToProcess; ++i)
		{
			SR_ContextType type = ourTypeExecutionOrder[i];
			myCmdListPools[type]->Update();
		}
	}

	void SR_QueueManager_DX12::NativeSignal(const SR_Fence& aFence)
	{
		myQueues[aFence.myContextType]->Signal(myFenceManager.myFenceData[aFence.myContextType].myFence, aFence.myFenceValue);
	}

	void SR_QueueManager_DX12::NativeWait(const SR_Fence& aFence, const SR_ContextType& aWaitingQueue)
	{
		myQueues[aWaitingQueue]->Wait(myFenceManager.myFenceData[aFence.myContextType].myFence, aFence.myFenceValue);
	}

	void SR_QueueManager_DX12::NativeBeginEvent(const SR_ContextType& aQueue)
	{
#if ENABLE_PIX
		PIXBeginEvent(myQueues[aQueue], PIX_COLOR_DEFAULT, "FRAME %i", SC_Timer::GetGlobalFrameIndex());
#else
		SC_UNUSED(aQueue);
#endif
	}

	void SR_QueueManager_DX12::NativeEndEvent(const SR_ContextType& aQueue)
	{
#if ENABLE_PIX
		PIXEndEvent(myQueues[aQueue]);
#else
		SC_UNUSED(aQueue);
#endif
	}

	SC_Ref<SR_CommandList> SR_QueueManager_DX12::GetCommandList_Internal(SR_ContextType aContextType)
	{
		return myCmdListPools[aContextType]->GetCommandList(aContextType);
	}
	bool SR_QueueManager_DX12::CloseCommandList(SR_CommandList_DX12* aCmdList)
	{
		if (aCmdList->Close(true))
			return true;

		mySignalUpperQueue = true;
		myHasUpdateWork.Signal();
		return false;
	}
}
#endif