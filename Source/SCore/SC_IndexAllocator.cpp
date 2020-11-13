#include "SCore_Precompiled.h"
#include "SC_IndexAllocator.h"

namespace Shift
{

	SC_IndexAllocator::SC_IndexAllocator()
		: myLastAllocated(0)
		, myNumAvailable(0)
		, myMaxUsed(0)
		, myNumUsedIndices(0)
	{

	}

	void SC_IndexAllocator::Init(uint aCount)
	{
		SC_ASSERT(!myFreeBits.Count());
		SC_ASSERT(!(aCount & 63));

		uint numIndices = aCount / 64;
		myFreeBits.AddN(uint64(-1), numIndices);
		myNumAvailable = numIndices;
	}

	uint SC_IndexAllocator::Reserve(uint aCount)
	{
		SC_ASSERT(!(aCount & 63));

		uint numIndices = aCount / 64;
		SC_ASSERT(numIndices < aCount);

		uint fistIndex = myNumAvailable - numIndices;
		myNumAvailable = fistIndex;

#if ENABLE_ASSERTS
		for (uint i = fistIndex, e = fistIndex + numIndices; i != e; ++i)
			assert(myFreeBits[i] == uint64(-1));
#endif

		return fistIndex * 64;
	}

	uint SC_IndexAllocator::Allocate()
	{
		uint allocIndex = myLastAllocated;
		uint numIndexSteps = 0;

		for (;;)
		{
			uint64& chunk = myFreeBits[allocIndex];
			uint64 oldBits = chunk;

			if (oldBits)
			{
				int bitIndex = SC_FindFirstBit(oldBits);
				if (bitIndex >= 0)
				{
					uint64 bit = 1ULL << uint(bitIndex);
					uint64 newBits = oldBits ^ bit;
					SC_ASSERT(oldBits != newBits);

					if (SC_Atomic_CompareExchange(chunk, newBits, oldBits))
					{
						myLastAllocated = allocIndex;
						++myNumUsedIndices;
						return allocIndex * 64 + bitIndex;
					}
					else
						continue;
				}
			}

			// if we get here there are no free bits in the current chunk - move on to the next one

			SC_ASSERT(numIndexSteps++ < myNumAvailable, "looped through all chunks without finding a free bit - check for leaks and increase size if none are found");

			allocIndex = (allocIndex + 1) % myNumAvailable;

			for (;;)
			{
				uint oldMax = myMaxUsed;
				if (SC_Atomic_CompareExchange(myMaxUsed, SC_Max(oldMax, allocIndex), oldMax))
					break;
			}
		}
	}

	void SC_IndexAllocator::Free(uint aIndex)
	{
		uint chunkIndex = aIndex / 64;
		uint bitIndex = aIndex & 63;
		uint64 bit = 1ULL << bitIndex;

		uint64& chunk = myFreeBits[chunkIndex];

		uint64 oldBits = SC_Atomic_Or(chunk, bit);
		SC_VERIFY(!(oldBits & bit));
		--myNumUsedIndices;
	}

}