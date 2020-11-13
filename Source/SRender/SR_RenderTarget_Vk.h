#pragma once
#include "SR_RenderTarget.h"
#if ENABLE_VULKAN

namespace Shift
{
	class SR_RenderTarget_Vk final : public SR_RenderTarget
	{
		friend class SR_GraphicsDevice_Vk;
		friend class SR_GraphicsContext_Vk;
	public:
		SR_RenderTarget_Vk();
		~SR_RenderTarget_Vk();

	private:
		VkImageView myVkImageView;
	};
}

#endif