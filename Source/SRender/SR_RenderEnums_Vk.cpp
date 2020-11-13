#include "SRender_Precompiled.h"
#include "SR_RenderEnums_Vk.h"

#if ENABLE_VULKAN

namespace Shift
{
	VkImageViewType SR_GetImageViewType_Vk(SR_Dimension aDimensionType, bool aIsArray)
	{
		switch (aDimensionType)
		{
		case SR_Dimension_Texture1D:
			return aIsArray ? VK_IMAGE_VIEW_TYPE_1D_ARRAY : VK_IMAGE_VIEW_TYPE_1D;
		case SR_Dimension_Texture2D:
			return aIsArray ? VK_IMAGE_VIEW_TYPE_2D_ARRAY : VK_IMAGE_VIEW_TYPE_2D;
		case SR_Dimension_Texture3D:
			SC_ASSERT(!aIsArray);
			return VK_IMAGE_VIEW_TYPE_3D;
		case SR_Dimension_TextureCube:
			return aIsArray ? VK_IMAGE_VIEW_TYPE_CUBE_ARRAY : VK_IMAGE_VIEW_TYPE_CUBE;
		default:
			SC_ASSERT(false, "Unknown dimension type/array combination: %d, %d", aDimensionType, aIsArray);
			return VK_IMAGE_VIEW_TYPE_MAX_ENUM;
		}
	}
	VkImageType SR_GetImageType_Vk(SR_Dimension aDimensionType)
	{
		switch (aDimensionType)
		{
		case SR_Dimension_Texture1D:
			return VK_IMAGE_TYPE_1D;
		case SR_Dimension_Texture2D:
		case SR_Dimension_TextureCube:
			return VK_IMAGE_TYPE_2D;
		case SR_Dimension_Texture3D:
			return VK_IMAGE_TYPE_3D;
		}

		SC_ASSERT(false, "Unknows dimension type: %d", aDimensionType);
		return VK_IMAGE_TYPE_2D;
	}
}

#endif