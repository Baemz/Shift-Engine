#pragma once
#include "SR_WaitEvent.h"
#include "SR_TextureResource.h"
#include "SR_RenderEnums.h"
#include "SR_Fence.h"

namespace Shift
{
	class SR_Texture;
	class SC_Window;

	struct SR_RenderInitParams
	{
		SR_GraphicsAPI myAPI;
		SC_Window* myWindow;
	};

	class SR_RenderInterface
	{
	public:

		static bool Init(const SR_RenderInitParams& aInitParams);
		static void Destroy();

		static void BeginFrame();
		static void EndFrame();

		static SC_Ref<SR_Waitable> PostRenderTask(std::function<void()> aTask, uint aWaitMode = SR_WaitableMode_CPU_GPU);
		static SC_Ref<SR_Waitable> PostComputeTask(std::function<void()> aTask, uint aWaitMode = SR_WaitableMode_CPU_GPU);
		static SC_Ref<SR_Waitable> PostCopyTask(std::function<void()> aTask, uint aWaitMode = SR_WaitableMode_CPU_GPU);
		static SC_Ref<SR_Waitable> PostPresentTask();

	public:
		static SC_Ref<SR_Waitable> ourPresentEvent;
		static SC_Ref<SR_Texture> ourBlack4x4;
		static SC_Ref<SR_Texture> ourWhite4x4;
		static SC_Ref<SR_Texture> ourGrey4x4;
		static SC_Ref<SR_Texture> ourNormal4x4;
		static SR_Fence ourLastViewTaskFence;
		static SR_Fence ourLastFence;
	};
}
