#pragma once
#include "SR_Fence.h"

#if ENABLE_DX12
namespace Shift
{
	struct SR_ResourceTracking_DX12
	{
		SR_ResourceTracking_DX12();

		uint myState;
		uint16 myReadState;
		uint16 myFirstWriteState;
		SR_Fence myFence;
		SR_ContextType myFenceContext;
	};
}
#endif