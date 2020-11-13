#pragma once
#include <bitset>
#include <atomic>
#include <mutex>
//#define new() Shift::CMemoryHandler::Allocate()

namespace Shift
{
	class SC_MemoryHandler
	{
	private:
		struct SMemoryBlock
		{
			uint32 size[4];
		};
	public:
		static bool Create();
		static void Destroy();
		inline static SC_MemoryHandler* GetInstance() { return ourInstance; }

		~SC_MemoryHandler();

		void* Allocate(unsigned int aSize);
		void Deallocate(void* aPointer, bool aIsArray);
		void Defrag();

		const float GetUsedMemoryAmount() const;

		static constexpr uint MemoryPoolBlockSize = sizeof(SMemoryBlock);
		static constexpr uint MemoryPoolSize = (1 * GB) / MemoryPoolBlockSize;
		static constexpr float MultiplierMB = (1.0f / MB);
	private:
		struct SBlockDesc
		{
			SBlockDesc()
				: myBlockIndex(0)
				, myNumBlocks(0)
				, myExternalAddress(nullptr)
			{}
			bool operator==(const SBlockDesc& aOther)
			{
				if (myBlockIndex == aOther.myBlockIndex &&
					myNumBlocks == aOther.myNumBlocks &&
					myExternalAddress == aOther.myExternalAddress)
					return true;

				return false;
			}
			bool operator==(const void* aOther)
			{
				if (myExternalAddress == aOther)
					return true;

				return false;
			}
			uint myBlockIndex;
			uint myNumBlocks;
			void* myExternalAddress;
		};

		SC_MemoryHandler();
		SMemoryBlock* GetFragmentIfAvailable(uint aMemoryBlocksNeeded);

		static SC_MemoryHandler* ourInstance;

		SC_GrowingArray<SBlockDesc> myFreeFragments;
		SC_GrowingArray<SBlockDesc> myAllocations;
		uint myLastUsedFreeBlock;
		uint myUsedSize;
		SMemoryBlock* myCurrentIndex;
		SMemoryBlock* myMemory;
		std::atomic_bool* myUsedBlocks;
		std::mutex myAllocateLock;
		std::mutex myDeallocateLock;
	};

}