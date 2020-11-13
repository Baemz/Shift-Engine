#include "SGraphics_Precompiled.h"
#include "SG_View.h"
#include "SG_World.h"
#include "SG_Shadows.h"
#include "SG_Terrain.h"
#include "SG_DrawSubmitStructs.h"
#include "SG_Material.h"
#include "SG_CommonRenderStates.h"
#include "SG_EnvironmentConstants.h"
#include "SG_ShadowConstants.h"
#include "SG_Voxelizer.h"
#include "SG_Raytracer.h"

#include "SR_WaitEvent.h"
#include "SR_SwapChain.h"
#include "SR_ViewConstants.h"
#include "SR_ResourceRefs.h"


namespace Shift
{

	SG_View::SG_View(SG_World* aWorld)
		: myCurrentFrameIndex(0)
		, myCurrentPrepareDataIndex(0)
		, myDebugLightHeatmap(false)
	{


		myWorld = aWorld;

		Initialize();

		SR_GraphicsDevice* device = SR_GraphicsDevice::GetDevice();
		SR_ShaderCompiler* compiler = device->GetShaderCompiler();
		SC_Ref<SR_ShaderByteCode> bc = compiler->CompileShaderFromFile("ShiftEngine/Shaders/SGraphics/Light/Light_Compute.ssf", SR_ShaderType_Compute);
		
		SR_ShaderStateDesc ssdesc;
		ssdesc.myShaderByteCodes[SR_ShaderType_Compute] = bc;
		
		SC_Ref<SR_ShaderState> ss = device->CreateShaderState(ssdesc);

	}

	SG_View::~SG_View()
	{
		delete myShadows;
		delete myVoxelizer;
	}

	void SG_View::Prepare()
	{
		SC_PROFILER_FUNCTION();
		StartPrepare();
		SG_RenderData& prepareData = GetPrepareData();
		prepareData.myFrameIndex = SC_Timer::GetGlobalFrameIndex();
		prepareData.myFrameDeltaTime = SC_Timer::GetGlobalDeltaTime();
		prepareData.myTotalTime = SC_Timer::GetGlobalTotalTime();

		prepareData.myMainCamera = myCamera;
		UpdateViewSettings();

		prepareData.myPrepareShadowProjections = SC_CreateFrameTask(myShadows, &SG_Shadows::PrepareView, this);
		prepareData.myViewWorldPrepareEvent_Main = SC_CreateFrameTask(myWorld, &SG_World::PrepareView, this);

		EndPrepare();
	}

	void SG_View::Render()
	{
		StartRender();
		SG_RenderData& renderData = GetRenderData_Mutable();

		SR_RenderInterface::ourPresentEvent->myEventCPU.Wait();

		PostRenderTask(renderData.mySetViewRefsEvent, &SG_View::Render_SetConstantBufferRefs);
		PostComputeTask(renderData.myUpdateRTSceneEvent, &SG_View::Render_UpdateRTScene, mySettings.myEnableRaytracing);
		PostRenderTask(renderData.myRenderDepthEvent, &SG_View::Render_Depth);
		PostRenderTask(renderData.myRenderPostDepthEvent, &SG_View::Render_PostDepth);
		PostRenderTask(renderData.myRenderGBufferEvent, &SG_View::Render_GBuffer, mySettings.myRenderGBuffers);
		PostRenderTask(renderData.myRenderShadowsEvent, &SG_View::Render_Shadows, true);
		PostComputeTask(renderData.myRenderSkyProbeEvent, &SG_View::Render_SkyProbes, true);
		PostComputeTask(renderData.myRenderLightCullingEvent, &SG_View::Render_LightCulling, true);
		PostComputeTask(renderData.myRenderRelightVoxelsEvent, &SG_View::Render_RelightVoxels, mySettings.myUseVoxels);
		PostComputeTask(renderData.myRenderAmbientOcclusionEvent, &SG_View::Render_AmbientOcclusion, mySettings.myEnableAmbientOcclusion);

		PostRenderTask(renderData.myRenderDeferredLightsEvent, &SG_View::Render_DeferredLights, mySettings.myRenderDeferredLights);

		PostRenderTask(renderData.myRenderForwardEvent, &SG_View::Render_Forward, true);
		PostRenderTask(renderData.myRenderPostFXEvent, &SG_View::Render_PostFX, mySettings.myRenderPostFX);
		PostRenderTask(renderData.myRenderUIEvent, &SG_View::Render_UI, true);
		

		for (auto& event : renderData.myEvents)
			event->myEventCPU.Wait();

		SR_RenderInterface::ourLastViewTaskFence.myFenceValue = renderData.myEvents.GetLast()->myFence;
		SR_RenderInterface::ourLastViewTaskFence.myContextType = renderData.myEvents.GetLast()->myFenceContext;
		myInFlightFrameEvents[myCurrentRenderDataIndex] = renderData.myEvents.GetLast();
		EndRender();
	}

	void SG_View::SetWorld(SG_World* aWorld)
	{
		myWorld = aWorld;
	}

	void SG_View::SetCamera(const SG_Camera& aCamera)
	{
		myCamera = aCamera;
	}

	SG_RenderData& SG_View::GetPrepareData()
	{
		return myViewData[myCurrentPrepareDataIndex];
	}

	const SG_RenderData& SG_View::GetRenderData() const
	{
		return myViewData[myCurrentRenderDataIndex];
	}

	SG_RenderData& SG_View::GetRenderData_Mutable()
	{
		return myViewData[myCurrentRenderDataIndex];
	}

	SR_Texture* SG_View::GetLastFinishedFrame()
	{
		return myFrameResources.myLastTextureWritten.myTexture;
	}

	SR_Texture* SG_View::GetShadowTexture()
	{
		return myShadows->GetSunShadowMap();
	}

	void SG_View::StartPrepare()
	{
		myCurrentPrepareDataIndex = (myCurrentPrepareDataIndex == ourMaxNumFramesInFlight - 1) ? 0 : ++myCurrentPrepareDataIndex;
		if (myInFlightFrameEvents[myCurrentPrepareDataIndex])
			myInFlightFrameEvents[myCurrentPrepareDataIndex]->myEventCPU.Wait();	

		GetPrepareData().Clear();
	}

	void SG_View::EndPrepare()
	{
	}

	void SG_View::StartRender()
	{
		myCurrentRenderDataIndex = myCurrentPrepareDataIndex;
		GetRenderData().myViewWorldPrepareEvent_Main.Wait();
	}

	void SG_View::EndRender()
	{
	}

	void SG_View::Initialize()
	{
		SR_GraphicsDevice* device = SR_GraphicsDevice::GetDevice();
		const SC_Vector2f& framebufferRes = device->GetResolution();

		myFrameResources.myGBuffer.Update();

		myFrameResources.myDepth.myCreateRenderTarget = true;
		myFrameResources.myDepth.myCreateTexture = true;
		myFrameResources.myDepth.myTextureBufferFormat = SR_Format_D32_Float;
		myFrameResources.myDepth.myTextureFormat = SR_Format_R32_Float;
		myFrameResources.myDepth.Update(framebufferRes, "DepthBuffer");

		myFrameResources.myDepthLinear.myCreateRenderTarget = false;
		myFrameResources.myDepthLinear.myCreateRWTexture = true;
		myFrameResources.myDepthLinear.myCreateTexture = true;
		myFrameResources.myDepthLinear.myTextureBufferFormat = SR_Format_R32_Float;
		myFrameResources.myDepthLinear.myTextureFormat = SR_Format_R32_Float;
		myFrameResources.myDepthLinear.Update(framebufferRes, "DepthBuffer Linear");

		myFrameResources.myAmbientOcclusion.myCreateTexture = true;
		myFrameResources.myAmbientOcclusion.myCreateRWTexture = true;
		myFrameResources.myAmbientOcclusion.myTextureFormat = SR_Format_R32_Float;
		myFrameResources.myAmbientOcclusion.Update(framebufferRes, "Ambient Occlusion");

		myFrameResources.myFullscreenHDR.myCreateTexture = true;
		myFrameResources.myFullscreenHDR.myCreateRWTexture = true;
		myFrameResources.myFullscreenHDR.myCreateRenderTarget = true;
		myFrameResources.myFullscreenHDR.myTextureFormat = SR_Format_RG11B10_Float;
		myFrameResources.myFullscreenHDR.Update(framebufferRes, "Fullscreen HDR Texture");

		myFrameResources.myFullscreen.myCreateTexture = true;
		myFrameResources.myFullscreen.myCreateRWTexture = true;
		myFrameResources.myFullscreen.myCreateRenderTarget = true;
		myFrameResources.myFullscreen.myTextureFormat = SR_Format_RGBA8_Unorm;
		myFrameResources.myFullscreen.Update(framebufferRes, "Fullscreen Texture");

		myFrameResources.myFullscreen2.myCreateTexture = true;
		myFrameResources.myFullscreen2.myCreateRWTexture = true;
		myFrameResources.myFullscreen2.myCreateRenderTarget = true;
		myFrameResources.myFullscreen2.myTextureFormat = SR_Format_RGBA8_Unorm;
		myFrameResources.myFullscreen2.Update(framebufferRes, "Fullscreen Texture 2");

		myFrameResources.myScreenHistory[0].myCreateTexture = true;
		myFrameResources.myScreenHistory[0].myTextureFormat = SR_Format_RGBA8_Unorm;
		myFrameResources.myScreenHistory[0].Update(framebufferRes, "Screen History Texture");

		myFrameResources.myScreenHistory[1].myCreateTexture = true;
		myFrameResources.myScreenHistory[1].myTextureFormat = SR_Format_RGBA8_Unorm;
		myFrameResources.myScreenHistory[1].Update(framebufferRes, "Screen History Texture");

		myFrameResources.myScreenHistory[2].myCreateTexture = true;
		myFrameResources.myScreenHistory[2].myTextureFormat = SR_Format_RGBA8_Unorm;
		myFrameResources.myScreenHistory[2].Update(framebufferRes, "Screen History Texture");

		myFrameResources.myScreenHistory[3].myCreateTexture = true;
		myFrameResources.myScreenHistory[3].myTextureFormat = SR_Format_RGBA8_Unorm;
		myFrameResources.myScreenHistory[3].Update(framebufferRes, "Screen History Texture");

		myDebugTileHeatmap.myCreateTexture = true;
		myDebugTileHeatmap.myCreateRWTexture = true;
		myDebugTileHeatmap.myTextureFormat = SR_Format_RGBA8_Unorm;
		myDebugTileHeatmap.Update({ ceilf(framebufferRes.x / 16), ceilf(framebufferRes.y / 16) }, "DEBUG TileData Heatmap");

		SR_TextureBufferDesc skyProbeDesc;
		skyProbeDesc.myDimension = SR_Dimension_TextureCube;
		skyProbeDesc.myFlags |= SR_ResourceFlag_AllowWrites;
		skyProbeDesc.myWidth = 256.f;
		skyProbeDesc.myHeight = 256.f;
		skyProbeDesc.myMips = 1;
		skyProbeDesc.myFormat = SR_Format_RG11B10_Float;

		myFrameResources.mySkyProbeTextureBuffer = device->CreateTextureBuffer(skyProbeDesc);
		myFrameResources.mySkyProbeIrradianceTextureBuffer = device->CreateTextureBuffer(skyProbeDesc);

		SR_TextureDesc skyProbeCubeDesc;
		skyProbeCubeDesc.myFormat = SR_Format_RG11B10_Float;
		myFrameResources.mySkyProbeTexture = device->CreateTexture(skyProbeCubeDesc, myFrameResources.mySkyProbeTextureBuffer);
		myFrameResources.mySkyProbeIrradianceTexture = device->CreateTexture(skyProbeCubeDesc, myFrameResources.mySkyProbeIrradianceTextureBuffer);

		SR_TextureDesc skyProbeRWDesc;
		skyProbeRWDesc.myFormat = SR_Format_RG11B10_Float;
		skyProbeRWDesc.myArraySize = 1;
		myFrameResources.mySkyProbeTextureRW = device->CreateRWTexture(skyProbeRWDesc, myFrameResources.mySkyProbeTextureBuffer);
		myFrameResources.mySkyProbeIrradianceTextureRW = device->CreateRWTexture(skyProbeRWDesc, myFrameResources.mySkyProbeIrradianceTextureBuffer);

		myFrameResources.mySkyBrdfTexture = device->GetCreateTexture("ShiftEngine/Textures/skyBrdf.dds");

		SR_BufferDesc quadBuffer;
		quadBuffer.mySize = 4 * sizeof(SVertex_Simple);
		quadBuffer.myStructSize = sizeof(SVertex_Simple);
		quadBuffer.myBindFlag = SR_BindFlag_VertexBuffer;
		myPostFX.myFullscreenQuadBuffer = device->CreateBuffer(quadBuffer, (void*)globalQuadVertexList, "PostFX Quad Vertex Buffer");

		SR_BufferDesc quadIBuffer;
		quadIBuffer.mySize = 6 * sizeof(uint);
		quadIBuffer.myStructSize = sizeof(uint);
		quadIBuffer.myBindFlag = SR_BindFlag_IndexBuffer;
		myPostFX.myFullscreenQuadIBuffer = device->CreateBuffer(quadIBuffer, (void*)globalQuadIndexList, "PostFX Quad Index Buffer");

		SR_ShaderCompiler* shaderCompiler = device->GetShaderCompiler();

		{
			SR_ShaderStateDesc sDesc;
			sDesc.myShaderByteCodes[SR_ShaderType_Compute] = shaderCompiler->CompileShaderFromFile("ShiftEngine/Shaders/SGraphics/PostFX/Tonemap/Tonemap.ssf", SR_ShaderType_Compute);
			myPostFX.myTonemapShader = device->CreateShaderState(sDesc);
		}

		{
			SR_ShaderStateDesc sDesc;
			sDesc.myShaderByteCodes[SR_ShaderType_Compute] = shaderCompiler->CompileShaderFromFile("ShiftEngine/Shaders/SGraphics/PostFX/LinearizeDepth.ssf", SR_ShaderType_Compute);
			myLinearizeDepthShader = device->CreateShaderState(sDesc);
		}

		myPostFX.myUseFXAA = false;
		myPostFX.myUseBloom = false;
		myPostFX.myExposure = 1.0f;
		myPostFX.myBloomIntensity = 0.75f;
		myPostFX.myBloomThreshold = 0.01f;

		SR_ShaderStateDesc ssaoDesc2;
		ssaoDesc2.myShaderByteCodes[SR_ShaderType_Compute] = shaderCompiler->CompileShaderFromFile("ShiftEngine/Shaders/SGraphics/SSAO.ssf", SR_ShaderType_Compute);
		myAmbientOcclusionSettings.myShader = device->CreateShaderState(ssaoDesc2);

		myAmbientOcclusionSettings.mySSAORand = device->GetCreateTexture("_Random.dds");

		myAmbientOcclusionSettings.mySSAOSamplingRadius = 0.175f;
		myAmbientOcclusionSettings.mySSAOScale = 15.0f;
		myAmbientOcclusionSettings.mySSAOBias = 0.1f;
		myAmbientOcclusionSettings.mySSAOIntensity = 5.f;

		myAmbientOcclusionSettings.myRTAONumRaysPerPixel = 1;
		myAmbientOcclusionSettings.myRTAORadius = 5.f;

		myAmbientOcclusionSettings.myType = AmbientOcclusionSettings::SSAO;
		myAmbientOcclusionSettings.myEnabled = true;


		std::wstring definesDebugLightCull[] = { L"ENABLE_HEATMAP_DEBUG", L"USE_DEPTH_CULLING" };
		std::wstring definesLightCull[] = { L"USE_DEPTH_CULLING" };
		SR_ShaderStateDesc lightDesc2;
		lightDesc2.myShaderByteCodes[SR_ShaderType_Compute] = shaderCompiler->CompileShaderFromFile("ShiftEngine/Shaders/SGraphics/Light/Light_Compute.ssf", SR_ShaderType_Compute, definesDebugLightCull, 1);
		myLighting.myShaderDebug = device->CreateShaderState(lightDesc2);
		lightDesc2.myShaderByteCodes[SR_ShaderType_Compute] = shaderCompiler->CompileShaderFromFile("ShiftEngine/Shaders/SGraphics/Light/Light_Compute.ssf", SR_ShaderType_Compute);
		myLighting.myShader = device->CreateShaderState(lightDesc2);

		SR_ShaderStateDesc lightCullDesc;
		lightCullDesc.myShaderByteCodes[SR_ShaderType_Compute] = shaderCompiler->CompileShaderFromFile("ShiftEngine/Shaders/SGraphics/Light/Light_Cull.ssf", SR_ShaderType_Compute, definesDebugLightCull, 2);
		myLighting.myCullingShaderDebug = device->CreateShaderState(lightCullDesc);
		lightCullDesc.myShaderByteCodes[SR_ShaderType_Compute] = shaderCompiler->CompileShaderFromFile("ShiftEngine/Shaders/SGraphics/Light/Light_Cull.ssf", SR_ShaderType_Compute, definesLightCull, 1);
		myLighting.myCullingShader = device->CreateShaderState(lightCullDesc);

		SR_BufferDesc timestampsDesc;
		timestampsDesc.myBindFlag = SBindFlag_Buffer;
		timestampsDesc.myCPUAccess = SR_AccessCPU_Map_Read;
		timestampsDesc.mySize = 24 * sizeof(uint64);
		myTimestampBuffer = device->CreateBuffer(timestampsDesc, nullptr, "Timestamp Buffer");

		uint numTiles = (uint)((framebufferRes.x + 16 - 1) / 16.f) * (uint)((framebufferRes.y + 16 - 1) / 16.f);
		const uint numLightsPerTile = 64;

		SR_BufferDesc lightsDataDesc;
		lightsDataDesc.myBindFlag = SBindFlag_StructuredBuffer;
		lightsDataDesc.myGPUAccess = SR_AccessGPU_Read;
		lightsDataDesc.myIsWritable = false;
		lightsDataDesc.myStructSize = sizeof(SG_Light::GPUData);
		lightsDataDesc.mySize = lightsDataDesc.myStructSize;
		myLightsData = device->CreateBuffer(lightsDataDesc, nullptr, "Lights Data Buffer");

		SR_BufferDesc lightTileDataDesc;
		lightTileDataDesc.myBindFlag = SBindFlag_StructuredBuffer;
		lightTileDataDesc.myGPUAccess = SR_AccessGPU_Write;
		lightTileDataDesc.myIsWritable = true;
		lightTileDataDesc.myStructSize = sizeof(uint) * numLightsPerTile + 1; // +1 is for num active lights count
		lightTileDataDesc.mySize = lightTileDataDesc.myStructSize * numTiles;

		SC_Ref<SR_Buffer> tileDataBuffer = device->CreateBuffer(lightTileDataDesc, nullptr, "Light TileData Buffer");

		SR_BufferViewDesc lightTileDataViewDesc;
		lightTileDataViewDesc.myFirstElement = 0;
		lightTileDataViewDesc.myNumElements = numTiles;
		lightTileDataViewDesc.myType = SR_BufferViewType_Structured;
		lightTileDataViewDesc.myIsShaderWritable = false;
		myLightCullingData = device->CreateBufferView(lightTileDataViewDesc, tileDataBuffer);

		lightTileDataViewDesc.myIsShaderWritable = true;
		myLightCullingDataRW = device->CreateBufferView(lightTileDataViewDesc, tileDataBuffer);

		myShadows = new SG_Shadows();
		myShadows->Init();

		myVoxelizer = new SG_Voxelizer();

		myPostEffects.Init(framebufferRes);

	}
	void SG_View::UpdateViewSettings()
	{
		SR_GraphicsDevice* device = SR_GraphicsDevice::GetDevice();

		mySettings.myRenderDepth = true;
		mySettings.myRenderGBuffers = true;
		mySettings.myRenderDeferredLights = true;
		mySettings.myRenderForward = true;
		mySettings.myRenderPostFX = true;
		mySettings.myRenderUI = true;
		mySettings.myRenderSky = true;
		mySettings.myUseVoxels = false;
		mySettings.myEnableAmbientOcclusion = myAmbientOcclusionSettings.myEnabled;
		mySettings.myEnableRaytracing = device->GetFeatureSupport().myEnableRaytracing && myAmbientOcclusionSettings.myType == AmbientOcclusionSettings::RTAO;
	}
	void SG_View::PostRenderTask(SC_Ref<SR_Waitable>& aEventOut, RenderFunc aTask, bool aEnabled, uint aWaitMode)
	{
		if (!aEnabled)
			return;

		SC_Ref<SR_Waitable> event = SR_RenderInterface::PostRenderTask(std::bind(aTask, this), aWaitMode);
		GetRenderData_Mutable().myEvents.Add(event);
		aEventOut = event;
	}
	void SG_View::PostComputeTask(SC_Ref<SR_Waitable>& aEventOut, RenderFunc aTask, bool aEnabled, uint aWaitMode)
	{
		if (!aEnabled)
			return;

		SC_Ref<SR_Waitable> event = SR_RenderInterface::PostComputeTask(std::bind(aTask, this), aWaitMode);
		GetRenderData_Mutable().myEvents.Add(event);
		aEventOut = event;
	}

	void SG_View::PostCopyTask(SC_Ref<SR_Waitable>& aEventOut, RenderFunc aTask, bool aEnabled, uint aWaitMode)
	{
		if (!aEnabled)
			return;

		SC_Ref<SR_Waitable> event = SR_RenderInterface::PostCopyTask(std::bind(aTask, this), aWaitMode);
		GetRenderData_Mutable().myEvents.Add(event);
		aEventOut = event;
	}

	void SG_View::Render_SetConstantBufferRefs()
	{
		SC_PROFILER_FUNCTION();
		const SG_RenderData& renderData = GetRenderData();
		renderData.myPrepareShadowProjections.Wait();

		SR_PROFILER_FUNCTION();
		const SG_Camera& camera = renderData.myMainCamera;

		const SC_Matrix44& view = camera.GetView();
		const SC_Matrix44& projection = camera.GetProjection();
		const SC_Matrix44& inverseView = camera.GetInverseView();
		const SC_Matrix44& inverseProjection = camera.GetInverseProjection();

		SR_ViewConstants constants;
		constants.myWorldToClip = view * projection;
		constants.myWorldToCamera = view;
		constants.myClipToWorld = inverseProjection * inverseView;
		constants.myClipToCamera = inverseProjection;
		constants.myCameraToClip = projection;
		constants.myCameraToWorld = inverseView;
		constants.myCameraPosition = camera.GetPosition();
		constants.myCameraFov = camera.GetFovInRadians();
		constants.myCameraNear = camera.GetNear();
		constants.myCameraFar = camera.GetFar();
		constants.myResolution = SR_GraphicsDevice::GetDevice()->GetResolution();
		constants.myFrameIndex = (uint)renderData.myFrameIndex;

		SR_GraphicsContext* ctx = SR_GraphicsContext::GetCurrent();
		
		ctx->BindConstantBufferRef(&constants, sizeof(SR_ViewConstants), SR_ConstantBufferRef::SR_ConstantBufferRef_ViewConstants);

		SG_EnvironmentConstants environmentConstants;
		myWorld->mySky->PrepareEnvironmentConstants(environmentConstants);
		ctx->BindConstantBufferRef(&environmentConstants, sizeof(environmentConstants), SR_ConstantBufferRef::SR_ConstantBufferRef_EnvironmentConstants);

		SG_ShadowConstants shadowConstants;
		myShadows->PrepareShadowConstants(shadowConstants, renderData);
		ctx->BindConstantBufferRef(&shadowConstants, sizeof(shadowConstants), SR_ConstantBufferRef::SR_ConstantBufferRef_ShadowConstants);
	}

	void SG_View::Render_UpdateRTScene()
	{
		SC_PROFILER_FUNCTION();
		const SG_RenderData& renderData = GetRenderData();
		renderData.myViewWorldPrepareEvent_UpdateRTScene.Wait();
		SR_PROFILER_FUNCTION();

		myWorld->myRaytracer->UpdateScene(this);
		SR_GraphicsContext::GetCurrent()->BindBufferRef(myWorld->myRaytracer->GetScene(), SR_BufferRef::SR_BufferRef_RaytracingScene);

	}

	void SG_View::Render_SkyProbes()
	{
		SC_PROFILER_FUNCTION();
		const SG_RenderData& renderData = GetRenderData();
		SR_GraphicsContext* ctx = SR_GraphicsContext::GetCurrent();
		ctx->InsertWait(renderData.myRenderPostDepthEvent);
		SR_PROFILER_FUNCTION();
		SG_Camera camera;
		camera.InitAsPerspective({256.f, 256.f}, 90.f, 1.f, 100.f);
		SC_Vector3f targets[6] =
		{
			SC_Vector3f(1, 0, 0),
			SC_Vector3f(-1, 0, 0),
			SC_Vector3f(0, 1, 0),
			SC_Vector3f(0, -1, 0),
			SC_Vector3f(0, 0, 1),
			SC_Vector3f(0, 0, -1),
		};
		{
			SC_Pair<SR_ResourceState, SR_TrackedResource*> transitions[2];
			transitions[0] = SC_Pair(SR_ResourceState_UnorderedAccess, myFrameResources.mySkyProbeIrradianceTexture->GetTextureBuffer());
			transitions[1] = SC_Pair(SR_ResourceState_UnorderedAccess, myFrameResources.mySkyProbeTexture->GetTextureBuffer());
			ctx->Transition(transitions, 2);
		}

		for (uint i = 0; i < 6; ++i)
		{
			std::string tag("SkyProbe Face ");
			tag += std::to_string(i);
			SR_PROFILER_FUNCTION_TAGGED(tag.c_str());

			SC_Vector3f up = { 0.0f, 1.0f, 0.0f };
			if (i == 2)
				up = { 0.0f, 0.0f, -1.0f };
			else if (i == 3)
				up = { 0.0f, 0.0f, 1.0f };

			camera.LookAt({0.0f, 0.0f, 0.0f}, targets[i], up);
			myWorld->mySky->RenderSkyProbeFace(camera, myFrameResources.mySkyProbeTextureRW, i);
		}

		for (uint i = 0; i < 6; ++i)
		{
			std::string tag("SkyProbe Irradiance Face ");
			tag += std::to_string(i);
			SR_PROFILER_FUNCTION_TAGGED(tag.c_str());

			SC_Vector3f up = { 0.0f, 1.0f, 0.0f };
			if (i == 2)
				up = { 0.0f, 0.0f, -1.0f };
			else if (i == 3)
				up = { 0.0f, 0.0f, 1.0f };

			camera.LookAt({ 0.0f, 0.0f, 0.0f }, targets[i], up);
			myWorld->mySky->RenderSkyProbeIrradianceFace(camera, myFrameResources.mySkyProbeIrradianceTextureRW, myFrameResources.mySkyProbeTexture, i);
		}

		const SG_Camera& vcamera = renderData.myMainCamera;

		const SC_Matrix44& view = vcamera.GetView();
		const SC_Matrix44& projection = vcamera.GetProjection();
		const SC_Matrix44& inverseView = vcamera.GetInverseView();
		const SC_Matrix44& inverseProjection = vcamera.GetInverseProjection();

		SR_ViewConstants constants;
		constants.myWorldToClip = view * projection;
		constants.myWorldToCamera = view;
		constants.myClipToWorld = inverseProjection * inverseView;
		constants.myClipToCamera = inverseProjection;
		constants.myCameraToClip = projection;
		constants.myCameraToWorld = inverseView;
		constants.myCameraPosition = vcamera.GetPosition();
		constants.myCameraFov = vcamera.GetFovInRadians();
		constants.myCameraNear = vcamera.GetNear();
		constants.myCameraFar = vcamera.GetFar();
		constants.myResolution = SR_GraphicsDevice::GetDevice()->GetResolution();
		constants.myFrameIndex = (uint)renderData.myFrameIndex;

		{
			SC_Pair<SR_ResourceState, SR_TrackedResource*> transitions[2];
			transitions[0] = SC_Pair(SR_ResourceState_NonPixelSRV, myFrameResources.mySkyProbeIrradianceTexture->GetTextureBuffer());
			transitions[1] = SC_Pair(SR_ResourceState_NonPixelSRV, myFrameResources.mySkyProbeTexture->GetTextureBuffer());
			ctx->Transition(transitions, 2);
		}

		ctx->BindConstantBufferRef(&constants, sizeof(SR_ViewConstants), SR_ConstantBufferRef::SR_ConstantBufferRef_ViewConstants);
		ctx->BindTextureRef(myFrameResources.mySkyProbeTexture, SR_TextureRef::SR_TextureRef_SkyDiffuse);
		ctx->BindTextureRef(myFrameResources.mySkyProbeIrradianceTexture, SR_TextureRef::SR_TextureRef_SkySpecular);
		ctx->BindTextureRef(myFrameResources.mySkyBrdfTexture, SR_TextureRef::SR_TextureRef_SkyBrdf);
	}

	void SG_View::Render_Depth()
	{
		SC_PROFILER_FUNCTION();
		const SG_RenderData& renderData = GetRenderData();
		SR_GraphicsDevice* device = SR_GraphicsDevice::GetDevice();
		SR_GraphicsContext* ctx = SR_GraphicsContext::GetCurrent();
		
		SG_CommonRenderStates* crs = SG_CommonRenderStates::GetInstance();

		renderData.myViewWorldPrepareEvent_MeshesDepth.Wait();

		ctx->InsertWait(renderData.mySetViewRefsEvent);

		SR_PROFILER_FUNCTION();

		SR_Viewport viewport;
		viewport.topLeftX = 0;
		viewport.topLeftY = 0;
		viewport.width = float(device->GetResolution().x);
		viewport.height = float(device->GetResolution().y);
		viewport.minDepth = 0.f;
		viewport.maxDepth = 1.0f;

		SR_ScissorRect scissor;
		scissor.left = 0;
		scissor.top = 0;
		scissor.right = long(device->GetResolution().x);
		scissor.bottom = long(device->GetResolution().y);

		ctx->SetViewport(viewport);
		ctx->SetScissorRect(scissor);

		ctx->Transition(SR_ResourceState_DepthWrite, myFrameResources.myDepth.myTextureBuffer);
		ctx->ClearDepthTarget(myFrameResources.myDepth.myRenderTarget, 0.0f, 0, 0);
		ctx->SetRenderTargets(nullptr, 0, myFrameResources.myDepth.myRenderTarget, 0);

		ctx->SetDepthState(crs->myDepthGreaterEqualWriteState, 0);
		ctx->SetTopology(SR_Topology_TriangleList);

		if (myWorld->myTerrain)
		{
			ctx->SetRasterizerState(crs->myDefaultRasterizerState);
			myWorld->myTerrain->Render(renderData, true);
		}

		renderData.myQueues.myDepth.Render("Render Depth");

		if (mySettings.myUseVoxels)
			myVoxelizer->Voxelize(this);

		ctx->Transition(SR_ResourceState(SR_ResourceState_NonPixelSRV | SR_ResourceState_PixelSRV | SR_ResourceState_DepthRead), myFrameResources.myDepth.myTextureBuffer);
		ctx->BindTextureRef(myFrameResources.myDepth.myTexture, SR_TextureRef::SR_TextureRef_Depth);
	}

	void SG_View::Render_PostDepth()
	{
		SC_PROFILER_FUNCTION();
		const SG_RenderData& renderData = GetRenderData();
		SR_GraphicsContext* ctx = SR_GraphicsContext::GetCurrent();

		ctx->InsertWait(renderData.myRenderDepthEvent);

		SR_PROFILER_FUNCTION();

		ctx->Transition(SR_ResourceState_UnorderedAccess, myFrameResources.myDepthLinear.myTextureBuffer);
		ctx->BindTextureRW(myFrameResources.myDepthLinear.myTextureRW, 0);
		const SC_Vector2f fullResolution = SR_GraphicsDevice::GetDevice()->GetResolution();
		ctx->Dispatch(myLinearizeDepthShader, { (uint)fullResolution.x, (uint)fullResolution.y, 1u });

		ctx->Transition(SR_ResourceState(SR_ResourceState_NonPixelSRV | SR_ResourceState_PixelSRV), myFrameResources.myDepthLinear.myTextureBuffer);
		ctx->BindTextureRef(myFrameResources.myDepthLinear.myTexture, SR_TextureRef::SR_TextureRef_LinearDepth);
	}

	void SG_View::Render_RelightVoxels()
	{
		SC_PROFILER_FUNCTION();
		const SG_RenderData& renderData = GetRenderData();
		SR_GraphicsContext* ctx = SR_GraphicsContext::GetCurrent();
		
		ctx->InsertWait(renderData.myRenderPostDepthEvent);
		ctx->InsertWait(renderData.myRenderLightCullingEvent);

		SR_PROFILER_FUNCTION();

		struct Constants
		{
			uint NumLights;
		} constants;
		constants.NumLights = renderData.myLights.Count();

		ctx->BindConstantBuffer(&constants, sizeof(constants), 1);
		ctx->BindBuffer(myLightsDataView, 0);
		myVoxelizer->RelightVoxelScene(this);
		ctx->BindTextureRef(myVoxelizer->GetVoxelScene(), SR_TextureRef::SR_TextureRef_VoxelScene);
	}

	void SG_View::Render_Shadows()
	{
		SC_PROFILER_FUNCTION();
		const SG_RenderData& renderData = GetRenderData();
		SR_GraphicsContext* ctx = SR_GraphicsContext::GetCurrent();

		renderData.myViewWorldPrepareEvent_MeshesShadow.Wait();
		ctx->InsertWait(renderData.myRenderPostDepthEvent);

		SR_PROFILER_FUNCTION();

		myShadows->RenderShadows(this);

		ctx->BindTextureRef(myShadows->GetSunShadowMap(), SR_TextureRef::SR_TextureRef_ShadowMapCSM);
		ctx->BindTextureRef(myShadows->GetShadowNoiseTexture(), SR_TextureRef::SR_TextureRef_ShadowMapNoise);
	}

	void SG_View::Render_GBuffer()
	{
		SC_PROFILER_FUNCTION();
		const SG_RenderData& renderData = GetRenderData();
		renderData.myViewWorldPrepareEvent_MeshesGbuffer.Wait();

		SR_GraphicsContext* ctx = SR_GraphicsContext::GetCurrent();
		
		SR_GraphicsDevice* device = SR_GraphicsDevice::GetDevice();
		ctx->InsertWait(renderData.myRenderPostDepthEvent);

		SR_PROFILER_FUNCTION();

		ctx->EndQuery(SQueryType_Timestamp, 2);
		{
			SC_Pair<SR_ResourceState, SR_TrackedResource*> pairs[4];
			pairs[0] = SC_Pair(SR_ResourceState_RenderTarget, myFrameResources.myGBuffer.myColors.myTextureBuffer);
			pairs[1] = SC_Pair(SR_ResourceState_RenderTarget, myFrameResources.myGBuffer.myNormals.myTextureBuffer);
			pairs[2] = SC_Pair(SR_ResourceState_RenderTarget, myFrameResources.myGBuffer.myMaterials.myTextureBuffer);
			pairs[3] = SC_Pair(SR_ResourceState_RenderTarget, myFrameResources.myGBuffer.myEmissive.myTextureBuffer);
			ctx->Transition(pairs, 4);
		}
		SR_RenderTarget* gbufferTargets[4];
		gbufferTargets[0] = myFrameResources.myGBuffer.myColors.myRenderTarget;
		gbufferTargets[1] = myFrameResources.myGBuffer.myNormals.myRenderTarget;
		gbufferTargets[2] = myFrameResources.myGBuffer.myMaterials.myRenderTarget;
		gbufferTargets[3] = myFrameResources.myGBuffer.myEmissive.myRenderTarget;

		ctx->ClearRenderTargets(gbufferTargets, 4, SC_Vector4f(0.0f, 0.0f, 0.0f, 0.f));

		if (!myWorld->myTerrain)
		{
			bool shouldReturn = true;
			for (uint i = 0, count = SR_GBufferPermutation_COUNT; i < count; ++i)
			{
				if (renderData.myQueues.myGBuffers[i].myQueueItems.Count() > 0)
				{
					shouldReturn = false;
					break;
				}
			}

			if (shouldReturn)
				return;
		}

		ctx->SetRenderTargets(gbufferTargets, 4, myFrameResources.myDepth.myRenderTarget, 0);

		SR_Viewport viewport;
		viewport.topLeftX = 0;
		viewport.topLeftY = 0;
		viewport.width = float(device->GetResolution().x);
		viewport.height = float(device->GetResolution().y);
		viewport.minDepth = 0.f;
		viewport.maxDepth = 1.0f;

		SR_ScissorRect scissor;
		scissor.left = 0;
		scissor.top = 0;
		scissor.right = long(device->GetResolution().x);
		scissor.bottom = long(device->GetResolution().y);

		ctx->SetViewport(viewport);
		ctx->SetScissorRect(scissor);

		SG_CommonRenderStates* crs = SG_CommonRenderStates::GetInstance();
		ctx->SetDepthState(crs->myDepthEqualReadState, 0);
		ctx->SetTopology(SR_Topology_TriangleList);

		if (myWorld->myTerrain)
		{
			ctx->SetRasterizerState(crs->myDefaultRasterizerState);
			myWorld->myTerrain->Render(renderData);
		}

		for (uint i = 0, count = SR_GBufferPermutation_COUNT; i < count; ++i)
		{
			if (renderData.myQueues.myGBuffers[i].myIsPrepared)
				renderData.myQueues.myGBuffers[i].Render("Render Gbuffer");
		}

		ctx->EndQuery(SQueryType_Timestamp, 3);

		ctx->EndQuery(SQueryType_Timestamp, 16);

		ctx->EndQuery(SQueryType_Timestamp, 17);
		{
			SC_Pair<SR_ResourceState, SR_TrackedResource*> pairs[4];
			pairs[0] = SC_Pair(SR_ResourceState_NonPixelSRV | SR_ResourceState_PixelSRV, myFrameResources.myGBuffer.myColors.myTextureBuffer);
			pairs[1] = SC_Pair(SR_ResourceState_NonPixelSRV | SR_ResourceState_PixelSRV, myFrameResources.myGBuffer.myNormals.myTextureBuffer);
			pairs[2] = SC_Pair(SR_ResourceState_NonPixelSRV | SR_ResourceState_PixelSRV, myFrameResources.myGBuffer.myMaterials.myTextureBuffer);
			pairs[3] = SC_Pair(SR_ResourceState_NonPixelSRV | SR_ResourceState_PixelSRV, myFrameResources.myGBuffer.myEmissive.myTextureBuffer);
			ctx->Transition(pairs, 4);
		}

		ctx->BindTextureRef(myFrameResources.myGBuffer.myColors.myTexture, SR_TextureRef::SR_TextureRef_GBuffer_Color);
		ctx->BindTextureRef(myFrameResources.myGBuffer.myNormals.myTexture, SR_TextureRef::SR_TextureRef_GBuffer_Normal);
		ctx->BindTextureRef(myFrameResources.myGBuffer.myMaterials.myTexture, SR_TextureRef::SR_TextureRef_GBuffer_ARM);
		ctx->BindTextureRef(myFrameResources.myGBuffer.myEmissive.myTexture, SR_TextureRef::SR_TextureRef_GBuffer_Optional_Emission);

	}
	void SG_View::Render_AmbientOcclusion()
	{
		SC_PROFILER_FUNCTION();
		const SG_RenderData& renderData = GetRenderData();
		SR_GraphicsContext* ctx = SR_GraphicsContext::GetCurrent();
		
		ctx->InsertWait(renderData.myRenderGBufferEvent);
		ctx->InsertWait(renderData.myUpdateRTSceneEvent);
		SR_PROFILER_FUNCTION();
		SR_PROFILER_FUNCTION_TAGGED(std::to_string(renderData.myFrameIndex).c_str());


		if (mySettings.myEnableRaytracing && myAmbientOcclusionSettings.myType == AmbientOcclusionSettings::RTAO)
		{
			if (myWorld->myRaytracer)
			{
				struct Constants
				{
					float myRadius;
					uint myNumRaysPerPixel;
				} constants;

				constants.myRadius = myAmbientOcclusionSettings.myRTAORadius;
				constants.myNumRaysPerPixel = myAmbientOcclusionSettings.myRTAONumRaysPerPixel;
				ctx->BindConstantBuffer(&constants, sizeof(constants), 0);

				myWorld->myRaytracer->TraceAmbientOcclusion();
				ctx->BindTextureRef(myWorld->myRaytracer->GetResultTexture(), SR_TextureRef::SR_TextureRef_AmbientOcclusion);
			}

			return;
		}

		struct SSAO_Settings
		{
			float SamplingRadius;
			float Scale;
			float Bias;
			float Intensity;
		} ssaoSettings;
		ssaoSettings.SamplingRadius = myAmbientOcclusionSettings.mySSAOSamplingRadius;
		ssaoSettings.Scale = myAmbientOcclusionSettings.mySSAOScale;
		ssaoSettings.Bias = myAmbientOcclusionSettings.mySSAOBias;
		ssaoSettings.Intensity = myAmbientOcclusionSettings.mySSAOIntensity;

		ctx->Transition(SR_ResourceState_UnorderedAccess, myFrameResources.myAmbientOcclusion.myTextureBuffer);

		ctx->BindConstantBuffer(&ssaoSettings, sizeof(ssaoSettings), 0);
		ctx->BindTextureRef(myFrameResources.myGBuffer.myNormals.myTexture, SR_TextureRef::SR_TextureRef_GBuffer_Normal);
		ctx->BindTexture(myAmbientOcclusionSettings.mySSAORand, 0);
		ctx->BindTextureRW(myFrameResources.myAmbientOcclusion.myTextureRW, 0);

		const SC_Vector2f fullResolution = SR_GraphicsDevice::GetDevice()->GetResolution();
		ctx->Dispatch(myAmbientOcclusionSettings.myShader, { (uint)fullResolution.x, (uint)fullResolution.y, 1u });
		ctx->EndQuery(SQueryType_Timestamp, 4);
		ctx->EndQuery(SQueryType_Timestamp, 5);

		ctx->Transition(SR_ResourceState_Common, myFrameResources.myAmbientOcclusion.myTextureBuffer);
		ctx->BindTextureRef(myFrameResources.myAmbientOcclusion.myTexture, SR_TextureRef::SR_TextureRef_AmbientOcclusion);
	}

	void SG_View::Render_LightCulling()
	{
		SC_PROFILER_FUNCTION();
		const SG_RenderData& renderData = GetRenderData();
		SR_GraphicsContext* ctx = SR_GraphicsContext::GetCurrent();
		renderData.myViewWorldPrepareEvent_Lights.Wait();
		
		ctx->InsertWait(renderData.myRenderPostDepthEvent);

		SR_GraphicsDevice* device = SR_GraphicsDevice::GetDevice();
		const SC_Vector2f fullResolution = device->GetResolution();

		SR_PROFILER_FUNCTION();
		if (renderData.myLights.Count())
		{
			ctx->UpdateBufferData(renderData.myLights.GetByteSize(), renderData.myLights.GetBuffer(), myLightsData);
			//SC_LOG("Light0 pos (x: %.3f y: %.3f z: %.3f)", renderData.myLights[0].myPositionAndRange.x, renderData.myLights[0].myPositionAndRange.y, renderData.myLights[0].myPositionAndRange.z);
		}

		SR_BufferViewDesc lightsDataViewDesc;
		lightsDataViewDesc.myFirstElement = 0;
		lightsDataViewDesc.myNumElements = SC_Max(renderData.myLights.Count(), 1);
		lightsDataViewDesc.myType = SR_BufferViewType_Structured;
		lightsDataViewDesc.myIsShaderWritable = false;
		myLightsDataView = device->CreateBufferView(lightsDataViewDesc, myLightsData);

		struct Constants
		{
			uint NumLights;
		} constants;
		constants.NumLights = renderData.myLights.Count();

		{
			uint numTransitions = (myDebugLightHeatmap) ? 3 : 2;
			SC_Pair<SR_ResourceState, SR_TrackedResource*> pairs[3];
			pairs[0] = SC_Pair(SR_ResourceState_NonPixelSRV, myLightsData);
			pairs[1] = SC_Pair(SR_ResourceState_UnorderedAccess, myLightCullingDataRW->GetBuffer());
			pairs[2] = SC_Pair(SR_ResourceState_UnorderedAccess, myDebugTileHeatmap.myTextureBuffer);
			ctx->Transition(pairs, numTransitions);
		}

		ctx->BindConstantBuffer(&constants, sizeof(constants), 0);
		ctx->BindBuffer(myLightsDataView, 0);
		ctx->BindBufferRW(myLightCullingDataRW, 0);
		if (myDebugLightHeatmap)
		{
			ctx->BindTextureRW(myDebugTileHeatmap.myTextureRW, 0);
			ctx->Dispatch(myLighting.myCullingShaderDebug, (uint)fullResolution.x, (uint)fullResolution.y);
		}
		else
			ctx->Dispatch(myLighting.myCullingShader, (uint)fullResolution.x, (uint)fullResolution.y);

		ctx->Transition(SR_ResourceState_Common, myLightCullingData->GetBuffer());
		ctx->BindBufferRef(myLightCullingData, SR_BufferRef::SR_BufferRef_LightTileData);
	}

	void SG_View::Render_DeferredLights()
	{
		SC_PROFILER_FUNCTION();
		const SG_RenderData& renderData = GetRenderData();
		SR_GraphicsContext* ctx = SR_GraphicsContext::GetCurrent();
		
		ctx->InsertWait(renderData.myRenderShadowsEvent);
		ctx->InsertWait(renderData.myRenderSkyProbeEvent);
		ctx->InsertWait(renderData.myRenderGBufferEvent);
		ctx->InsertWait(renderData.myRenderAmbientOcclusionEvent);
		ctx->InsertWait(renderData.myRenderRelightVoxelsEvent);
		ctx->InsertWait(renderData.myRenderLightCullingEvent);

		SR_PROFILER_FUNCTION();
		ctx->EndQuery(SQueryType_Timestamp, 6);

		const SC_Float2 fullResolution = SR_GraphicsDevice::GetDevice()->GetResolution();

		if (mySettings.myRenderSky)
			RenderSky();
		
		if (!mySettings.myEnableAmbientOcclusion)
			ctx->BindTextureRef(SR_RenderInterface::ourWhite4x4, SR_TextureRef::SR_TextureRef_AmbientOcclusion);

		ctx->BindTextureRW(myFrameResources.myFullscreenHDR.myTextureRW, 0);
		ctx->BindBuffer(myLightsDataView, 0);

		myVoxelizer->SetVoxelConstants();

		if (myDebugLightHeatmap)
		{
			ctx->BindTexture(myDebugTileHeatmap.myTexture, 0);
			ctx->SetShaderState(myLighting.myShaderDebug);
		}
		else
			ctx->SetShaderState(myLighting.myShader);


		ctx->Dispatch((uint)ceil(fullResolution.x / 16.f), (uint)ceil(fullResolution.y / 16.f), 1);

		ctx->EndEvent();
		ctx->EndQuery(SQueryType_Timestamp, 7);
	}

	void SG_View::Render_Forward()
	{
		SC_PROFILER_FUNCTION();
		const SG_RenderData& renderData = GetRenderData();

		SR_GraphicsContext* ctx = SR_GraphicsContext::GetCurrent();
		
		SR_GraphicsDevice* device = SR_GraphicsDevice::GetDevice();
		SG_CommonRenderStates* crs = SG_CommonRenderStates::GetInstance();
		renderData.myViewWorldPrepareEvent_MeshesForward.Wait();

		if (renderData.myQueues.myForwardOpaque.myQueueItems.Empty() && renderData.myQueues.myForwardTransparent.myQueueItems.Empty())
			return;

		ctx->InsertWait(renderData.myRenderDeferredLightsEvent);

		SR_PROFILER_FUNCTION();

		SR_Viewport viewport;
		viewport.topLeftX = 0;
		viewport.topLeftY = 0;
		viewport.width = float(device->GetResolution().x);
		viewport.height = float(device->GetResolution().y);
		viewport.minDepth = 0.f;
		viewport.maxDepth = 1.0f;

		SR_ScissorRect scissor;
		scissor.left = 0;
		scissor.top = 0;
		scissor.right = long(device->GetResolution().x);
		scissor.bottom = long(device->GetResolution().y);

		ctx->SetViewport(viewport);
		ctx->SetScissorRect(scissor);

		ctx->Transition(SR_ResourceState_RenderTarget, myFrameResources.myFullscreenHDR.myTextureBuffer);

		SR_RenderTarget* renderTarget = myFrameResources.myFullscreenHDR.myRenderTarget;
		ctx->SetRenderTargets(&renderTarget, 1, myFrameResources.myDepth.myRenderTarget, 0);
		ctx->SetDepthState(crs->myDepthEqualReadState, 0);

		renderData.myQueues.myForwardOpaque.Render("Forward Opaque");

		ctx->SetDepthState(crs->myDepthGreaterEqualReadState, 0);
		renderData.myQueues.myForwardTransparent.Render("Forward Transparent");

		ctx->Transition(SR_ResourceState(SR_ResourceState_NonPixelSRV | SR_ResourceState_PixelSRV), myFrameResources.myFullscreenHDR.myTextureBuffer);
	}

	void SG_View::Render_PostFX()
	{
		SC_PROFILER_FUNCTION();
		const SG_RenderData& renderData = GetRenderData();
		SR_GraphicsContext* ctx = SR_GraphicsContext::GetCurrent();

		ctx->InsertWait(renderData.myRenderDeferredLightsEvent);
		ctx->InsertWait(renderData.myRenderForwardEvent);

		SR_PROFILER_FUNCTION();
		ctx->EndQuery(SQueryType_Timestamp, 8);
		{
			ctx->EndQuery(SQueryType_Timestamp, 10);
			myPostEffects.Render(this);
			ctx->EndQuery(SQueryType_Timestamp, 11);
		}

		ctx->EndQuery(SQueryType_Timestamp, 12);
		if (myPostFX.myUseFXAA)
		{
			SR_PROFILER_FUNCTION_TAGGED("FXAA");

			SR_GraphicsDevice* device = SR_GraphicsDevice::GetDevice();
			
			ctx->Transition(SR_ResourceState_GenericRead, myFrameResources.myFullscreen.myTextureBuffer);
			ctx->Transition(SR_ResourceState_RenderTarget, myFrameResources.myFullscreen2.myTextureBuffer);
			SR_Viewport viewport;
			viewport.topLeftX = 0;
			viewport.topLeftY = 0;
			viewport.width = float(device->GetResolution().x);
			viewport.height = float(device->GetResolution().y);
			viewport.minDepth = 0.f;
			viewport.maxDepth = 1.0f;

			SR_ScissorRect scissor;
			scissor.left = 0;
			scissor.top = 0;
			scissor.right = long(device->GetResolution().x);
			scissor.bottom = long(device->GetResolution().y);

			ctx->SetScissorRect(scissor);
			ctx->SetViewport(viewport);
			ctx->SetVertexBuffer(0, myPostFX.myFullscreenQuadBuffer);
			ctx->SetIndexBuffer(myPostFX.myFullscreenQuadIBuffer);
			SR_RenderTarget* rt = myFrameResources.myFullscreen2.myRenderTarget;
			ctx->SetRenderTargets(&rt, 1, nullptr, 0);
			ctx->SetTopology(SR_Topology_TriangleList);
			ctx->BindTexture(myFrameResources.myFullscreen.myTexture, 0);
			//ctx->BindGraphicsPSO(*myPostFX.myAAPSO);
			ctx->DrawIndexed(6, 0, 0);

			myFrameResources.myLastTextureWritten = myFrameResources.myFullscreen2;
		}
		ctx->EndQuery(SQueryType_Timestamp, 13);
		ctx->ResolveQuery(SQueryType_Timestamp, 0, 18, myTimestampBuffer);
	}

	void SG_View::Render_UI()
	{
		SC_PROFILER_FUNCTION();
		SR_GraphicsContext* ctx = SR_GraphicsContext::GetCurrent();
		
		const SG_RenderData& renderData = GetRenderData();

		ctx->InsertWait(renderData.myRenderPostFXEvent);

		SR_PROFILER_FUNCTION_TAGGED("UI");

		SR_GraphicsDevice* device = SR_GraphicsDevice::GetDevice();

		ctx->SetScissorRect(SR_ScissorRect(device->GetResolution()));
		ctx->SetViewport(SR_Viewport(device->GetResolution()));

		ctx->Transition(SR_ResourceState_RenderTarget, myFrameResources.myLastTextureWritten.myTextureBuffer);
		SR_RenderTarget* rt = myFrameResources.myLastTextureWritten.myRenderTarget;
		ctx->SetRenderTargets(&rt, 1, myFrameResources.myDepth.myRenderTarget, 0);

		RenderDebugPrimitives(); 
	}

	void SG_View::RenderSky()
	{
		SR_PROFILER_FUNCTION_TAGGED("Render Sky");
		myWorld->mySky->Render(this);
	}

	void SG_View::RenderDebugPrimitives()
	{
		SR_PROFILER_FUNCTION_TAGGED("Render Debug Primitives");
#if ENABLE_DEBUG_PRIMITIVES
		SG_DebugPrimitives::Get()->RenderPrimitives(this);
#endif
	}
}