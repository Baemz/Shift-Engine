#include "SGraphics_Precompiled.h"
#include "SG_RenderData.h"
#include "SG_Material.h"
#include "SG_Model.h"

namespace Shift
{
#if SR_ENABLE_RAYTRACING
	void SG_RaytracingRenderData::Clear()
	{
		myInstances.RemoveAll();

		myShouldUpdateScene = false;
	}
#endif

	SG_RenderData::SG_RenderData()
		: myFrameIndex(0)
	{
	}

	SG_RenderData::~SG_RenderData()
	{
	}

	void SG_RenderData::Clear()
	{
		myQueues.Clear();

		for (uint i = 0; i < SG_Shadows::ourNumShadowCascades; ++i)
			myCSMQueues[i].Clear();

		myLights.RemoveAll();

#if SR_ENABLE_RAYTRACING
		myRaytracingData.Clear();
#endif
		mySky = SG_PrimitiveRenderObject();

		myViewWorldPrepareEvent_Main.Reset();
		myViewWorldPrepareEvent_MeshesDepth.Reset();
		myViewWorldPrepareEvent_MeshesGbuffer.Reset();
		myViewWorldPrepareEvent_MeshesForward.Reset();
		myViewWorldPrepareEvent_MeshesShadow.Reset();
		myViewWorldPrepareEvent_Lights.Reset();
		myViewWorldPrepareEvent_UpdateRTScene.Reset();
		myPrepareSettings.Reset();
		myPrepareShadowProjections.Reset();

		mySetViewRefsEvent.Reset();
		myUpdateRTSceneEvent.Reset();
		myRenderDepthEvent.Reset();
		myRenderPostDepthEvent.Reset();
		myRenderShadowsEvent.Reset();
		myRenderSkyProbeEvent.Reset();
		myRenderGBufferEvent.Reset();
		myRenderAmbientOcclusionEvent.Reset();
		myRenderRelightVoxelsEvent.Reset();
		myRenderLightCullingEvent.Reset();
		myRenderDeferredLightsEvent.Reset();
		myRenderForwardEvent.Reset();
		myRenderPostFXEvent.Reset();
		myRenderUIEvent.Reset();
		myEvents.RemoveAll();

		myFrameIndex = 0;
		myFrameDeltaTime = 0.0;
		myTotalTime = 0.0;
	}
}