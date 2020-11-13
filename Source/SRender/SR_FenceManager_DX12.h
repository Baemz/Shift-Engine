#pragma once
#if ENABLE_DX12

#include "SR_Fence.h"

struct ID3D12Fence;
namespace Shift
{
	class SR_GraphicsContext_DX12;
	class SR_FenceManager_DX12
	{
		friend class SR_QueueManager_DX12;
	public:
		SR_FenceManager_DX12();
		~SR_FenceManager_DX12();

		SR_Fence InsertFence(SR_GraphicsContext_DX12* aCtx);
		bool IsPending(const SR_Fence& aFence);
		bool Wait(const SR_Fence& aFence);

		void InsertWait(SR_GraphicsContext_DX12* aCtx, const SR_Fence& aFence);

		SR_Fence GetLastInsertedFence(const SR_ContextType& aContext) const;
		SR_Fence GetLastWaitedFence(const SR_ContextType& aSrcContext, const SR_ContextType& aWaitContext) const;

	private:

		void InsertWaitInternal(SR_GraphicsContext_DX12* aCtx, const SR_ContextType& aContext, uint64 aFenceValue, bool aExecContext);

		struct PerContextFenceData
		{
			ID3D12Fence* myFence;
			volatile uint64 myLastKnownCompleted;
			volatile uint64 myLastInserted;
			uint64 myLastWaited[SR_ContextType_COUNT];

			bool IsPending(uint64 aValue);
		};

		PerContextFenceData myFenceData[SR_ContextType_COUNT];
		HANDLE myFenceHandles[SR_ContextType_COUNT];

	};
}

#endif