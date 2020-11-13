#pragma once
#include "SC_Atomics.h"

namespace Shift
{
	template<int DefaultValue>
	struct SC_DirtyFlag
	{
		volatile int myValue;
		explicit SC_DirtyFlag(int aValue = DefaultValue) : myValue(aValue) {}
		SC_DirtyFlag(const SC_DirtyFlag&) : myValue(DefaultValue) {}
		SC_DirtyFlag& operator=(const SC_DirtyFlag&) { myValue = DefaultValue; return *this; }

		bool IsDirty()
		{
			for (;;)
			{
				int dirty = myValue;
				if (!dirty)
					return false;

				if (SC_Atomic_CompareExchange(myValue, 2, 1))
					return true;
			}
		}
	};
}