#pragma once
#include "SR_QueueManager.h"
#include "SR_FenceManager_DX12.h"

#if ENABLE_DX12

struct ID3D12CommandQueue;
namespace Shift
{
	class SR_CommandList_DX12;
	class SR_CommandListPool_DX12;
	class SR_QueueManager_DX12 : public SR_QueueManager
	{
	public:
		SR_QueueManager_DX12();
		~SR_QueueManager_DX12();

		ID3D12CommandQueue* GetQueue(const SR_ContextType& aType) const;

		void Close(SR_CommandList* aCmdList) override;
		void Wait(const SR_Fence& aFence) override;
		SR_Fence GetNextExpectedFence(const SR_ContextType& aType) const override;
		uint64 GetTimestampFreq(const SR_ContextType& aType) const override; 
		bool IsPending(const SR_Fence& aFence) override;
		
		SR_FenceManager_DX12& GetFenceManager() { return myFenceManager; }

		static SR_QueueManager_DX12* GetInstance() { return static_cast<SR_QueueManager_DX12*>(ourInstance); }

	private:
		void UpdateCmdListPoolFunc();
		void Init_Internal() override;
		bool Execute_Internal(QueueData2& aTask) override;
		void UpdateCommandListPool_Internal() override; 

		void NativeSignal(const SR_Fence& aFence) override;
		void NativeWait(const SR_Fence& aFence, const SR_ContextType& aWaitingQueue) override;

		void NativeBeginEvent(const SR_ContextType& aQueue) override;
		void NativeEndEvent(const SR_ContextType& aQueue) override;

		SC_Ref<SR_CommandList> GetCommandList_Internal(SR_ContextType aContextType) override;

		bool CloseCommandList(SR_CommandList_DX12* aCmdList);

		SR_FenceManager_DX12 myFenceManager;

		SC_FutureBase myUpdateCmdListPoolThread;
		SC_FastEvent myHasUpdateWork;
		SC_Mutex myPendingCloseMutex;
		SC_GrowingArray<SC_Ref<SR_CommandList_DX12>> myPendingCloseCmd;
		SC_GrowingArray<SC_Ref<SR_CommandList_DX12>> myPendingCloseCmdTemp;

		SC_Ref<SR_CommandListPool_DX12> myCmdListPools[SR_ContextType::SR_ContextType_COUNT];
		ID3D12CommandQueue* myQueues[SR_ContextType_COUNT];

		volatile bool mySignalUpperQueue;
	};

}

#endif