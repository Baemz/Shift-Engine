#include "SCore_Precompiled.h"
#include "SC_ThreadTools.h"

#include <sstream>

namespace Shift
{
	const SC_ThreadAffinity SC_ThreadTools::ourDefaultThreadAffinity	= SC_ThreadAffinity(-1);
	const SC_ThreadAffinity SC_ThreadTools::ourMainThreadAffinity		= SC_ThreadAffinity(-1);
	const SC_ThreadAffinity SC_ThreadTools::ourCriticalThreadAffinity	= SC_ThreadAffinity(-1);
	const SC_ThreadAffinity SC_ThreadTools::ourRenderThreadAffinity		= SC_ThreadAffinity(-1);
	const SC_ThreadAffinity SC_ThreadTools::ourLongTaskThreadAffinity	= SC_ThreadAffinity(-1);
	const SC_ThreadAffinity SC_ThreadTools::ourFrameTaskThreadAffinity	= SC_ThreadAffinity(-1);

	thread_local bool SC_ThreadTools::IsMainThread;
	thread_local bool SC_ThreadTools::IsRenderThread;
	thread_local bool SC_ThreadTools::IsFrameTaskThread;
	thread_local bool SC_ThreadTools::IsTextureStreamerThread;
	thread_local bool SC_ThreadTools::IsMeshStreamerThread;

#if IS_WINDOWS

#if IS_PC_PLATFORM
	void SC_SetThreadName_Win64(SC_ThreadHandle aThreadHandle, const char* aName)
	{
		typedef HRESULT(WINAPI* SetThreadDescriptionType)(HANDLE aThread, PCWSTR aThreadDescription);
		static SetThreadDescriptionType setThreadDescriptionFunc = (SetThreadDescriptionType)GetProcAddress(GetModuleHandle(L"Kernel32.dll"), "SetThreadDescription");
		if (setThreadDescriptionFunc)
			setThreadDescriptionFunc(aThreadHandle, ToWString(aName).c_str());
	}
#endif

	void SC_Sleep_NonProfiled(uint aMilliseconds)
	{
		if (aMilliseconds == 0)
			SC_Yield_NonProfiled();
		else
			::Sleep(aMilliseconds);
	}

	void SC_Yield_NonProfiled()
	{
		::SwitchToThread();
	}

	int Win64ThreadPrios[] =
	{
		THREAD_PRIORITY_TIME_CRITICAL,
		THREAD_PRIORITY_HIGHEST,
		THREAD_PRIORITY_ABOVE_NORMAL,
		THREAD_PRIORITY_NORMAL,
		THREAD_PRIORITY_LOWEST
	};
	static_assert(_countof(Win64ThreadPrios) == SC_ThreadPriority_COUNT);
	void SC_ThreadTools::SetCurrentThreadPriority(SC_ThreadPriority aPriority)
	{
		SetThreadPriority(GetCurrentThread(), Win64ThreadPrios[aPriority]);
	}

	SC_ThreadPriority SC_ThreadTools::GetCurrentThreadPriority()
	{
		int prio = ::GetThreadPriority(GetCurrentThread());
		for (uint i = 0; i < SC_ThreadPriority_COUNT - 1; i++)
			if (prio > Win64ThreadPrios[i + 1])
				return (SC_ThreadPriority)i;

		return SC_ThreadPriority_Low;
	}

#elif IS_LINUX
	void SC_Sleep_NonProfiled(uint aMilliseconds){}
	void SC_Yield_NonProfiled(){}
	void SC_ThreadTools::SetCurrentThreadPriority(SC_ThreadPriority aPriority) {}
	SC_ThreadPriority SC_ThreadTools::GetCurrentThreadPriority() {}
#elif IS_PS4_PLATFORM
	void SC_Sleep_NonProfiled(uint aMilliseconds){}
	void SC_Yield_NonProfiled(){}
	void SC_ThreadTools::SetCurrentThreadPriority(SC_ThreadPriority aPriority) {}
	SC_ThreadPriority SC_ThreadTools::GetCurrentThreadPriority() {}
#endif

	void SC_Sleep(uint aMilliseconds)
	{
		// TODO: Add profile markers here
		SC_Sleep_NonProfiled(aMilliseconds);
	}

	void SC_Yield()
	{
		// TODO: Add profile markers here
		SC_Yield_NonProfiled();
	}

	void SC_ThreadTools::RegisterMainThread()
	{
		SC_ThreadTools::IsMainThread = true;
		SC_ThreadTools::SetCurrentThreadName("Main");

		uint32 seed = uint(uintptr_t(time(0)));
		srand(seed);
		SC_SRand(seed);
	}

	uint SC_ThreadTools::GetLogicalThreadCount()
	{
#if IS_PC_PLATFORM
		static uint cachedLogicalThreadCount = SC_UINT_MAX;
		if (cachedLogicalThreadCount != SC_UINT_MAX)
			return cachedLogicalThreadCount;

		SYSTEM_INFO sysInfo;
		GetSystemInfo(&sysInfo);

		cachedLogicalThreadCount = sysInfo.dwNumberOfProcessors;
		cachedLogicalThreadCount = SC_Clamp(cachedLogicalThreadCount, 1u, (uint)SC_MAX_NUM_CORES);
		return cachedLogicalThreadCount;

#elif IS_PS4_PLATFORM || IS_XBOX_PLATFORM
		return SC_MAX_NUM_CORES;
#else
#error PLATFORM NOT SUPPORTED!
#endif
	}

	void SC_ThreadTools::SetCurrentThreadName(const char* aName)
	{
#if IS_WINDOWS
#if !IS_FINAL_BUILD

#if IS_PC_PLATFORM
		SC_SetThreadName_Win64(GetCurrentThread(), aName);
#endif // IS_PC_PLATFORM

#pragma pack(push,8)
	typedef struct tagTHREADNAME_INFO
	{
		DWORD dwType; // must be 0x1000
		LPCSTR szName; // pointer to name (in user addr space)
		DWORD dwThreadID; // thread ID (-1=caller thread)
		DWORD dwFlags; // reserved for future use, must be zero
	} THREADNAME_INFO;
#pragma pack(pop)

		THREADNAME_INFO info;
		info.dwType = 0x1000;
		info.szName = aName;
		info.dwThreadID = ULONG_MAX;
		info.dwFlags = 0;

		__try 
		{
			RaiseException(0x406D1388, 0, sizeof(info) / sizeof(ULONG_PTR), (ULONG_PTR*)&info);
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
		}
#endif // !IS_FINAL_BUILD
#elif IS_LINUX
#elif IS_PS4_PLATFORM
#endif
		SC_UNUSED(aName);
	}

	SC_ThreadID SC_ThreadTools::GetCurrentThreadId()
	{
#if IS_WINDOWS
		return ::GetCurrentThreadId();
#elif IS_LINUX
		return pthread_self();
#elif IS_PS4_PLATFORM
		return (uint)0;
#else
#error "Unknown platform!"
#endif
	}

	bool SC_ThreadTools::SetThreadAffinityMask(SC_ThreadHandle aThread, SC_ThreadAffinity aAffinity)
	{
		assert(aThread != SC_ThreadInvalidHandle);

#if IS_WINDOWS
		DWORD_PTR processAffinity = 0;
		DWORD_PTR systemAffinity = 0;
		bool ok = ::GetProcessAffinityMask(GetCurrentProcess(), &processAffinity, &systemAffinity) != FALSE;
		if (!ok || !processAffinity)
		{
			SC_ERROR_LOG("Could not get process affinity mask");
			return false;
		}

		DWORD_PTR newAffinity = aAffinity & processAffinity;
		if (newAffinity != aAffinity && newAffinity != processAffinity)
		{
			SC_ERROR_LOG("Requested thread affinity (%p) is not compatible with current process affinity (%p)", DWORD_PTR(aAffinity), processAffinity);
			return false;
		}
		
		DWORD_PTR prevAffinity = ::SetThreadAffinityMask(aThread, newAffinity);
		assert(prevAffinity != 0);
		return prevAffinity != 0;

#elif IS_PS4_PLATFORM
		bool result = false;
		return result;
#else
		return false;
#endif
	}

}

