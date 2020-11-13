#pragma once

#if defined(_WIN64)
#	define IS_PC_PLATFORM			(1)
#	define IS_WINDOWS				(1)
#	define IS_X86_PLATFORM			(1)

#	ifndef WIN32_LEAN_AND_MEAN
#		define WIN32_LEAN_AND_MEAN
#	endif
#	ifndef NOMINMAX
#		define NOMINMAX
#	endif
#	include <windows.h>
#	include <synchapi.h>

#elif defined(__linux__)
#	define IS_PC_PLATFORM			(1)
#	define IS_LINUX					(1)
#	define IS_X86_PLATFORM			(1)

#elif defined(__ANDROID__)
#	define IS_ANDROID_PLATFORM		(1)

#elif defined(_XBOX_ONE)
#	define IS_XBOX_ONE_PLATFORM		(1)

#elif defined(__PS4__)
#	define IS_PS4_PLATFORM			(1)

#else
#	error Platform not implemented!
#endif



#if !defined(IS_PC_PLATFORM)
#define IS_PC_PLATFORM				(0)
#endif

#if !defined(IS_WINDOWS)
#define IS_WINDOWS					(0)
#endif

#ifndef IS_X86_PLATFORM
#define IS_X86_PLATFORM				(0)
#endif

#if !defined(IS_LINUX)
#define IS_LINUX					(0)
#endif

#if !defined(IS_ANDROID_PLATFORM)
#define IS_ANDROID_PLATFORM			(0)
#endif

#if !defined(IS_XBOX_ONE_PLATFORM)
#define IS_XBOX_ONE_PLATFORM		(0)
#endif

#if !defined(IS_PS4_PLATFORM)
#define IS_PS4_PLATFORM				(0)
#endif


#if IS_PC_PLATFORM
#	if IS_WINDOWS
#		define IS_WINDOWS_20H1		(0)
#		define ENABLE_DX12			(1)
#		define ENABLE_DXIL			(1)
#		define ENABLE_DX12_20H1		(0)
#		define ENABLE_VULKAN		(0)
#	elif IS_LINUX
#		define ENABLE_VULKAN		(1)
#	else
#		error Platform not supported.
#	endif
#elif IS_XBOX_ONE_PLATFORM
#	define ENABLE_DX12				(1)
#elif IS_PS4_PLATFORM
#	define ENABLE_PS4				(1)
#else
#	error Platform not supported.
#endif

#if !defined(IS_WINDOWS_20H1)
#define IS_WINDOWS_20H1				(0)
#endif

#if !defined(ENABLE_DX12)
#define ENABLE_DX12					(0)
#endif

#if !defined(ENABLE_DX12_20H1)
#define ENABLE_DX12_20H1			(0)
#endif

#if !defined(ENABLE_DXIL)
#define ENABLE_DXIL					(0)
#endif

#if !defined(ENABLE_VULKAN)
#define ENABLE_VULKAN				(0)
#endif

#if !defined(ENABLE_PS4)
#define ENABLE_PS4					(0)
#endif

#if _MSC_VER
#	define IS_VISUAL_STUDIO			(1)
#else
#	define IS_VISUAL_STUDIO			(0)
#endif

#if _MSC_VER >= 1920
#define IS_VS16						(1)
#define IS_VISUAL_STUDIO_2019		(1)

#elif _MSC_VER >= 1910
#define IS_VS15						(1)
#define IS_VISUAL_STUDIO_2017		(1)

#elif _MSC_VER >= 1900
#define IS_VS14						(1)
#define IS_VISUAL_STUDIO_2015		(1)
#endif

#if !defined(IS_VS16)
#define IS_VS16						(0)
#define IS_VISUAL_STUDIO_2019		(0)
#endif

#if !defined(IS_VS15)
#define IS_VS15						(0)
#define IS_VISUAL_STUDIO_2017		(0)
#endif

#if !defined(IS_VS14)
#define IS_VS14						(0)
#define IS_VISUAL_STUDIO_2015		(0)
#endif