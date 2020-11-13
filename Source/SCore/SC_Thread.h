#pragma once
#include "SC_ThreadTools.h"
#include "SC_Event.h"
#include "SC_Semaphore.h"

namespace Shift
{
	using SC_ThreadStarterArgType = void*;

	struct SC_ThreadProperties
	{
		SC_ThreadProperties(const char* aName = "Unnamed thread" , SC_ThreadPriority aPriority = SC_ThreadPriority_Medium, SC_ThreadAffinity aAffinity = SC_ThreadTools::ourDefaultThreadAffinity)
			: myName(aName)
			, myPriority(aPriority)
			, myAffinity(aAffinity)
		{
		}
		std::string myName;
		SC_ThreadPriority myPriority;
		SC_ThreadAffinity myAffinity;
	};

	class SC_Thread
	{
	public:
		static SC_Thread* GetCurrent();
		static SC_ThreadHandle Start(SC_Thread* aThread, const SC_ThreadProperties& aProperties);
		static SC_Semaphore* GetCurrentThreadSemaphore();

		static void	CloseThreadHandle(SC_ThreadHandle aThreadHandle);
		static void Wait(SC_ThreadHandle aThreadHandle);

		SC_Thread();

		bool Start(const SC_ThreadProperties& aProperties);
		virtual void Stop();

		SC_ThreadHandle	GetHandle()	const { return myThreadHandle; }
		SC_ThreadID	GetThreadId() const { return myThreadID; }

		SC_Event myStopRequest;
		SC_Semaphore mySemaphore;
	protected:
		virtual ~SC_Thread();
		virtual void Run() = 0;

		bool myIsRunning;
		bool myShouldAutoDelete;
		bool myShouldCloseHandleOnDelete;

	private:
		static SC_ThreadHandle StartThread(void* aArg);

		SC_ThreadProperties myProperties;
		SC_ThreadHandle myThreadHandle;
		SC_ThreadID myThreadID;

		friend void SC_Thread_ThreadStarter(SC_ThreadStarterArgType aThread);
	};
}