#pragma once

#include "SC_PlatformDefines.h"

#if ENABLE_DX12

#	if IS_PC_PLATFORM
#		define SR_IID_PPV_ARGS IID_PPV_ARGS
#	endif

#	define SR_INCLUDE_FILE_D3D12 <d3d12.h>
#	define SR_INCLUDE_FILE_D3D12SHADER <d3d12shader.h>

#	define SR_INCLUDE_FILE_DXGI <dxgi1_6.h>
#	define SR_INCLUDE_FILE_DXGITYPE <dxgitype.h>

#	define SR_INCLUDE_FILE_D3DCOMMON <d3dcommon.h>
#	define SR_INCLUDE_FILE_D3DCOMPILER <d3dcompiler.h>
#endif

#if ENABLE_VULKAN
#	define ENABLE_VULKAN_BETA_HEADER 1

#	define SR_INCLUDE_FILE_VULKAN <vulkan/vulkan.h>

#	if IS_WINDOWS
#		define SR_INCLUDE_FILE_VULKAN_WIN32 <vulkan/vulkan_win32.h>
#	elif IS_LINUX
#		define SR_INCLUDE_FILE_VULKAN_LINUX <vulkan/vulkan_xlib.h>
#	endif
#endif

#define SR_ENABLE_TESSELATION_SHADERS 1
#define SR_ENABLE_GEOMETRY_SHADERS 1
#define SR_ENABLE_MESH_SHADERS 0
#define SR_ENABLE_RAYTRACING (ENABLE_DX12 && IS_PC_PLATFORM)