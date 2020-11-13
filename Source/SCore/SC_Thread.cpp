#include "SCore_Precompiled.h"
#include "SC_Thread.h"

namespace Shift
{
	void SC_Thread_ThreadStarter(SC_ThreadStarterArgType aThread);

	struct SC_MainThreadProxy : SC_Thread
	{
		virtual void Run() {}
	};

	static thread_local SC_Thread* CurrentThread;
	static SC_MainThreadProxy locMainThreadProxy;

	SC_Thread* SC_Thread::GetCurrent()
	{
		if (!CurrentThread)
		{
			assert("IsMainThread");
			CurrentThread = &locMainThreadProxy;
			return CurrentThread;
		}

		return CurrentThread;
	}

	SC_ThreadHandle SC_Thread::Start(SC_Thread* aThread, const SC_ThreadProperties& aProperties)
	{
		aThread->myProperties = aProperties;
		aThread->myIsRunning = true;
		aThread->myThreadHandle = SC_Thread::StartThread(static_cast<void*>(aThread));
		return aThread->myThreadHandle;
	}

	SC_Semaphore* SC_Thread::GetCurrentThreadSemaphore()
	{
		if (CurrentThread)
			return &CurrentThread->mySemaphore;

		if (SC_ThreadTools::IsMainThread)
			return &locMainThreadProxy.mySemaphore; 

		return nullptr;
	}

	void SC_Thread::CloseThreadHandle(SC_ThreadHandle aThreadHandle)
	{
#if IS_WINDOWS
		CloseHandle(aThreadHandle);
#endif
	}

	SC_Thread::SC_Thread()
		: myThreadHandle()
		, myThreadID(0)
		, myIsRunning(false)
		, myShouldAutoDelete(false)
		, myShouldCloseHandleOnDelete(true)

	{
	}

	SC_Thread::~SC_Thread()
	{
		assert(!myIsRunning);

		if (myShouldCloseHandleOnDelete && myThreadHandle != SC_ThreadInvalidHandle)
		{
			CloseThreadHandle(myThreadHandle);
		}

	}

	bool SC_Thread::Start(const SC_ThreadProperties& aProperties)
	{
		return SC_Thread::Start(this, aProperties) != SC_ThreadInvalidHandle;
	}

	void SC_Thread::Stop()
	{
	}

#if IS_WINDOWS
#include <process.h>

	extern int Win64ThreadPrios[SC_ThreadPriority_COUNT];
	static uint __stdcall SC_Thread_ThreadStarterImpl(SC_ThreadStarterArgType anArgument)
	{
		SC_Thread_ThreadStarter(anArgument);
		return 0;
	}

#if IS_PC_PLATFORM
	void SC_SetThreadName_Win64(SC_ThreadHandle aThreadHandle, const char* aName);
#endif

	SC_ThreadHandle SC_Thread::StartThread(void* aArg)
	{
		SC_Thread* thread = (SC_Thread*)aArg;
		SC_ThreadHandle threadHandle;

#if IS_PC_PLATFORM
		threadHandle = (SC_ThreadHandle)_beginthreadex(NULL, (1 * MB), SC_Thread_ThreadStarterImpl, aArg, CREATE_SUSPENDED | STACK_SIZE_PARAM_IS_A_RESERVATION, NULL);

		SC_SetThreadName_Win64(threadHandle, thread->myProperties.myName.c_str());
#endif 

		::SetThreadPriority(threadHandle, Win64ThreadPrios[thread->myProperties.myPriority]);
		SC_ThreadTools::SetThreadAffinityMask(threadHandle, thread->myProperties.myAffinity);

		::ResumeThread(threadHandle);
		return threadHandle;
	}

	void SC_Thread::Wait(SC_ThreadHandle aThreadHandle)
	{
		::WaitForSingleObjectEx(aThreadHandle, INFINITE, FALSE);
	}

#elif IS_LINUX
	SC_ThreadHandle SC_Thread::StartThread(void* aArg) { return (SC_ThreadHandle)0; }
	void SC_Thread::Wait(SC_ThreadHandle aThreadHandle) {}
#elif IS_PS4_PLATFORM
	SC_ThreadHandle SC_Thread::StartThread(void* aArg) { return (SC_ThreadHandle)0; }
	void SC_Thread::Wait(SC_ThreadHandle aThreadHandle) {}
#endif

	void SC_Thread_ThreadStarter(SC_ThreadStarterArgType aThread)
	{
		//const int threadInd = SC_ThreadTools::GetCurrentThreadIndex();
		SC_Thread* thread = reinterpret_cast<SC_Thread*>(aThread);
		assert(!SC_ThreadTools::IsMainThread);

		CurrentThread = thread;

		uint32 seed = uint(uintptr_t(time(0)) + uintptr_t(thread));
		srand(seed);
		SC_SRand(seed);

		{
			thread->Run();
			thread->myIsRunning = false;
		}

		SC_RandCleanup();

		CurrentThread = nullptr; 
		if (thread->myShouldAutoDelete)
			delete thread;
	}
}