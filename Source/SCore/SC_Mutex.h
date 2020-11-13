#pragma once
#include "SC_MutexImpl.h"
#include "SC_AutoLock.h"

namespace Shift
{
	class SC_Mutex
	{
	public:
		SC_Mutex(int aSpinLockCount = 0);
		SC_Mutex(const SC_Mutex&) = delete;
		SC_Mutex& operator= (const SC_Mutex&) = delete;

		void Lock();
		bool TryLock();
		void Unlock();

		uint GetLockedCount();

	private:
		SC_MutexImpl myMutex;
		uint myLockedCount;
	};


	typedef SC_AutoLock<SC_Mutex> SC_MutexLock;

	#define SC_MutexLock(args) __Incorrect_Usage__
}