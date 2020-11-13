#pragma once
#include "SR_GraphicsEngineEnums.h"

#include "SC_Future.h"

#define MAX_NUM_CONTEXTS_BATCH 15

namespace Shift
{
	class SC_Thread;
	class SR_GraphicsDevice;
	class SR_GraphicsContext;
	class SR_WaitEvent;
	enum SR_QueueTaskType
	{
		SR_QueueTaskType_Execute,
		SR_QueueTaskType_Signal,
		SR_QueueTaskType_Wait,
		SR_QueueTaskType_EventStart,
		SR_QueueTaskType_EventEnd,
		SR_QueueTaskType_Quit
	};

	class SR_CommandList;
	class SR_QueueManager
	{
	public:
		virtual ~SR_QueueManager();

		static void Init();
		static void Destroy();
		static void ExecuteCommandLists(SC_GrowingArray<SR_CommandList*>& aCommandLists, const SR_ContextType& aQueue, uint64 aExpectedFence = 0);

		static void BeginFrameEvent(const SR_ContextType& aQueue);
		static void EndFrameEvent(const SR_ContextType& aQueue);

		static void Signal(uint64 aFence, const SR_ContextType& aQueue);
		static void InsertWait(const SR_Fence& aFence, const SR_ContextType& aWaitingQueue);

		static SC_Ref<SR_CommandList> GetCommandList(SR_ContextType aContextType);

		virtual void Close(SR_CommandList* aCmdList) = 0;
		virtual void Wait(const SR_Fence& aFence) = 0;
		virtual SR_Fence GetNextExpectedFence(const SR_ContextType& aType) const = 0;
		virtual uint64 GetTimestampFreq(const SR_ContextType& aType) const = 0;
		virtual bool IsPending(const SR_Fence& aFence) = 0;

		static SR_QueueManager* Get();

		void FinishBlockingCmdList(SR_ContextType aQueueType);

		struct QueueData2
		{
			SR_Fence myFence;
			SR_ContextType myContextType;
			SR_QueueTaskType myTaskType;
			union 
			{
				SR_CommandList* myCmdList;
				SC_Semaphore* mySemaphore;
			};
		};

		struct QueueDataPerType
		{
			uint64 myLastQueuedSignal;

			SC_Mutex myMutex;
			SC_DynamicCircularArray<QueueData2> myQueue;

			QueueData2 myCurrentData;
			bool myHasCurrentTask;

			SR_Fence myBlockingFence;
			volatile int myNumBlockingCmdLists;

			QueueDataPerType()
				: myLastQueuedSignal(0)
				, myHasCurrentTask(false)
				, myNumBlockingCmdLists(0)
			{}
		};

	protected:

		SR_QueueManager();
		void ExectueFunc();

		virtual void Init_Internal() = 0;
		virtual bool Execute_Internal(QueueData2& aTask) = 0;
		virtual void UpdateCommandListPool_Internal() = 0;

		virtual void NativeSignal(const SR_Fence& aFence) = 0;
		virtual void NativeWait(const SR_Fence& aFence, const SR_ContextType& aWaitingQueue) = 0;
		virtual void NativeBeginEvent(const SR_ContextType& aQueue) = 0;
		virtual void NativeEndEvent(const SR_ContextType& aQueue) = 0;

		void AddTask(const QueueData2& aTask);

		bool TryExecuteTask(bool& aQuit);
		bool TryExecuteTask(QueueData2& aTask, bool& aQuit);

		virtual SC_Ref<SR_CommandList> GetCommandList_Internal(SR_ContextType aContextType) = 0;

		static SR_QueueManager* ourInstance;
		static SR_ContextType ourTypeExecutionOrder[SR_ContextType_COUNT];
		static uint ourNumTypesProcessed;

		QueueDataPerType myQueueDatasPerType[SR_ContextType_COUNT];
		SC_FastEvent myHasWork;

		SC_FutureBase myThread;
		volatile uint64 myLastFlushedSignals[SR_ContextType_COUNT];
	};

}