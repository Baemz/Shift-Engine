#pragma once
#include "SR_CommandList.h"
#if ENABLE_DX12
struct ID3D12GraphicsCommandList;

#if ENABLE_DX12_20H1
struct ID3D12GraphicsCommandList4;
#endif

namespace Shift
{
	class SR_QueueManager_DX12;
	class SR_CommandAllocator_DX12;
	class SR_CommandListPool_DX12;
	class SR_Fence_DX12;
	class SR_CommandList_DX12 final : public SR_CommandList
	{
		friend class SR_GraphicsDevice_DX12;
		friend class SR_GraphicsContext_DX12;
		friend class SR_CommandListPool_DX12;
		friend class SR_QueueManager_DX12;
	public:
		SR_CommandList_DX12(D3D12_COMMAND_LIST_TYPE aType, SR_CommandAllocator_DX12* aCmdAllocator, SR_CommandListPool_DX12* aPool, const SR_ContextType& aContextType);
		~SR_CommandList_DX12();

		ID3D12GraphicsCommandList* GetCommandList() const;

#if SR_ENABLE_RAYTRACING
		ID3D12GraphicsCommandList4* GetCommandList4() const;
#endif
		void FinishRecording();
		bool Close(bool aIsAllowedToFail = false);

		void FlushPendingTransitions() override;
		void UpdateResourceGlobalStates() override;

		virtual bool CanRelease(volatile uint& aRefCount) override;

		uint myNumCommands;

	private:
		void Open(SR_CommandAllocator_DX12* aCmdAllocator);
		void BeginRecording(const SR_ContextType& aContextType);

		volatile uint64 myLastFrameOpened;
		volatile float myLastTimeOpened;

		SC_Ref<SR_CommandList_DX12> myTransitionCmdList;

		SR_ContextType myCurrentContextType;
		D3D12_COMMAND_LIST_TYPE myType;
		ID3D12GraphicsCommandList* myCommandList;

#if SR_ENABLE_RAYTRACING
		ID3D12GraphicsCommandList4* myCommandList4;
#endif
		SR_CommandAllocator_DX12* myAllocator; 
		SR_CommandListPool_DX12* myPool;
		SR_QueueManager_DX12* myQueueManager;
		uint myPoolId;
	};
}
#endif