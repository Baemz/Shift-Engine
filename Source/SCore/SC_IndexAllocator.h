#pragma once
#include "SC_GrowingArray.h"

namespace Shift
{
	class SC_IndexAllocator
	{
	public:
		SC_IndexAllocator();

		void Init(uint aCount);
		uint Reserve(uint aCount);

		uint Allocate();
		void Free(uint aIndex);

		uint GetUsedCount() const { return myNumUsedIndices; }

	private:
		SC_GrowingArray<uint64> myFreeBits;
		uint myLastAllocated;
		uint myNumAvailable;
		uint myMaxUsed;
		uint myNumUsedIndices;
	};

}