#pragma once
#include "SR_QueueManager.h"

#if ENABLE_VULKAN
#include "SR_ForwardDeclarations_Vk.h"

namespace Shift
{
	class SR_CommandList;
	struct SR_Fence;
	class SR_QueueManager_Vk : public SR_QueueManager
	{
	public:
		SR_QueueManager_Vk();
		~SR_QueueManager_Vk();

		void Close(SR_CommandList* aCmdList) override;
		void Wait(const SR_Fence& aFence) override;
		SR_Fence GetNextExpectedFence(const SR_ContextType& aType) const override;
		uint64 GetTimestampFreq(const SR_ContextType& aType) const override;
		bool IsPending(const SR_Fence& aFence) override;

		static SR_QueueManager_Vk* GetInstance() { return static_cast<SR_QueueManager_Vk*>(ourInstance); }

	private:
		void Init_Internal() override;
		bool Execute_Internal(QueueData2& aTask) override;
		void UpdateCommandListPool_Internal() override;

		void NativeSignal(const SR_Fence& aFence) override;
		void NativeWait(const SR_Fence& aFence, const SR_ContextType& aWaitingQueue) override;

		void NativeBeginEvent(const SR_ContextType& aQueue) override;
		void NativeEndEvent(const SR_ContextType& aQueue) override;

		SC_Ref<SR_CommandList> GetCommandList_Internal(SR_ContextType aContextType) override;

		VkQueue myQueues[SR_ContextType_COUNT];
	};
}
#endif
