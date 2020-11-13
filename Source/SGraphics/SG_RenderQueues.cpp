#include "SGraphics_Precompiled.h"
#include "SG_RenderQueues.h"

namespace Shift
{
	void SG_CameraRenderQueues::Clear()
	{
		myDepth.Clear();
		myVoxelize.Clear();
		for (uint i = 0; i < SR_GBufferPermutation_COUNT; ++i)
			myGBuffers[i].Clear();

		myForwardOpaque.Clear();
		myForwardTransparent.Clear();
	}

	void SG_LightRenderQueues::Clear()
	{
		myDepth.Clear();
	}
}