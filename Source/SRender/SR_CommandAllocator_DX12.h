#pragma once
#if ENABLE_DX12
#include "SC_RefCounted.h"

struct ID3D12CommandAllocator;

namespace Shift
{
	class SR_Fence_DX12;
	class SR_CommandListPool_DX12;
	class SR_CommandAllocator_DX12 : public SC_RefCounted
	{
		friend class SR_CommandList_DX12;
		friend class SR_CommandListPool_DX12;
		friend class SR_QueueManager_DX12;
	public:
		SR_CommandAllocator_DX12(D3D12_COMMAND_LIST_TYPE aType, SR_CommandListPool_DX12* aPool);
		~SR_CommandAllocator_DX12();

		bool BeginCommandList();
		void Reset();

		bool IsFilled() const;
		bool IsIdle();
	private:
		volatile uint64 myLastFrameOpened;
		volatile float myLastTimeOpened;

		D3D12_COMMAND_LIST_TYPE myType;
		ID3D12CommandAllocator* myAllocator;
		SR_CommandListPool_DX12* myPool; 
		SR_QueueManager_DX12* myQueueManager;

		volatile uint myCommandListsActive;
		volatile uint myCommandListsAlive;
		volatile uint myCommandsSinceResetCount;
		uint myCommandsPerResetLimit;

		SC_Mutex myMutex;
		SR_Fence myFence;
	};
}
#endif
