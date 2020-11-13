#pragma once

#if ENABLE_VULKAN
#include "SR_RenderEnums.h"

namespace Shift
{
	VkImageViewType SR_GetImageViewType_Vk(SR_Dimension aDimensionType, bool aIsArray);
	VkImageType SR_GetImageType_Vk(SR_Dimension aDimensionType);
}

#endif

