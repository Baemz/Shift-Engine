#pragma once
#include "SR_BufferView.h"

#if ENABLE_VULKAN
namespace Shift
{
	class SR_BufferView_Vk final : public SR_BufferView
	{
		friend class SR_GraphicsDevice_Vk;
		friend class SR_GraphicsContext_Vk;

	public:
		SR_BufferView_Vk();
		~SR_BufferView_Vk();
	private:

	};
}

#endif

