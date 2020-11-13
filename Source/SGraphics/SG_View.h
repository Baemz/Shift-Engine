#pragma once

#include "SC_RefCounted.h"

#include "SG_RenderData.h"
#include "SG_Camera.h"
#include "SG_FrameResources.h"
#include "SG_ViewSettings.h"
#include "SG_PostEffects.h"

namespace Shift
{
	class SG_World;
	class SG_Shadows;
	class SG_Voxelizer;
	class SR_BufferView;

	class SG_View : public SC_RefCounted
	{
	public:
		SG_View(SG_World* aWorld);
		~SG_View();

		void Prepare();
		void Render();

		void SetWorld(SG_World* aWorld);
		void SetCamera(const SG_Camera& aCamera);
		const SG_Camera& GetCamera() const { return myCamera; }

		SG_RenderData& GetPrepareData();
		const SG_RenderData& GetRenderData() const;
		SG_RenderData& GetRenderData_Mutable();

		SR_Texture* GetLastFinishedFrame();
		SR_Texture* GetShadowTexture();

		const SG_ViewSettings& GetViewSettings() const { return mySettings; }

		SG_FrameResources myFrameResources;
		SG_World* myWorld;

		struct AmbientOcclusionSettings
		{
			enum Type : uint8 
			{
				SSAO,
				RTAO
			};

			SC_Ref<SR_Texture> mySSAORand;
			SC_Ref<SR_ShaderState> myShader;

			float mySSAOSamplingRadius;
			float mySSAOScale;
			float mySSAOBias;
			float mySSAOIntensity;

			float myRTAORadius;
			uint myRTAONumRaysPerPixel;

			Type myType;
			bool myEnabled;
		};

		struct Lighting
		{
			SC_Ref<SR_ShaderState> myCullingShader;
			SC_Ref<SR_ShaderState> myCullingShaderDebug;
			SC_Ref<SR_ShaderState> myShader;
			SC_Ref<SR_ShaderState> myShaderDebug;
		};

		struct PostFX
		{
			SC_Ref<SR_Buffer> myFullscreenQuadBuffer;
			SC_Ref<SR_Buffer> myFullscreenQuadIBuffer;

			SC_Ref<SR_ShaderState> myFXAAShader;
			SC_Ref<SR_ShaderState> myTonemapShader;

			float myBloomThreshold;
			float myBloomIntensity;

			float myExposure;


			bool myUseBloom;
			bool myUseFXAA;
		};

		AmbientOcclusionSettings myAmbientOcclusionSettings;
		bool myDebugLightHeatmap;
		SG_Shadows* myShadows;

		static constexpr uint ourMaxNumFramesInFlight = 3;
	private:

		void StartPrepare();
		void EndPrepare();

		void StartRender();
		void EndRender();

	private:

		void Initialize();

		void UpdateViewSettings();

		typedef void (SG_View::* RenderFunc)();
		void PostRenderTask(SC_Ref<SR_Waitable>& aEventOut, RenderFunc aTask, bool aEnabled = true, uint aWaitMode = SR_WaitableMode_CPU_GPU);
		void PostComputeTask(SC_Ref<SR_Waitable>& aEventOut, RenderFunc aTask, bool aEnabled = true, uint aWaitMode = SR_WaitableMode_CPU_GPU);
		void PostCopyTask(SC_Ref<SR_Waitable>& aEventOut, RenderFunc aTask, bool aEnabled = true, uint aWaitMode = SR_WaitableMode_CPU_GPU);

		void Render_SetConstantBufferRefs();
		void Render_UpdateRTScene();
		void Render_SkyProbes();
		void Render_Depth();
		void Render_PostDepth();
		void Render_RelightVoxels();
		void Render_Shadows();
		void Render_GBuffer();
		void Render_AmbientOcclusion();
		void Render_LightCulling();
		void Render_DeferredLights();
		void Render_Forward();
		void Render_PostFX();
		void Render_UI();

		void RenderSky();
		
		void RenderDebugPrimitives();

		SG_Voxelizer* myVoxelizer;

		SG_RenderData myViewData[ourMaxNumFramesInFlight];
		SC_Ref<SR_Waitable> myInFlightFrameEvents[ourMaxNumFramesInFlight];

		uint myCurrentPrepareDataIndex;
		uint myCurrentRenderDataIndex;

		SC_GrowingArray<SC_Ref<SR_WaitEvent>> myEventsThisFrame;
		Lighting myLighting;
		PostFX myPostFX;
		SG_ViewSettings mySettings;
		SG_Camera myCamera;
		SG_PostEffects myPostEffects;
		volatile uint64 myCurrentFrameIndex;

		SC_Ref<SR_ShaderState> myLinearizeDepthShader;
		SC_Ref<SR_Buffer> myTimestampBuffer;
		SC_Ref<SR_Buffer> myLightsData;
		SC_Ref<SR_BufferView> myLightsDataView;
		SC_Ref<SR_BufferView> myLightCullingData;
		SC_Ref<SR_BufferView> myLightCullingDataRW;
		SR_TextureResource myDebugTileHeatmap;


	};

}