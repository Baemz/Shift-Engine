#pragma once
#include "SR_GraphicsEngineEnums.h"

namespace Shift
{
	struct SR_Fence
	{
		SR_Fence() : myFenceValue(0), myContextType(SR_ContextType_Render) {}
		SR_Fence(uint64 aValue, SR_ContextType aCtxType) : myFenceValue(aValue), myContextType(aCtxType) {}

		bool operator==(const SR_Fence& aOther) const
		{
			if (myFenceValue == aOther.myFenceValue &&
				myContextType == aOther.myContextType)
				return true;

			return false;
		}

		uint64 myFenceValue;
		SR_ContextType myContextType;
	};
}