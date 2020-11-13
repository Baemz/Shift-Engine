#include "SRender_Precompiled.h"
#include "SR_RenderThreadPool.h"
#include "SR_RenderThread.h"
#include "SR_CommandList.h"

namespace Shift
{
	SR_RenderThreadPool::SR_RenderThreadPool(uint aNumThreads)
	{
		myThreads.PreAllocate(aNumThreads);
		for (uint i = 0; i < aNumThreads; ++i)
		{
			std::string name("Render Worker");
			SR_RenderThread* thread = myThreads.Add(new SR_RenderThread(this));
			thread->Start((name + std::to_string(i)).c_str());
		}
	}
	SR_RenderThreadPool::~SR_RenderThreadPool()
	{
		{
			SC_MutexLock lock(myMutex);
			_myQueueData.Clear();
		}
		for (uint i = 0; i < myThreads.Count(); ++i)
			myThreads[i]->Stop();

		myHasWork.Signal();

		for (uint i = 0; i < myThreads.Count(); ++i)
			delete myThreads[i];
	}
	SC_Ref<SR_Waitable> SR_RenderThreadPool::PostRenderTask(std::function<void()> aTask, uint aWaitMode)
	{
		return PushTask(aTask, aWaitMode, SR_ContextType_Render);
	}
	SC_Ref<SR_Waitable> SR_RenderThreadPool::PostComputeTask(std::function<void()> aTask, uint aWaitMode)
	{
		return PushTask(aTask, aWaitMode, SR_ContextType_Compute);
	}
	SC_Ref<SR_Waitable> SR_RenderThreadPool::PostCopyTask(std::function<void()> aTask, uint aWaitMode)
	{
		return PushTask(aTask, aWaitMode, SR_ContextType_CopyFast);
	}
	bool SR_RenderThreadPool::Flush(bool aBlock)
	{
		if (aBlock)
		{
			while (!_myQueueData.Empty())
				std::this_thread::sleep_for(std::chrono::microseconds(1));
		}
		else
		{
			return _myQueueData.Empty();;
		}
		return true;
	}
	bool SR_RenderThreadPool::IsIdle() const
	{
		return _myQueueData.Empty();
	}
	bool SR_RenderThreadPool::PopTask(std::function<void()>& aTaskOut)
	{
		if (_myQueueData.Empty())
			return false;

		SC_MutexLock lock(myMutex);
		if (_myQueueData.Empty())
			return false;

		aTaskOut = _myQueueData.Peek();
		_myQueueData.Remove();
		return true;
	}

	void SR_RenderThreadPool::WaitForWork()
	{
		myHasWork.Wait();
	}

	void SR_RenderThreadPool::ResetHasWorkSignal()
	{
		SC_MutexLock lock(myWorkMutex);
		if (myHasWork.IsSignalled())
			myHasWork.ResetSignal();
	}

	SC_Ref<SR_Waitable> SR_RenderThreadPool::PushTask(std::function<void()> aTask, uint aWaitMode, SR_ContextType aContextType)
	{
		static SC_Mutex locSubmissionMutex;

		SC_Ref<SR_Waitable> event = new SR_Waitable(SR_WaitableMode(aWaitMode));

		SR_Waitable* eventPtr = event;
		eventPtr->AddRef();

		auto task = [&, aTask, eventPtr, aWaitMode, aContextType]() mutable
		{
			SC_PROFILER_BEGIN_SECTION(SC_PROFILER_TAG_VIEW_RENDER);

			SR_GraphicsContext* ctx = nullptr;
			if (aWaitMode > SR_WaitableMode_CPU)
			{
				ctx = SR_GraphicsDevice::GetDevice()->GetContext(aContextType);
				ctx->Begin();
				ctx->BeginRecording();
			}

			aTask();

			if (aWaitMode > SR_WaitableMode_CPU)
			{

				for (SR_Waitable* event : ctx->myInsertedWaitables)
					SR_QueueManager::InsertWait(SR_Fence(event->myFence, event->myFenceContext), aContextType);

				myPendingContexts[aContextType].push(ctx);

				SC_GrowingArray<SC_Ref<SR_GraphicsContext>> contexts;
				if (myPendingContexts[aContextType].TryPopAll(contexts))
				{
					SC_MutexLock submitLock(locSubmissionMutex);
					SC_GrowingArray<SR_CommandList*> cls;
					cls.PreAllocate(contexts.Count());
					for (SR_GraphicsContext* queuedCtx : contexts)
					{
						queuedCtx->EndRecording();
						cls.Add(queuedCtx->GetCommandList());
					}

					if (cls.Count() > 1)
						std::sort(cls.begin(), cls.end());

					SR_Fence fence = cls.GetLast()->GetFence();
					eventPtr->myFence = fence.myFenceValue;
					eventPtr->myFenceContext = fence.myContextType;
					SR_QueueManager::ExecuteCommandLists(cls, aContextType, fence.myFenceValue);

					for (SR_GraphicsContext* queuedCtx : contexts)
						queuedCtx->End();
				}

				SR_GraphicsContext::SetCurrentContext(nullptr);
			}
			eventPtr->myEventCPU.Signal();

			eventPtr->RemoveRef();

			SC_PROFILER_END_SECTION();
		};

		{
			SC_MutexLock lock(myMutex);
			_myQueueData.Add(task);
		}
		myHasWork.Signal();
		return event;
	}
}