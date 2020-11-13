#include "SRender_Precompiled.h"
#include "SR_RenderInterface.h"
#include "SR_SwapChain.h"
#include "SR_WaitEvent.h"
#include "SR_CommandList.h"

#if ENABLE_DX12
#include "SR_GraphicsDevice_DX12.h"
#endif
#if ENABLE_VULKAN
#include "SR_GraphicsDevice_Vk.h"
#endif
#if ENABLE_PS4
#include "SR_GraphicsDevice_PS4.h"
#endif
namespace Shift
{
	SC_Ref<SR_Waitable> SR_RenderInterface::ourPresentEvent;
	SC_Ref<SR_Texture> SR_RenderInterface::ourBlack4x4;
	SC_Ref<SR_Texture> SR_RenderInterface::ourWhite4x4;
	SC_Ref<SR_Texture> SR_RenderInterface::ourGrey4x4;
	SC_Ref<SR_Texture> SR_RenderInterface::ourNormal4x4;

	SR_Fence SR_RenderInterface::ourLastViewTaskFence;
	SR_Fence SR_RenderInterface::ourLastFence;

	bool SR_RenderInterface::Init(const SR_RenderInitParams& aInitParams)
	{
		SR_GraphicsDevice* device = nullptr;

		switch (aInitParams.myAPI)
		{
#if ENABLE_VULKAN
		case SR_GraphicsAPI::Vulkan:
			device = new SR_GraphicsDevice_Vk();
			break;
#endif
#if ENABLE_PS4
		case SR_GraphicsAPI::PS4:
			//device = new SR_GraphicsDevice_PS4();
			break;
#endif
#if ENABLE_DX12
		case SR_GraphicsAPI::DirectX12:
			device = new SR_GraphicsDevice_DX12();
			break;
#endif
		default:
			assert(false && "Graphics API not supported!");
		}

		if (!device->Init(aInitParams.myWindow))
			return false;

		if (!SR_ImGUI::Init(aInitParams.myWindow, device))
			return false;

		ourPresentEvent = new SR_Waitable(SR_WaitableMode_CPU);
		ourPresentEvent->myEventCPU.Signal();

		return true;
	}

	void SR_RenderInterface::Destroy()
	{
		SR_ImGUI::Shutdown();
		SR_GraphicsDevice::Destroy();
	}

	void SR_RenderInterface::BeginFrame()
	{
		SR_GraphicsContext* ctx = SR_GraphicsDevice::GetDevice()->GetContext(SR_ContextType_Render);
		ctx->Begin();
		ctx->BeginRecording();
	}

	void SR_RenderInterface::EndFrame()
	{
		SR_GraphicsContext* ctx = SR_GraphicsContext::GetCurrent();
		
		{
			SR_PROFILER_FUNCTION_TAGGED("End Frame");
			SR_SwapChain* swapChain = SR_GraphicsDevice::GetDevice()->GetSwapChain();
			ctx->Transition(SR_ResourceState_Present, swapChain->GetCurrentTarget()->GetTextureBuffer());
		}
		ctx->EndRecording();

		SC_GrowingArray<SR_CommandList*> cls;
		cls.Add(ctx->GetCommandList());
		SR_Fence fence = ctx->GetCurrentFence();
		SR_QueueManager::ExecuteCommandLists(cls, SR_ContextType_Render, fence.myFenceValue);
		ctx->End();

		ourLastFence.myFenceValue = fence.myFenceValue;
		ourLastFence.myContextType = fence.myContextType;
	}

	SC_Ref<SR_Waitable> SR_RenderInterface::PostRenderTask(std::function<void()> aTask, uint aWaitMode)
	{
		return SR_GraphicsDevice::GetDevice()->PostRenderTask(aTask, aWaitMode);
	}
	SC_Ref<SR_Waitable> SR_RenderInterface::PostComputeTask(std::function<void()> aTask, uint aWaitMode)
	{
		return SR_GraphicsDevice::GetDevice()->PostComputeTask(aTask, aWaitMode);
	}
	SC_Ref<SR_Waitable> SR_RenderInterface::PostCopyTask(std::function<void()> aTask, uint aWaitMode)
	{
		return SR_GraphicsDevice::GetDevice()->PostCopyTask(aTask, aWaitMode);
	}

	SC_Ref<SR_Waitable> SR_RenderInterface::PostPresentTask()
	{
		SR_Fence lastViewTaskFence = ourLastViewTaskFence;
		SR_Fence lastFence = ourLastFence;
		auto present = [lastViewTaskFence, lastFence]()
		{
			SR_GraphicsDevice* device = SR_GraphicsDevice::GetDevice();
			SR_QueueManager::Get()->Wait(lastViewTaskFence);
			SR_QueueManager::Get()->Wait(lastFence);

			device->Present();

			device->EndFrame();
			device->ourLastCompletedFrame = SC_Timer::GetRenderFrameIndex();
			SC_Timer::IncrementRenderFrameCount();
		};
		ourPresentEvent = SR_GraphicsDevice::GetDevice()->PostRenderTask(present, SR_WaitableMode_CPU);
		return ourPresentEvent;
	}
}