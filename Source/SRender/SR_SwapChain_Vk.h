#pragma once
#include "SR_SwapChain.h"
#include "SR_ForwardDeclarations_Vk.h"

#if ENABLE_VULKAN

namespace Shift
{
	class SR_SwapChain_Vk final : public SR_SwapChain
	{
	public:
		SR_SwapChain_Vk();
		~SR_SwapChain_Vk();

		bool Init(const SR_SwapChainDesc& aDesc) override;
		void Present(bool aSync = false) override;

	private:
		void GetBackbufferTextures();

		SC_GrowingArray<VkImage> myBackbufferTextures;
		VkSwapchainKHR mySwapChain;
		VkDevice myDevice;
		SC_Vector2f myResolution;
		SR_Format myFormat;
	};
}

#endif

