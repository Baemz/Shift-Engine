#include "SRender_Precompiled.h"
#include "SR_TrackedResource.h"

namespace Shift
{
	SR_TrackedResource::SR_TrackedResource()
#if ENABLE_DX12
		: myDX12Resource(nullptr)
#endif
	{
	}

	SR_TrackedResource::~SR_TrackedResource()
	{
#if ENABLE_DX12
		if (myDX12Resource)
			myDX12Resource->Release();
#endif
	}
}