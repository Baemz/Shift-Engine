#pragma once
#include "SC_RefCounted.h"
#include "SC_FastEvent.h"
#include "SR_RenderEnums.h"

namespace Shift
{
	enum SR_WaitMode : uint
	{
		SR_WaitMode_CPU = 0x1,
		SR_WaitMode_GPU = 0x2,
	};

	enum SR_WaitableMode : uint8
	{
		SR_WaitableMode_CPU,
		SR_WaitableMode_CPU_GPU,
	};

	class SR_Waitable : public SC_RefCounted
	{
	public:
		explicit SR_Waitable(SR_WaitableMode aWaitableMode) : myWaitableMode(aWaitableMode), myFence(0) {}

		SC_FastEvent myEventCPU;

		uint64 myFence;
		SR_ContextType myFenceContext;
		SR_WaitableMode myWaitableMode;
	};


}