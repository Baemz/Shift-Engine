#include "SRender_Precompiled.h"
#include "SR_RenderThread.h"
#include "SR_RenderThreadPool.h"

namespace Shift
{
	SR_RenderThread::SR_RenderThread(SR_RenderThreadPool* aPool)
		: myIsRunning(false)
		, myPool(aPool)
	{
	}
	SR_RenderThread::~SR_RenderThread()
	{
		if (myIsRunning)
			Stop();

		myThread.Stop();
	}
	void SR_RenderThread::Start(const char* aThreadName)
	{
		myIsRunning = true;

		SC_ThreadProperties threadProps;
		threadProps.myName = aThreadName;
		threadProps.myPriority = SC_ThreadPriority_Highest;
		threadProps.myAffinity = SC_ThreadTools::ourRenderThreadAffinity;

		myThread = SC_CreateThread(threadProps, this, &SR_RenderThread::Execute_Internal);
	}
	void SR_RenderThread::Stop()
	{
		myIsRunning = false;
	}

	void SR_RenderThread::Init_Internal()
	{
		SC_ThreadTools::IsRenderThread = true;
	}
	
	void SR_RenderThread::Execute_Internal()
	{
		Init_Internal();
		while (myIsRunning)
		{
			myPool->WaitForWork();
			myPool->ResetHasWorkSignal();
			for (;;)
			{
				std::function<void()> func;
				if (myPool->PopTask(func))
					func();
				else
					break;
			}
		}
	}
}