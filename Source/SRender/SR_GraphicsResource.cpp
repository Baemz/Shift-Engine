#include "SRender_Precompiled.h"
#include "SR_GraphicsResource.h"

namespace Shift
{
	SR_Resource::SR_Resource()
	{
	}

	bool SR_Resource::CanRelease(volatile uint& aRefCount)
	{
		if (SC_RefCounted::CanRelease(aRefCount))
		{
			SR_GraphicsDevice* device = SR_GraphicsDevice::GetDevice();
			if (device && device->AddToReleasedResourceCache(this, SC_Timer::GetGlobalFrameIndex() + 1))
				return false;
			else
				return true;
		}
		return false;
	}
}