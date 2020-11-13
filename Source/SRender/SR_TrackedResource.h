#pragma once
#include "SR_GraphicsResource.h"
#include "SR_RenderEnums.h"
#include "SR_WaitEvent.h"
#include "SR_GraphicsDefinitions.h"

#if ENABLE_DX12
#include "SR_ResourceTracking_DX12.h"
#endif

#if ENABLE_VULKAN
#include "SR_ResourceTracking_Vk.h"
#include "SR_ForwardDeclarations_Vk.h"
#endif


struct ID3D12Resource;

namespace Shift
{
	class SR_WaitEvent;
	class SR_TrackedResource : public SR_Resource
	{			
		using BaseClass = SR_Resource;
	public:
		SR_TrackedResource();
		virtual ~SR_TrackedResource(); 

#if ENABLE_DX12
		ID3D12Resource* myDX12Resource;
#endif

		union {
#if ENABLE_DX12
			SR_ResourceTracking_DX12 myDX12Tracking;
#endif
#if ENABLE_VULKAN
			SR_ResourceTracking_Vk myVkTracking;
#endif
		};
	};
}
