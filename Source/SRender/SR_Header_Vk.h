#pragma once
#include "SR_RenderDefines.h"

#if ENABLE_VULKAN

#if ENABLE_VULKAN_BETA_HEADER
#	define VK_ENABLE_BETA_EXTENSIONS
#endif

#include SR_INCLUDE_FILE_VULKAN


#if IS_WINDOWS
#	include SR_INCLUDE_FILE_VULKAN_WIN32
#elif IS_LINUX
#	include SR_INCLUDE_FILE_VULKAN_LINUX
#endif

#pragma comment(lib,"vulkan-1.lib")

#endif