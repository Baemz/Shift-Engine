#include "SCore_Precompiled.h"
#include "SC_Memory.h"

#include <new>

namespace Shift
{
#if SC_DEBUG_MEMORY_ALLOCS
	SC_Mutex SC_Memory::ourStatsLock;
	SC_Memory::Allocations* SC_Memory::ourAllocations;
#endif

	void SC_Memory::Setup()
	{
#if SC_DEBUG_MEMORY_ALLOCS
		if (ourAllocations == nullptr)
			ourAllocations = new SC_Memory::Allocations();
#endif
	}

	void SC_Memory::Cleanup()
	{
#if SC_DEBUG_MEMORY_ALLOCS
		SC_Memory::Allocations* allocs = ourAllocations;
		ourAllocations = nullptr;
		delete allocs;
#endif
	}

	void* SC_Memory::Allocate(uint64 aSize, bool /*aIsArray*/)
	{

		void* p = malloc(aSize);
		if (p == nullptr)
		{
			assert(false && "failed to allocate from heap");
			return nullptr;
		}
#if SC_DEBUG_MEMORY_ALLOCS
		{
			if (ourAllocations)
			{
				SC_MutexLock lock(ourStatsLock);
				ourAllocations->Add(p, aSize);
			}
		}
#endif
		return p;
	}

	void SC_Memory::Deallocate(void* aPointer, bool /*aIsArray*/)
	{
#if SC_DEBUG_MEMORY_ALLOCS
		{
			if (ourAllocations)
			{
				SC_MutexLock lock(ourStatsLock);
				ourAllocations->Remove(aPointer);
			}
		}
#endif
		free(aPointer);
	}

	uint64 SC_Memory::GetMemoryUsage()
	{
#if SC_DEBUG_MEMORY_ALLOCS
		if (!ourAllocations)
			return 0;

		SC_MutexLock lock(ourStatsLock);
		return ourAllocations->GetMemoryUsage();
#else
		return 0;
#endif
	}

#if SC_DEBUG_MEMORY_ALLOCS
	SC_Memory::Allocations::Allocations()
		: myCurrentBufferSize(100)
		, myCurrentUsedSize(0)
	{
		myAllocations = (void**)calloc(myCurrentBufferSize, sizeof(void*));
		myAllocationSizes = (uint64*)calloc(myCurrentBufferSize, sizeof(uint64));
	}

	SC_Memory::Allocations::~Allocations()
	{
		free(myAllocations);
		free(myAllocationSizes);
	}

	void SC_Memory::Allocations::Add(void* aPtr, uint64 aSize)
	{
		if (myCurrentUsedSize == myCurrentBufferSize)
			Grow();

		if (myCurrentUsedSize < myCurrentBufferSize)
		{
			myAllocations[myCurrentUsedSize] = aPtr;
			myAllocationSizes[myCurrentUsedSize] = aSize;
			myCurrentUsedSize++;
		}
	}

	void SC_Memory::Allocations::Remove(void* aPtr)
	{
		for (int64 i = myCurrentUsedSize - 1; i >= 0; --i)
		{
			if (myAllocations[i] == aPtr)
			{
				SC_Memmove(&myAllocations[i] , &myAllocations[myCurrentUsedSize - 1], sizeof(void*));
				SC_Memmove(&myAllocationSizes[i], &myAllocationSizes[myCurrentUsedSize - 1], sizeof(uint64));
				--myCurrentUsedSize;
				return;
			}
		}
	}

	uint64 SC_Memory::Allocations::GetMemoryUsage() const
	{
		uint64 size = 0;
		for (uint64 i = 0; i < myCurrentUsedSize; ++i)
			size += myAllocationSizes[i];

		return size;
	}

	void SC_Memory::Allocations::Grow()
	{
		uint64 oldSize = myCurrentBufferSize;
		myCurrentBufferSize = oldSize * 2;

		void** oldAllocs = myAllocations;
		uint64* oldSizes = myAllocationSizes;

		myAllocations = (void**)calloc(myCurrentBufferSize, sizeof(void*));
		myAllocationSizes = (uint64*)calloc(myCurrentBufferSize, sizeof(uint64));

		if (oldAllocs)
		{
			SC_Memmove(myAllocations, oldAllocs, sizeof(void*) * myCurrentUsedSize);
			SC_Memmove(myAllocationSizes, oldSizes, sizeof(uint64) * myCurrentUsedSize);
			free(oldAllocs);
			free(oldSizes);
		}
	}
#endif
}