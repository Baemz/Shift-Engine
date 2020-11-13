#include "SCore_Precompiled.h"
#include "SC_Semaphore.h"
#include "SC_Thread.h"

namespace Shift
{

#if IS_WINDOWS

	SC_SemaphoreImpl_Win64::SC_SemaphoreImpl_Win64(long aStartCount)
	{
		mySemaphore = ::CreateSemaphoreExW(NULL, aStartCount, SC_INT_MAX, NULL, 0, SEMAPHORE_ALL_ACCESS);
		assert(mySemaphore != NULL);
	}

	SC_SemaphoreImpl_Win64::~SC_SemaphoreImpl_Win64()
	{
		CloseHandle(mySemaphore);
	}

	bool SC_SemaphoreImpl_Win64::Acquire()
	{
		bool result = (WaitForSingleObjectEx(mySemaphore, INFINITE, FALSE) != WAIT_FAILED);
		assert(result);
		return result;
	}

	void SC_SemaphoreImpl_Win64::Release(uint aCount)
	{
		ReleaseSemaphore(mySemaphore, aCount, NULL);
	}

	bool SC_SemaphoreImpl_Win64::TimedAcquire(uint aTimeoutPeriod)
	{
		bool outResult = false;

		DWORD res = WaitForSingleObjectEx(mySemaphore, aTimeoutPeriod, FALSE);
		if (res == WAIT_OBJECT_0)
			outResult = true;
		else if (res == WAIT_TIMEOUT)
			outResult = false;
		else
			assert(false && "Wait for semaphore failed");

		return outResult;
	}

#elif IS_LINUX
#	error NOT SUPPORTED YET!
#elif IS_PS4_PLATFORM
#	error NOT SUPPORTED YET!
#endif

	SC_Semaphore::SC_Semaphore(long aStartCount)
		: mySemaphoreImpl(aStartCount)
	{
	}

	void SC_Semaphore::Acquire()
	{
		if (mySemaphoreImpl.TimedAcquire(0))
			return;

		bool outResult = mySemaphoreImpl.Acquire();
		assert(outResult);
	}

	void SC_Semaphore::Release(uint aCount)
	{
		mySemaphoreImpl.Release(aCount);
	}

	bool SC_Semaphore::TimedAcquire(uint aTimeoutPeriod)
	{
		if (mySemaphoreImpl.TimedAcquire(0))
			return true;

		if (!aTimeoutPeriod)
			return false;

		bool outResult = mySemaphoreImpl.TimedAcquire(aTimeoutPeriod);
		return outResult;
	}

	SC_CurrentThreadSemaphoreRef::SC_CurrentThreadSemaphoreRef()
	{
		mySemaphoreImpl = SC_Thread::GetCurrentThreadSemaphore();
		myOwnsSemaphore = !mySemaphoreImpl;

		if (myOwnsSemaphore)
			mySemaphoreImpl = new SC_Semaphore();
	}

	SC_CurrentThreadSemaphoreRef::~SC_CurrentThreadSemaphoreRef()
	{
		if (myOwnsSemaphore)
			delete mySemaphoreImpl;
	}
}