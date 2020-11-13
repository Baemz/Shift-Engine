#pragma once
namespace Shift
{
	enum SC_FastEventFlag
	{
		SC_FastEventFlag_Locked = 1,
		SC_FastEventFlag_Signalled = 2,
	};

	class SC_Semaphore;
	class SC_FastEvent
	{
	public:
		SC_FastEvent();
		~SC_FastEvent();

		void Signal();
		void Wait();
		void ResetSignal();
		bool IsSignalled() const;

		SC_Semaphore* RegisterWait();

	private:
		volatile int myState;
		SC_GrowingArray<SC_Semaphore*> myWaitingSemaphores;
	};
}

