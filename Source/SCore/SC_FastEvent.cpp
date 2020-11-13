#include "SCore_Precompiled.h"
#include "SC_FastEvent.h"
#include "SC_Semaphore.h"

namespace Shift
{
	SC_FastEvent::SC_FastEvent()
		: myState(0)
	{
	}

	SC_FastEvent::~SC_FastEvent()
	{
		while (myState & SC_FastEventFlag_Locked)
			std::this_thread::yield();

		assert(myWaitingSemaphores.Count() == 0);
	}

	void SC_FastEvent::Signal()
	{
		for (;;)
		{
			int prevState = SC_Atomic_CompareExchange_GetOld(myState, SC_FastEventFlag_Signalled | SC_FastEventFlag_Locked, 0);

			if (prevState & SC_FastEventFlag_Signalled)
				return;
			else if (prevState == 0)
				break;
		}

		for (uint i = 0, count = myWaitingSemaphores.Count(); i < count; ++i)
			myWaitingSemaphores[i]->Release();
		myWaitingSemaphores.RemoveAll();

		int prev = SC_Atomic_Exchange_GetOld(myState, SC_FastEventFlag_Signalled);
		assert(prev == (SC_FastEventFlag_Signalled | SC_FastEventFlag_Locked));
		SC_UNUSED(prev);
	}

	void SC_FastEvent::Wait()
	{
		if (SC_Semaphore* semaphore = RegisterWait())
			semaphore->Acquire();
	}

	void SC_FastEvent::ResetSignal()
	{
		for (;;)
		{
			int prevState = SC_Atomic_CompareExchange_GetOld(myState, 0, SC_FastEventFlag_Signalled);
			if ((prevState & SC_FastEventFlag_Signalled) == 0 || prevState == SC_FastEventFlag_Signalled)
				return;
		}
	}

	bool SC_FastEvent::IsSignalled() const
	{
		int state = myState;
		return (state & SC_FastEventFlag_Signalled) != 0;
	}

	SC_Semaphore* SC_FastEvent::RegisterWait()
	{
		for (;;)
		{
			int prevState = SC_Atomic_CompareExchange_GetOld(myState, SC_FastEventFlag_Locked, 0);

			if (prevState & SC_FastEventFlag_Signalled)
				return nullptr;
			else if (prevState == 0)
				break;
		}

		SC_Thread* thread = SC_Thread::GetCurrent();
		SC_Semaphore& semaphore = thread->mySemaphore;

		myWaitingSemaphores.Add(&semaphore);

		int prev = SC_Atomic_Exchange_GetOld(myState, 0);
		assert(prev == (SC_FastEventFlag_Locked));
		SC_UNUSED(prev);

		return &semaphore;
	}
}