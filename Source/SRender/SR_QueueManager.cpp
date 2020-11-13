#include "SRender_Precompiled.h"
#include "SR_QueueManager.h"

#if ENABLE_VULKAN
#include "SR_QueueManager_Vk.h"
#endif
#if ENABLE_PS4
// Include PS4 stuff
#	include "SR_GraphicsQueueManager_PS4.h" 
#endif
#if ENABLE_DX12
#	include "SR_QueueManager_DX12.h" 
#endif

#include "SR_CommandList.h"

namespace Shift
{
	SR_QueueManager* SR_QueueManager::ourInstance = nullptr;
	SR_ContextType SR_QueueManager::ourTypeExecutionOrder[SR_ContextType_COUNT];
	uint SR_QueueManager::ourNumTypesProcessed;

	SR_QueueManager::SR_QueueManager()
	{
		for (uint i = 0; i< SR_ContextType_COUNT; ++i)
			myLastFlushedSignals[i] = 0;
	}

	SR_QueueManager::~SR_QueueManager()
	{
	}
	void SR_QueueManager::Init()
	{
		if (ourInstance == nullptr)
		{
			switch (SR_GraphicsDevice::GetDevice()->APIType())
			{
#if ENABLE_VULKAN
			case SR_GraphicsAPI::Vulkan:
				ourInstance = new SR_QueueManager_Vk();
				break;
#endif
#if ENABLE_PS4
			case SR_GraphicsAPI::PS4:
				ourInstance = new SR_GraphicsQueueManager_PS4();
				break;
#endif
#if ENABLE_DX12
			case SR_GraphicsAPI::DirectX12:
				ourInstance = new SR_QueueManager_DX12();
				break;
#endif
			}

			SC_ThreadProperties threadProps;
			threadProps.myName = "Queue Manager Execute";
			threadProps.myAffinity = SC_ThreadTools::ourRenderThreadAffinity | SC_ThreadTools::ourFrameTaskThreadAffinity;
			threadProps.myPriority = SC_ThreadPriority_Critical;
			ourInstance->myThread = SC_CreateThread(threadProps, ourInstance, &SR_QueueManager::ExectueFunc);
		}
	}
	void SR_QueueManager::Destroy()
	{
		//QueueData qData;
		//qData.myTaskType = SR_QueueTaskType_Quit;
		//SR_GraphicsQueueManager_Private::ourQueueData.push(qData);
		QueueData2 task;
		task.myTaskType = SR_QueueTaskType_Quit;
		task.myContextType = SR_ContextType_Render;
		ourInstance->AddTask(task);

		ourInstance->myThread.Stop();
		SC_SAFE_DELETE(ourInstance);
	}

	void SR_QueueManager::ExecuteCommandLists(SC_GrowingArray<SR_CommandList*>& aCommandLists, const SR_ContextType& aQueue, uint64 aExpectedFence)
	{
		SR_CommandList* prev = nullptr;
		for (SR_CommandList* cmdList : aCommandLists)
		{
			assert(!cmdList->myNext);

			cmdList->AddRef();

			if (prev)
				prev->myNext = cmdList;

			prev = cmdList;
		}

		QueueData2 task;
		task.myContextType = aQueue;
		task.myTaskType = SR_QueueTaskType_Execute;
		task.myFence = SR_Fence(aExpectedFence, aQueue);
		task.myCmdList = aCommandLists.GetFirst();

		QueueDataPerType& queueData = ourInstance->myQueueDatasPerType[aQueue];

		SC_ASSERT(aExpectedFence > queueData.myLastQueuedSignal);
		if (aExpectedFence > queueData.myLastQueuedSignal)
		{
			SC_ASSERT(aExpectedFence > queueData.myLastQueuedSignal || !queueData.myLastQueuedSignal || aExpectedFence != SC_UINT64_MAX);
			queueData.myLastQueuedSignal = aExpectedFence;
		}

		ourInstance->AddTask(task);
	}

	void SR_QueueManager::BeginFrameEvent(const SR_ContextType& aQueue)
	{
		QueueData2 task;
		task.myTaskType = SR_QueueTaskType_EventStart;
		task.myContextType = aQueue;
		ourInstance->AddTask(task);
	}

	void SR_QueueManager::EndFrameEvent(const SR_ContextType& aQueue)
	{
		QueueData2 task;
		task.myTaskType = SR_QueueTaskType_EventEnd;
		task.myContextType = aQueue;
		ourInstance->AddTask(task);
	}

	void SR_QueueManager::Signal(uint64 aFence, const SR_ContextType& aQueue)
	{
		QueueDataPerType& queueData = ourInstance->myQueueDatasPerType[aQueue];
		SC_ASSERT(aFence > queueData.myLastQueuedSignal);
		if (queueData.myLastQueuedSignal >= aFence)
			return;

		if (aFence > queueData.myLastQueuedSignal)
		{
			assert(aFence > queueData.myLastQueuedSignal || !queueData.myLastQueuedSignal);
			queueData.myLastQueuedSignal = aFence;
		}

		QueueData2 task;
		task.myContextType = aQueue;
		task.myTaskType = SR_QueueTaskType_Signal;
		task.myFence = SR_Fence(aFence, aQueue);

		ourInstance->AddTask(task);
		//ourInstance->Signal_Internal(aFence, aQueue);
	}

	void SR_QueueManager::InsertWait(const SR_Fence& aFence, const SR_ContextType& aWaitingQueue)
	{
		QueueData2 task;
		task.myContextType = aWaitingQueue;
		task.myTaskType = SR_QueueTaskType_Wait;
		task.myFence = aFence;
		ourInstance->AddTask(task);
	}

	SC_Ref<SR_CommandList> SR_QueueManager::GetCommandList(SR_ContextType aContextType)
	{
		return ourInstance->GetCommandList_Internal(aContextType);
	}

	SR_QueueManager* SR_QueueManager::Get()
	{
		return ourInstance;
	}

	void SR_QueueManager::FinishBlockingCmdList(SR_ContextType aQueueType)
	{
		if (SC_Atomic_Decrement_GetNew(myQueueDatasPerType[aQueueType].myNumBlockingCmdLists) <= 0)
			myHasWork.Signal();
	}

	void SR_QueueManager::ExectueFunc()
	{
		bool quit = false;

		while (!quit)
		{
			myHasWork.Wait();
			myHasWork.ResetSignal();

			for (;;)
			{
				if (!TryExecuteTask(quit))
					break;
			}
		}
	}

	void SR_QueueManager::AddTask(const QueueData2& aTask)
	{
		QueueDataPerType& qData = myQueueDatasPerType[aTask.myContextType];
		{
			SC_MutexLock lock(qData.myMutex);
			qData.myQueue.Add(aTask);
		}
		myHasWork.Signal();
	}

	bool SR_QueueManager::TryExecuteTask(bool& aQuit)
	{
		uint numTypesToProcess = ourNumTypesProcessed;

		for (uint i = 0; i < numTypesToProcess; ++i)
		{
			SR_ContextType type = ourTypeExecutionOrder[i];
			QueueDataPerType& queueData = myQueueDatasPerType[type];
			if (queueData.myQueue.Empty() && !queueData.myHasCurrentTask)
				continue;

			if (queueData.myBlockingFence.myFenceValue)
			{
				SR_ContextType waitingQueueType = queueData.myBlockingFence.myContextType;
				if (myLastFlushedSignals[waitingQueueType] < queueData.myBlockingFence.myFenceValue)
					continue;

				queueData.myBlockingFence.myFenceValue = 0;
			}

			if (!queueData.myHasCurrentTask)
			{
				SC_MutexLock lock(queueData.myMutex);
				queueData.myCurrentData = queueData.myQueue.Peek();
				queueData.myHasCurrentTask = true;
				queueData.myQueue.Remove();
			}

			if (TryExecuteTask(queueData.myCurrentData, aQuit))
			{
				queueData.myHasCurrentTask = false;
				return true;
			}
			
		}

		return false;
	}

	bool SR_QueueManager::TryExecuteTask(QueueData2& aTask, bool& aQuit)
	{
		QueueDataPerType& queueData = myQueueDatasPerType[aTask.myContextType];

		switch (aTask.myTaskType)
		{
		case SR_QueueTaskType_Execute:
		{
			return Execute_Internal(aTask);
		}
		case SR_QueueTaskType_Signal:
		{
			SR_ContextType fenceQueueType = aTask.myFence.myContextType;
			NativeSignal(aTask.myFence);

			assert(myLastFlushedSignals[fenceQueueType] < aTask.myFence.myFenceValue);
			myLastFlushedSignals[fenceQueueType] = aTask.myFence.myFenceValue;

			return true;
		}
		case SR_QueueTaskType_Wait:
		{
			SR_ContextType fenceQueueType = aTask.myFence.myContextType;
			assert(aTask.myContextType != fenceQueueType);

			if (myLastFlushedSignals[fenceQueueType] < queueData.myCurrentData.myFence.myFenceValue)
			{
				assert(aTask.myFence.myFenceValue > queueData.myBlockingFence.myFenceValue);
				queueData.myBlockingFence = SR_Fence(aTask.myFence.myFenceValue, fenceQueueType);
				return false;
			}

			NativeWait(aTask.myFence, aTask.myContextType);
			return true;
		}
		case SR_QueueTaskType_Quit:
		{
			aQuit = true;
			return true;
		}
		case SR_QueueTaskType_EventStart:
		{
			NativeBeginEvent(aTask.myContextType);
			return true;
		}
		case SR_QueueTaskType_EventEnd:
		{
			NativeEndEvent(aTask.myContextType);
			return true;
		}
		}
		assert(0 && "TaskType not implemented");
		return true;
	}
}