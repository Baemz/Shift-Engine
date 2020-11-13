#pragma once
namespace Shift
{
#if IS_WINDOWS
	class SC_MutexImpl_Win64
	{
	protected:
		friend class SC_Mutex;
		SC_MutexImpl_Win64(DWORD aSpinLockCount);
		~SC_MutexImpl_Win64();

		void Lock();
		bool TryLock();
		void Unlock();

		CRITICAL_SECTION myCritical;
	};

	typedef SC_MutexImpl_Win64 SC_MutexImpl;
#endif
}