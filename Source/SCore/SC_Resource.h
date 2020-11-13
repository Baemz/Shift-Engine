#pragma once
#include "SC_RefCounted.h"

namespace Shift
{
	enum SC_LoadState : uint8
	{
		Unloaded,
		Loading,
		Loaded
	};

	class SC_Resource : public SC_RefCounted
	{
	public:
		SC_Resource();
		virtual ~SC_Resource();

		void SetState(const SC_LoadState& aState) 
		{ 
			for (;;)
			{
				uint8 state = myState;
				if (SC_Atomic_CompareExchange(myState, aState, state))
					break;
			}
		}
		bool IsLoaded() const { return myState == Loaded; }
		bool IsUnloaded() const { return myState == Unloaded; }
		bool IsLoading() const { return myState == Loading; }

	protected:
		volatile uint8 myState;
	};
}