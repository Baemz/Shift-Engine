#include "SCore_Precompiled.h"
#include "SC_RefCounted.h"

//#define DEBUG_PRINT_DELETE

namespace Shift
{
	SC_RefCounted::SC_RefCounted()
		: myRefCount(InitValue)
	{
	}

	SC_RefCounted::~SC_RefCounted()
	{
	}

	void SC_RefCounted::AddRef()
	{
		for (;;)
		{
			uint count = myRefCount;
			assert(count > 0);
			assert(count == InitValue || count < InitValue - 1);
			uint newCount = count + 1;
			if (count == InitValue)
				newCount = 1;
			if (SC_Atomic_CompareExchange(myRefCount, newCount, count))
				return;
		}
	}

	void SC_RefCounted::RemoveRef()
	{
		for (;;)
		{
			uint count = myRefCount;
			assert(count > 0 && count != InitValue);
			if (count == 1)
			{
				if (this->CanRelease(myRefCount))
				{
#ifdef DEBUG_PRINT_DELETE
					SC_LOG("Deleted ref (%p)", this);
#endif
					delete this;
				}
				return;
			}

			if (SC_Atomic_CompareExchange(myRefCount, count - 1, count))
				return;
		}
	}

	uint SC_RefCounted::GetRefCount() const
	{
		return myRefCount;
	}

	bool SC_RefCounted::CanRelease(volatile uint& aRefCount)
	{
		return (SC_Atomic_Decrement_GetNew(aRefCount) == 0);
	}
}