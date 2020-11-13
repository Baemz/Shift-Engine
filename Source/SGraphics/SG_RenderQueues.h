#pragma once
#include "SG_RenderQueue.h"

#include "SR_GBufferDefines.h"

namespace Shift
{
	struct SG_CameraRenderQueues
	{
		SG_RenderQueue_State myDepth;
		SG_RenderQueue_State myVoxelize;
		SG_RenderQueue_State myGBuffers[SR_GBufferPermutation_COUNT];
		SG_RenderQueue_State myForwardOpaque;
		SG_RenderQueue_State myForwardTransparent;

		void Clear();
	};

	struct SG_LightRenderQueues
	{
		SG_RenderQueue_State myDepth;

		void Clear();
	};
}