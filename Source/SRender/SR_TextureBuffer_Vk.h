#pragma once
#include "SR_TextureBuffer.h"

#if ENABLE_VULKAN
#include "SR_ForwardDeclarations_Vk.h"

namespace Shift
{
	class SR_TextureBuffer_Vk final : public SR_TextureBuffer
	{
		friend class SR_GraphicsDevice_Vk;
		friend class SR_GraphicsContext_Vk;
	public:
		SR_TextureBuffer_Vk();
		~SR_TextureBuffer_Vk();

		VkImage myVkImage;
	private:
	};
}
#endif

