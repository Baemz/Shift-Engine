#include "SRender_Precompiled.h"

#if ENABLE_DX12
#include "SR_QueueManager_DX12.h"

namespace Shift
{
	SR_ResourceTracking_DX12::SR_ResourceTracking_DX12()
		: myState(0)
		, myReadState(0)
		, myFirstWriteState(0)
		, myFenceContext(SR_ContextType_Render)
	{}
}
#endif