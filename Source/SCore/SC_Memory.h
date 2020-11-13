#pragma once
#include "SC_Mutex.h"

#define SC_DEBUG_MEMORY_ALLOCS !IS_FINAL_BUILD

namespace Shift
{
	class SC_Memory
	{
	public:
		static void Setup();
		static void Cleanup();
		static void* Allocate(uint64 aSize, bool aIsArray);
		static void Deallocate(void* aPointer, bool aIsArray);

		static uint64 GetMemoryUsage();

	private:
#if SC_DEBUG_MEMORY_ALLOCS
		struct Allocations
		{
		public:
			Allocations();
			~Allocations();
			void Add(void* aPtr, uint64 aSize);
			void Remove(void* aPtr);

			uint64 GetMemoryUsage() const;

		private:
			void Grow();

			uint64 myCurrentBufferSize;
			uint64 myCurrentUsedSize;
			void** myAllocations;
			uint64* myAllocationSizes;
		};
		
		static Allocations* ourAllocations;
		static SC_Mutex ourStatsLock;
#endif
	};
}
