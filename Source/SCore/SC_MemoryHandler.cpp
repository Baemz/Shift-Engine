#include "SCore_Precompiled.h"
#include "SC_MemoryHandler.h"

namespace Shift
{
	SC_MemoryHandler* SC_MemoryHandler::ourInstance = nullptr;

	SC_MemoryHandler::SC_MemoryHandler()
		: myLastUsedFreeBlock(0)
		, myUsedSize(0)
	{
		myMemory = new SMemoryBlock[MemoryPoolSize];
		ZeroMemory(myMemory, sizeof(SMemoryBlock) * MemoryPoolSize);
		myCurrentIndex = myMemory;
		myUsedBlocks = new std::atomic_bool[MemoryPoolSize];
		ZeroMemory((void*)myUsedBlocks, sizeof(std::atomic_bool) * MemoryPoolSize);
		myAllocations.PreAllocate(16);
		myFreeFragments.PreAllocate(16);
	}

	SC_MemoryHandler::SMemoryBlock* SC_MemoryHandler::GetFragmentIfAvailable(uint aMemoryBlocksNeeded)
	{
		SMemoryBlock* memoryBlockPtr = nullptr;
		for (uint i = 0, end = myFreeFragments.Count(); (i < end); ++i)
		{
			SBlockDesc blockDesc;
			SBlockDesc& freeBlock = myFreeFragments[i];
			if (freeBlock.myNumBlocks <= aMemoryBlocksNeeded)
			{ 
				blockDesc.myBlockIndex = freeBlock.myBlockIndex;
				// If allocated size is less than the existing size in the block
				// we need to split the block and update the "free" block.
				memoryBlockPtr = &myMemory[freeBlock.myBlockIndex];

				for (uint block = freeBlock.myBlockIndex; block < (freeBlock.myBlockIndex + aMemoryBlocksNeeded); ++block)
				{
					std::atomic_exchange(&myUsedBlocks[block], true);
				}

				if (freeBlock.myNumBlocks < aMemoryBlocksNeeded)
				{
					freeBlock.myBlockIndex += aMemoryBlocksNeeded;
					freeBlock.myNumBlocks -= aMemoryBlocksNeeded;
				}
				else
					myFreeFragments.RemoveByIndexCyclic(i);

				blockDesc.myNumBlocks = aMemoryBlocksNeeded;
				blockDesc.myExternalAddress = memoryBlockPtr;
				myAllocations.Add(blockDesc);
				break;
			}
		}
		return memoryBlockPtr;
	}


	bool SC_MemoryHandler::Create()
	{
		assert((!ourInstance) && "Instance already created.");

		ourInstance = new SC_MemoryHandler();

		return (ourInstance);
	}

	void SC_MemoryHandler::Destroy()
	{
		SC_MemoryHandler* handler = ourInstance;
		ourInstance = nullptr;
		delete[] (handler->myMemory);
		delete[] (handler->myUsedBlocks);
		delete (handler);
	}

	SC_MemoryHandler::~SC_MemoryHandler()
	{
	}
	void* SC_MemoryHandler::Allocate(unsigned int aSize)
	{
		std::unique_lock<std::mutex> lock(myAllocateLock);
		const uint numBlocksNeeded = static_cast<uint>(SC_Ceil(aSize/(float)MemoryPoolBlockSize));
		SMemoryBlock* currentStartPtr = GetFragmentIfAvailable(numBlocksNeeded);
		if (currentStartPtr)
		{
			myUsedSize += aSize;
			return currentStartPtr;
		}

		int startIndex = -1;
		uint numFreeBlocksFound = 0;
		for (uint block = myLastUsedFreeBlock; block < MemoryPoolSize; ++block)
		{
			if (!myUsedBlocks[block])
			{
				if (startIndex == -1)
				{
					startIndex = block;
				}
				++numFreeBlocksFound;
				if (numFreeBlocksFound == numBlocksNeeded)
				{
					for (uint i = startIndex; i < (startIndex + numBlocksNeeded); ++i)
						std::atomic_exchange(&myUsedBlocks[i], true);

					myLastUsedFreeBlock = block;
					SBlockDesc blockDesc;
					blockDesc.myBlockIndex = startIndex;
					blockDesc.myNumBlocks = numBlocksNeeded;
					blockDesc.myExternalAddress = (void*)&myMemory[block];
					myAllocations.Add(blockDesc);
					myUsedSize += aSize;
					return blockDesc.myExternalAddress;
				}
			}
			else
			{
				startIndex = -1;
				numFreeBlocksFound = 0;
			}
		}

		return nullptr;
	}
	void SC_MemoryHandler::Deallocate(void* aPointer, bool /*aIsArray*/)
	{
		std::unique_lock<std::mutex> lock(myAllocateLock);
		for (uint i = 0; i < myAllocations.Count(); ++i)
		{
			SBlockDesc& allocation = myAllocations[i];
			if (allocation == aPointer)
			{
				ZeroMemory(allocation.myExternalAddress, allocation.myNumBlocks * MemoryPoolBlockSize);
				
				for (uint j = allocation.myBlockIndex; j < (allocation.myBlockIndex + allocation.myNumBlocks); ++j)
					std::atomic_exchange(&myUsedBlocks[j], false);

				allocation.myExternalAddress = nullptr;
				myFreeFragments.Add(allocation);
				myUsedSize -= allocation.myNumBlocks * MemoryPoolBlockSize;
				myAllocations.RemoveByIndexCyclic(i);
				break;
			}
		}
	}
	void SC_MemoryHandler::Defrag()
	{
	}
	const float SC_MemoryHandler::GetUsedMemoryAmount() const
	{
		return (float)(myUsedSize * MultiplierMB);
	}
}