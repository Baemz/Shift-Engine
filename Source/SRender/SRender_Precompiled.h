#pragma once

// Includes
#include "SC_CommonDefines.h"
#include "SC_CommonIncludes.h"
#include "SC_Mutex.h"
#include "SC_Profiler.h"

#include "SR_RenderDefines.h"

#if ENABLE_DX12
#	include "SR_Header_DX12.h"
#	include "SR_RenderEnums_DX12.h"
#	include "SR_Converters_DX12.h"
#endif
#if ENABLE_VULKAN
#	include "SR_Header_Vk.h"
#	include "SR_Converters_Vk.h"
#	include "SR_RenderEnums_Vk.h"
#endif
#if ENABLE_PS4
#	include "SR_Header_PS4.h"
#	include "SR_RenderEnums_PS4.h"
#endif

#include "SR_Strings.h"
#include "SR_ResourceRefs.h"
#include "SR_GraphicsDefinitions.h"
#include "SR_GeneralShaderInterop.h"
#include "SR_GraphicsResources.h"
#include "SR_WaitEvent.h"
#include "SR_GraphicsDevice.h"
#include "SR_GraphicsContext.h"
#include "SR_QueueManager.h"

#include "SR_ImGUI.h"