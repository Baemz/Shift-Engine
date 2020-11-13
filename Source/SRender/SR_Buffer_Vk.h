#pragma once
#include "SR_Buffer.h"

#if ENABLE_VULKAN
namespace Shift
{
	class SR_Buffer_Vk final : public SR_Buffer
	{
		friend class SR_GraphicsDevice_Vk;
		friend class SR_GraphicsContext_Vk;
	public:
		SR_Buffer_Vk();
		~SR_Buffer_Vk();
	private:

	};
}

#endif