#pragma once
namespace Shift
{
	struct SG_ViewSettings
	{

		bool myRenderDepth : 1;
		bool myRenderGBuffers : 1;
		bool myRenderDeferredLights : 1;
		bool myRenderForward : 1;
		bool myRenderPostFX : 1;
		bool myRenderUI : 1;
		bool myRenderSky : 1;
		bool myUseVoxels : 1;
		bool myEnableRaytracing : 1;

		bool myEnableAmbientOcclusion : 1;
	};
}