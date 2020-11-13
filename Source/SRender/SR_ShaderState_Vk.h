#pragma once
#include "SR_ShaderState.h"

#if ENABLE_VULKAN

namespace Shift 
{
	class SR_ShaderState_Vk final : public SR_ShaderState
	{
		friend class SR_GraphicsDevice_Vk;
		friend class SR_GraphicsContext_Vk;
	public:
		SR_ShaderState_Vk();
		~SR_ShaderState_Vk();
	private:

	};
}

#endif