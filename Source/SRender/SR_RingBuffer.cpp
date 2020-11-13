#include "SRender_Precompiled.h"
#include "SR_RingBuffer.h"

namespace Shift
{
	SR_RingBuffer::SR_RingBuffer(OffsetType aMaxSize) noexcept
		: myMaxSize(aMaxSize)
		, myTail(0)
		, myHead(0)
		, myUsedSize(0)
		, myCurrFrameSize(0)
	{
	}
	SR_RingBuffer::SR_RingBuffer(SR_RingBuffer&& rhs) noexcept
		: myCompletedFrameTails(std::move(rhs.myCompletedFrameTails))
		, myTail(rhs.myTail)
		, myHead(rhs.myHead)
		, myMaxSize(rhs.myMaxSize)
		, myUsedSize(rhs.myUsedSize)
		, myCurrFrameSize(rhs.myCurrFrameSize)
	{
		rhs.myTail = 0;
		rhs.myHead = 0;
		rhs.myMaxSize = 0;
		rhs.myUsedSize = 0;
		rhs.myCurrFrameSize = 0;
	}
	SR_RingBuffer& SR_RingBuffer::operator=(SR_RingBuffer&& rhs) noexcept
	{
		myCompletedFrameTails = std::move(rhs.myCompletedFrameTails);
		myTail = rhs.myTail;
		myHead = rhs.myHead;
		myMaxSize = rhs.myMaxSize;
		myUsedSize = rhs.myUsedSize;
		myCurrFrameSize = rhs.myCurrFrameSize;

		rhs.myMaxSize = 0;
		rhs.myTail = 0;
		rhs.myHead = 0;
		rhs.myUsedSize = 0;
		rhs.myCurrFrameSize = 0;

		return *this;
	}

	SR_RingBuffer::~SR_RingBuffer()
	{
		assert(myUsedSize == 0 && "All space in the ring buffer must be released");
	}

	SR_RingBuffer::OffsetType SR_RingBuffer::Allocate(OffsetType aSize)
	{
		if (IsFull())
		{
			return InvalidOffset;
		}

		if (myTail >= myHead)
		{
			if (myTail + aSize <= myMaxSize)
			{
				auto Offset = myTail;
				myTail += aSize;
				myUsedSize += aSize;
				myCurrFrameSize += aSize;
				return Offset;
			}
			else if (aSize <= myHead)
			{
				// Allocate from the beginning of the buffer
				OffsetType AddSize = (myMaxSize - myTail) + aSize;
				myUsedSize += AddSize;
				myCurrFrameSize += AddSize;
				myTail = aSize;
				return 0;
			}
		}
		else if (myTail + aSize <= myHead)
		{
			auto Offset = myTail;
			myTail += aSize;
			myUsedSize += aSize;
			myCurrFrameSize += aSize;
			return Offset;
		}

		return InvalidOffset;
	}

	void SR_RingBuffer::FinishCurrentFrame(uint64 aFenceValue)
	{
		myCompletedFrameTails.emplace_back(aFenceValue, myTail, myCurrFrameSize);
		myCurrFrameSize = 0;
	}
	void SR_RingBuffer::ReleaseCompletedFrames(uint64 aCompletedFenceValue)
	{
		while (!myCompletedFrameTails.empty() &&
			myCompletedFrameTails.front().myFenceValue <= aCompletedFenceValue)
		{
			const auto &OldestFrameTail = myCompletedFrameTails.front();
			assert(OldestFrameTail.mySize <= myUsedSize);
			myUsedSize -= OldestFrameTail.mySize;
			myHead = OldestFrameTail.myOffset;
			myCompletedFrameTails.pop_front();
		}
	}



	SR_RingBuffer2::SR_RingBuffer2(SR_Buffer* aBuffer, uint aAlignment)
		: myBuffer(aBuffer)
		, mySize(aBuffer ? aBuffer->GetProperties().mySize : 0)
		, myMaxAllocSize(aBuffer ? (aBuffer->GetProperties().mySize / 4) : 0)
		, myUsedOffset(mySize)
		, myFreeOffset(0)
		, myCurrentOffsetBase(0)
		, myAlignment(aAlignment)
		, myLastFinishedFrameIndex(SC_UINT64_MAX)
		, myLastUpdatedFrame(SC_UINT64_MAX)
		, myLastAllocationFrame(SC_UINT64_MAX)
	{
		if (myBuffer)
		{
			assert(SC_IsPowerOfTwo(aAlignment));
		}
	}

	SR_RingBuffer2::SR_RingBuffer2(SR_RingBuffer2&& aOther)
		: myBuffer(SC_Move(aOther.myBuffer))
		, myPendingFrames(SC_Move(aOther.myPendingFrames))
		, mySize(aOther.mySize)
		, myMaxAllocSize(aOther.myMaxAllocSize)
		, myUsedOffset(aOther.myUsedOffset)
		, myFreeOffset(aOther.myFreeOffset)
		, myCurrentOffsetBase(aOther.myCurrentOffsetBase)
		, myAlignment(aOther.myAlignment)
		, myLastFinishedFrameIndex(aOther.myLastFinishedFrameIndex)
		, myLastUpdatedFrame(aOther.myLastUpdatedFrame)
		, myLastAllocationFrame(aOther.myLastAllocationFrame)
		, myLastAllocationFence(aOther.myLastAllocationFence)
	{

	}

	SR_RingBuffer2& SR_RingBuffer2::operator=(SR_RingBuffer2&& aOther)
	{
		assert(this != &aOther);
		this->~SR_RingBuffer2();
		new (this) SR_RingBuffer2(SC_Move(aOther));
		return *this;
	}

	SR_RingBuffer2::~SR_RingBuffer2()
	{

	}

	bool SR_RingBuffer2::Allocate(uint& aAllocationOffsetOut, uint aSize, SR_GraphicsContext* aCtx, const SR_Fence& aFence, uint aAlignment)
	{
		assert((aSize & (myAlignment-1)) == 0);

		if (!aSize)
		{
			aAllocationOffsetOut = 0;
			return true;
		}

		uint size = aSize;
		uint maxMisalignment = 0;
		if (aAlignment > 1)
		{
			assert(SC_IsPowerOfTwo(aAlignment));
			if (aAlignment > myAlignment)
			{
				maxMisalignment = aAlignment - myAlignment;
				size = aSize + maxMisalignment;
			}
		}
		if (size > myMaxAllocSize)
		{
			assert(myMaxAllocSize || !mySize);
			return false;
		}

		UpdateFrame(aCtx, false, aFence);

		if (SR_GraphicsDevice::ourLastCompletedFrame != myLastFinishedFrameIndex)
			Update();

		uint bytesLeft = myUsedOffset - myFreeOffset;
		if (size > bytesLeft)
			return false;

		if (myCurrentOffsetBase + myFreeOffset + size > mySize && myCurrentOffsetBase + myFreeOffset <= mySize)
		{
			bytesLeft = myUsedOffset - mySize + myCurrentOffsetBase;
			if (size > bytesLeft)
				return false;

			myFreeOffset += mySize - (myCurrentOffsetBase + myFreeOffset);
		}

		UpdateFrame(aCtx, true, aFence);

		aAllocationOffsetOut = (myCurrentOffsetBase + myFreeOffset) % mySize;

		if (maxMisalignment)
			aAllocationOffsetOut = SC_Align(aAllocationOffsetOut, aAlignment);

		myFreeOffset += size;
		return true;
	}

	void SR_RingBuffer2::UpdateFrame(SR_GraphicsContext* aCtx, bool aAllocFlag, const SR_Fence& aFence)
	{
		uint64 frameIndex = (aCtx && SC_ThreadTools::IsRenderThread) ? SC_Timer::GetRenderFrameIndex() : SC_Timer::GetGlobalFrameIndex();
		if (frameIndex != myLastUpdatedFrame || (aAllocFlag && (myLastAllocationFrame != frameIndex || myLastAllocationFence != aFence)))
		{
			FrameData lastFrame;
			lastFrame.myFence = myLastAllocationFence;
			lastFrame.myFrame = myLastUpdatedFrame != SC_UINT64_MAX ? myLastUpdatedFrame : 0;
			lastFrame.myFirstOffset = myFreeOffset;

			myPendingFrames.Add(lastFrame);

			Recenter();
			myLastUpdatedFrame = frameIndex;
			if (aAllocFlag)
			{
				myLastAllocationFrame = frameIndex;
				myLastAllocationFence = aFence;
			}
		}
	}

	void SR_RingBuffer2::Update()
	{
		uint64 lastFinishedFrame = SR_GraphicsDevice::ourLastCompletedFrame;

		while (myPendingFrames.Count())
		{
			FrameData& frame = myPendingFrames.Peek();
			if (frame.myFrame <= lastFinishedFrame && !(frame.myFence.myFenceValue && SR_QueueManager::Get()->IsPending(frame.myFence)))
			{
				myUsedOffset += frame.myFirstOffset;
				myPendingFrames.Remove();
			}
			else
				break;
		}

		myLastFinishedFrameIndex = lastFinishedFrame;
	}

	void SR_RingBuffer2::Recenter()
	{
		myCurrentOffsetBase += myFreeOffset;
		myUsedOffset -= myFreeOffset;
		myFreeOffset = 0;
		myCurrentOffsetBase = myCurrentOffsetBase % mySize;
	}

}
