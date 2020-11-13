#pragma once
#include "SR_RenderEnums.h"

namespace Shift
{
	class SR_CommandList : public SR_Resource
	{
	public:
		SR_CommandList(uint8 aInitialState, const SR_ContextType& aContextType);
		virtual ~SR_CommandList();

		enum RecordingState : uint8
		{
			Recording = 0x80,
			Idle = 0x40,
			Closing = 0x20,
			BlockingExecute = 0x1f, 
		};

		virtual void FlushPendingTransitions() = 0;
		virtual void UpdateResourceGlobalStates() = 0;

		void SetLastFence(const SR_Fence& aFence);
		const SR_Fence& GetFence() const { return myFence; }

		volatile uint8 myState;
		SR_CommandList* myNext;
		SR_CommandList* myPrevious;
		SR_ContextType myContextType;

		SC_HashMap<SR_TrackedResource*, SC_Pair<uint, uint>> myPendingTransitions;
		bool myNeedsExecute;

		bool operator<(const SR_CommandList& aOther) 
		{ 
			return myFence.myFenceValue < aOther.myFence.myFenceValue; 
		}

	protected:
		SR_Fence myFence;
	};
}

