#pragma once
#include "SR_QueueManager.h"

namespace Shift
{
	struct SR_RenderThread_Private;
	class SR_RenderThreadPool;
	class SR_RenderThread
	{ 
		friend class SR_RenderThreadPool;
	public:
		~SR_RenderThread();

	private:
		SR_RenderThread(SR_RenderThreadPool* aPool);

		void Start(const char* aThreadName);
		void Stop();
		void Init_Internal();
		void Execute_Internal();

		SR_RenderThreadPool* myPool;
		SC_Future<void> myThread;
		std::atomic_bool myIsRunning;
	};

}