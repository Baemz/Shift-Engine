#pragma once
#include "SC_Misc.h"

namespace Shift
{
	template<class Mutex>
	class SC_AutoLock : SC_Noncopyable
	{
	public:
		SC_AutoLock() : myMutex(nullptr) {}
		SC_AutoLock(Mutex& aMutex) : myMutex(&aMutex) { myMutex->Lock(); }
		SC_AutoLock(SC_AutoLock&& aLock) : myMutex(aLock.myMutex) { aLock.myMutex = nullptr; }
		~SC_AutoLock() { Unlock(); }

		SC_FORCEINLINE Mutex* GetMutex() const { return myMutex; }

		SC_FORCEINLINE bool IsLocked() const { return myMutex != nullptr; }
		SC_FORCEINLINE bool IsLocked(const Mutex& aMutex) const { return myMutex == &aMutex; }

		SC_FORCEINLINE void Lock(Mutex& aMutex)
		{
			assert(myMutex == nullptr);
			myMutex = &aMutex;
			myMutex->Lock();
		}

		SC_FORCEINLINE bool TryLock(Mutex& aMutex)
		{
			assert(myMutex == nullptr);
			if (aMutex.TryLock())
			{
				myMutex = &aMutex;
				return true;
			}
			return false;
		}

		SC_FORCEINLINE void Unlock()
		{
			if (!myMutex)
				return;

			myMutex->Unlock();
			myMutex = nullptr;
		}

	private:
		Mutex* myMutex;
	};
}