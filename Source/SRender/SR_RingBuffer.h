#pragma once
#include "SR_GraphicsResource.h"
#include <deque>

namespace Shift
{

#	define DEFAULT_ALIGN 256

	class SR_RingBuffer: public SR_Resource
	{
	public:
		using OffsetType = uint64;
		struct FrameTailAttributes
		{
			FrameTailAttributes(uint64 aFenceValue, OffsetType aOffset, OffsetType aSize) :
				myFenceValue(aFenceValue),
				myOffset(aOffset),
				mySize(aSize)
			{}
			// Fence value associated with the command list in which
			// the allocation could have been referenced last time
			uint64 myFenceValue;
			OffsetType myOffset;
			OffsetType mySize;
		};
		static const OffsetType InvalidOffset = static_cast<OffsetType>(-1);

		SR_RingBuffer(OffsetType aMaxSize) noexcept;
		SR_RingBuffer(SR_RingBuffer&& rhs) noexcept;
		SR_RingBuffer& operator= (SR_RingBuffer&& rhs) noexcept;

		SR_RingBuffer(const SR_RingBuffer&) = delete;
		SR_RingBuffer& operator = (const SR_RingBuffer&) = delete;

		virtual ~SR_RingBuffer();

		OffsetType Allocate(OffsetType aSize);

		void FinishCurrentFrame(uint64 aFenceValue);
		void ReleaseCompletedFrames(uint64 aCompletedFenceValue);

		OffsetType GetMaxSize()const { return myMaxSize; }
		bool IsFull()const { return myUsedSize == myMaxSize; };
		bool IsEmpty()const { return myUsedSize == 0; };
		OffsetType GetUsedSize()const { return myUsedSize; }

	protected:
		std::deque< FrameTailAttributes > myCompletedFrameTails;
		OffsetType myHead;
		OffsetType myTail;
		OffsetType myMaxSize;
		OffsetType myUsedSize;
		OffsetType myCurrFrameSize;
	};

	struct SR_TempAllocation
	{
		SR_TempAllocation(SC_Handle aBuffer, uint64 aOffset, uint64 aSize)
			: myBuffer(aBuffer)
			, myOffset(aOffset)
			, mySize(aSize) 
			, myCPUAddress(0)
			, myGPUAddress(0)
		{}

		SC_Handle myBuffer;
		uint64 myOffset;
		uint64 mySize;
		void* myCPUAddress;
		uint64 myGPUAddress;
	};






	class SR_RingBuffer2
	{
	public:

		struct FrameData
		{
			SR_Fence myFence;
			uint64 myFrame;
			uint myFirstOffset;
		};

	public:
		SR_RingBuffer2(SR_Buffer* aBuffer = nullptr, uint aAlignment = 1);
		SR_RingBuffer2(SR_RingBuffer2&& aOther);
		~SR_RingBuffer2();

		SR_RingBuffer2& operator=(SR_RingBuffer2&& aOther);

		bool Allocate(uint& aAllocationOffsetOut, uint aSize, SR_GraphicsContext* aCtx, const SR_Fence& aFence, uint aAlignment = 0);
		void UpdateFrame(SR_GraphicsContext* aCtx, bool aAllocFlag, const SR_Fence& aFence);
		void Update();

		uint GetUsedCount() const { return (myFreeOffset + mySize - myUsedOffset) % mySize; }

		SC_Ref<SR_Buffer> myBuffer;
		SC_DynamicCircularArray<FrameData> myPendingFrames;
		uint mySize;
		uint myMaxAllocSize;
		uint myUsedOffset;
		uint myFreeOffset;
		uint myCurrentOffsetBase;
		uint myAlignment;

		uint64 myLastFinishedFrameIndex;
		uint64 myLastUpdatedFrame;
		uint64 myLastAllocationFrame;
		SR_Fence myLastAllocationFence;

	private:
		SR_RingBuffer2(const SR_RingBuffer2&) {}
		void operator=(const SR_RingBuffer2&) {}

		void Recenter();
	};
	SC_ALLOW_RELOCATE(SR_RingBuffer2);
}