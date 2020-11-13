#pragma once
#include "SR_Texture.h"

#if ENABLE_VULKAN
#include "SR_ForwardDeclarations_Vk.h"

namespace Shift
{
	class SR_Texture_Vk final : public SR_Texture
	{
		friend class SR_GraphicsDevice_Vk;
		friend class SR_GraphicsContext_Vk;
	public:
		SR_Texture_Vk();
		~SR_Texture_Vk();

	private:
		VkImageView myVkImageView;
	};
}
#endif

