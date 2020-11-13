#pragma once

namespace Shift
{
	class SR_CommandList;
	class SR_RenderThread;
	class SR_GraphicsContext;
	class SR_RenderThreadPool
	{
	public:
		SR_RenderThreadPool(uint aNumThreads);
		~SR_RenderThreadPool();

		SC_Ref<SR_Waitable> PostRenderTask(std::function<void()> aTask, uint aWaitMode = SR_WaitMode_CPU);
		SC_Ref<SR_Waitable> PostComputeTask(std::function<void()> aTask, uint aWaitMode = SR_WaitMode_CPU);
		SC_Ref<SR_Waitable> PostCopyTask(std::function<void()> aTask, uint aWaitMode = SR_WaitMode_CPU);

		bool Flush(bool aBlock);
		bool IsIdle() const;

		bool PopTask(std::function<void()>& aTaskOut);
		void WaitForWork();
		void ResetHasWorkSignal();

	private:
		SC_Ref<SR_Waitable> PushTask(std::function<void()> aTask, uint aWaitMode, SR_ContextType aContextType);

		SC_GrowingArray<SR_RenderThread*> myThreads;

		SC_Mutex myWorkMutex;
		SC_FastEvent myHasWork;
		SC_Mutex myMutex;
		SC_DynamicCircularArray<std::function<void()>> _myQueueData;

		SC_ThreadSafeQueue<SC_Ref<SR_GraphicsContext>> myPendingContexts[SR_ContextType::SR_ContextType_COUNT];
	};
}
