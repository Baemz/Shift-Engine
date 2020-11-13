#include "SCore_Precompiled.h"
#include "SC_MutexImpl.h"

namespace Shift
{
#if IS_WINDOWS
	SC_MutexImpl_Win64::SC_MutexImpl_Win64(DWORD aSpinLockCount)
	{
		InitializeCriticalSectionEx(&myCritical, aSpinLockCount, 0);
	}

	SC_MutexImpl_Win64::~SC_MutexImpl_Win64()
	{
		DeleteCriticalSection(&myCritical);
	}
	void SC_MutexImpl_Win64::Lock()
	{
		EnterCriticalSection(&myCritical);
	}
	bool SC_MutexImpl_Win64::TryLock()
	{
		return (TryEnterCriticalSection(&myCritical) != 0);
	}
	void SC_MutexImpl_Win64::Unlock()
	{
		LeaveCriticalSection(&myCritical);
	}
#endif
}