#include "SRender_Precompiled.h"
#include "SR_QueueManager_Vk.h"

#if ENABLE_VULKAN
#include "SR_CommandList.h"


namespace Shift
{

	SR_QueueManager_Vk::SR_QueueManager_Vk()
	{

	}

	SR_QueueManager_Vk::~SR_QueueManager_Vk()
	{

	}

	void SR_QueueManager_Vk::Close(SR_CommandList* /*aCmdList*/)
	{

	}

	void SR_QueueManager_Vk::Wait(const SR_Fence& /*aFence*/)
	{

	}

	SR_Fence SR_QueueManager_Vk::GetNextExpectedFence(const SR_ContextType& /*aType*/) const
	{
		return SR_Fence();
	}

	uint64 SR_QueueManager_Vk::GetTimestampFreq(const SR_ContextType& /*aType*/) const
	{
		return 0;
	}

	bool SR_QueueManager_Vk::IsPending(const SR_Fence& /*aFence*/)
	{
		return true;
	}

	void SR_QueueManager_Vk::Init_Internal()
	{

	}

	bool SR_QueueManager_Vk::Execute_Internal(QueueData2& /*aTask*/)
	{
		//VkQueue& queue = myQueues[aTask.myContextType];
		//VkFence fence;

		//VkSubmitInfo submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
		//submitInfo.waitSemaphoreCount = aCtxData.myPendingWaitSemaphores.CountU();
		//submitInfo.pWaitSemaphores = aCtxData.myPendingWaitSemaphores.GetBuffer();
		//submitInfo.pWaitDstStageMask = waitStages.GetBuffer();
		//submitInfo.commandBufferCount = aNumCommandBuffers;
		//submitInfo.pCommandBuffers = someCommandBuffers;
		//submitInfo.signalSemaphoreCount = signalSemaphores.CountU();
		//submitInfo.pSignalSemaphores = signalSemaphores.GetBuffer();

		//if (vkQueueSubmit(queue, 1, &submitInfo, fence) != VK_SUCCESS)
		//{
		//	SC_ERROR_LOG("Could not submit commands to queue");
			return false;
		//}
	}

	void SR_QueueManager_Vk::UpdateCommandListPool_Internal()
	{

	}

	void SR_QueueManager_Vk::NativeSignal(const SR_Fence& /*aFence*/)
	{
	}

	void SR_QueueManager_Vk::NativeWait(const SR_Fence& /*aFence*/, const SR_ContextType& /*aWaitingQueue*/)
	{

	}

	void SR_QueueManager_Vk::NativeBeginEvent(const SR_ContextType& /*aQueue*/)
	{

	}

	void SR_QueueManager_Vk::NativeEndEvent(const SR_ContextType& /*aQueue*/)
	{

	}

	SC_Ref<SR_CommandList> SR_QueueManager_Vk::GetCommandList_Internal(SR_ContextType /*aContextType*/)
	{
		return nullptr;
	}

}
#endif