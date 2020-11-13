#pragma once

namespace Shift
{
#if IS_WINDOWS

	class SC_SemaphoreImpl_Win64 : public SC_Noncopyable
	{
	public:
		SC_SemaphoreImpl_Win64(long aStartCount = 0);
		~SC_SemaphoreImpl_Win64();

		bool Acquire();
		void Release(uint aCount = 1);
		bool TimedAcquire(uint aTimeoutPeriod);

		HANDLE mySemaphore;
	};

	typedef SC_SemaphoreImpl_Win64 SC_SemaphoreImpl;

#elif IS_LINUX
#	error NOT SUPPORTED YET!
#elif IS_PS4_PLATFORM
#	error NOT SUPPORTED YET!
#endif

	class SC_Semaphore
	{
	public:
		SC_Semaphore(long aStartCount = 0);

		void Acquire();
		void Release(uint aCount = 1);
		bool TimedAcquire(uint aTimeoutPeriod);

		SC_SemaphoreImpl mySemaphoreImpl;

	};

	// This semaphore references the unique semaphore for the current thread.
	class SC_CurrentThreadSemaphoreRef
	{
	public:
		SC_CurrentThreadSemaphoreRef();
		~SC_CurrentThreadSemaphoreRef();

		operator SC_Semaphore*() { return mySemaphoreImpl; }
		SC_Semaphore& operator*() { return *mySemaphoreImpl; }
		SC_Semaphore* operator->() { return mySemaphoreImpl; }
	private:
		SC_Semaphore* mySemaphoreImpl;
		bool myOwnsSemaphore : 1;
	};

}