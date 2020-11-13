#include "SCore_Precompiled.h"
#include "SC_Mutex.h"

namespace Shift
{
	SC_Mutex::SC_Mutex(int aSpinLockCount)
		: myMutex(aSpinLockCount)
	{
		myLockedCount = 0;
	}
	void SC_Mutex::Lock()
	{
		if (!myMutex.TryLock())
		{
			// Track locks?
			myMutex.Lock();
		}

		++myLockedCount;
	}
	bool SC_Mutex::TryLock()
	{
		if (!myMutex.TryLock())
			return false;

		++myLockedCount;
		return true;
	}
	void SC_Mutex::Unlock()
	{
		assert(myLockedCount > 0);
		--myLockedCount;

		myMutex.Unlock();
	}
	uint SC_Mutex::GetLockedCount()
	{
		return myLockedCount;
	}
}