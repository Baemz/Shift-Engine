#pragma once
#include "SG_Camera.h"
#include "SG_DirectionalLight.h"
#include "SG_DrawSubmitStructs.h"
#include "SG_Shadows.h"
#include "SG_RenderQueues.h"

#include "SR_BufferView.h"

namespace Shift
{
	class SR_WaitEvent;

#if SR_ENABLE_RAYTRACING
	struct SG_RaytracingRenderData
	{
		SC_GrowingArray<SR_RaytracingInstanceData> myInstances;

		bool myShouldUpdateScene;

		void Clear();
	};
#endif

	class SG_RenderData
	{
	public:
		SG_RenderData();
		~SG_RenderData();

		void Clear();

		SG_CameraRenderQueues myQueues;
		SG_LightRenderQueues myCSMQueues[SG_Shadows::ourNumShadowCascades];

#if SR_ENABLE_RAYTRACING
		SG_RaytracingRenderData myRaytracingData;
#endif
		SC_GrowingArray<SG_Light::GPUData> myLights;
		SG_PrimitiveRenderObject mySky;
		SG_DirectionalLight mySun;
		SG_PointLightObject _PL;

		SC_Future<bool> myViewWorldPrepareEvent_Main;
		SC_Future<bool> myViewWorldPrepareEvent_MeshesDepth;
		SC_Future<bool> myViewWorldPrepareEvent_MeshesGbuffer;
		SC_Future<bool> myViewWorldPrepareEvent_MeshesForward;
		SC_Future<bool> myViewWorldPrepareEvent_MeshesShadow;
		SC_Future<bool> myViewWorldPrepareEvent_Lights;
		SC_Future<void> myViewWorldPrepareEvent_UpdateRTScene;

		SC_Future<void> myPrepareSettings;
		SC_Future<void> myPrepareShadowProjections;

		SC_Ref<SR_Waitable> mySetViewRefsEvent;
		SC_Ref<SR_Waitable> myUpdateRTSceneEvent;
		SC_Ref<SR_Waitable> myRenderDepthEvent;
		SC_Ref<SR_Waitable> myRenderPostDepthEvent;
		SC_Ref<SR_Waitable> myRenderShadowsEvent;
		SC_Ref<SR_Waitable> myRenderSkyProbeEvent;
		SC_Ref<SR_Waitable> myRenderGBufferEvent;
		SC_Ref<SR_Waitable> myRenderAmbientOcclusionEvent;
		SC_Ref<SR_Waitable> myRenderRelightVoxelsEvent;
		SC_Ref<SR_Waitable> myRenderLightCullingEvent;
		SC_Ref<SR_Waitable> myRenderDeferredLightsEvent;
		SC_Ref<SR_Waitable> myRenderForwardEvent;
		SC_Ref<SR_Waitable> myRenderPostFXEvent;
		SC_Ref<SR_Waitable> myRenderUIEvent;
		SC_GrowingArray<SC_Ref<SR_Waitable>> myEvents;


		SG_Camera myShadowLightCameras[SG_Shadows::ourNumShadowCascades];
		SG_Camera myMainCamera;
		uint64 myFrameIndex;
		float myFrameDeltaTime;
		float myTotalTime;
	};
}