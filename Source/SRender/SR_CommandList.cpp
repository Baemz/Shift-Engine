#include "SRender_Precompiled.h"
#include "SR_CommandList.h"

namespace Shift
{
	SR_CommandList::SR_CommandList(uint8 aInitialState, const SR_ContextType& aContextType)
		: myState(aInitialState)
		, myNext(nullptr)
		, myPrevious(nullptr)
		, myContextType(aContextType)
		, myFence(0, aContextType)
	{
	}

	SR_CommandList::~SR_CommandList()
	{
		myPendingTransitions.Reset();
	}

	void SR_CommandList::SetLastFence(const SR_Fence& aFence)
	{
		assert(aFence.myFenceValue > 0);
		myFence = aFence;
	}
}
