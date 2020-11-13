#pragma once
#include "SR_GraphicsDefinitions.h"
#include "SR_RenderEnums.h"
#include "SR_GraphicsEngineEnums.h"
#include "SR_GraphicsResources.h"
#include "SR_RenderThreadPool.h"

#define GPU_RESOURCE_HEAP_CBV_COUNT	14
#define GPU_RESOURCE_HEAP_SRV_COUNT	64
#define GPU_RESOURCE_HEAP_UAV_COUNT	64
#define GPU_RESOURCE_MAX_COUNT_PER_CONTEXT (GPU_RESOURCE_HEAP_CBV_COUNT + GPU_RESOURCE_HEAP_SRV_COUNT + GPU_RESOURCE_HEAP_UAV_COUNT)

#define GPU_SAMPLER_HEAP_COUNT	16

namespace Shift
{
	enum ERootParamIndex : unsigned char
	{
		CBV = 0,
		SRV,
		UAV,
		Sampler,
	};

	class SR_Texture;
	class SR_WaitEvent;
	class SR_GraphicsContext;
	class SC_Window;
	class SR_SwapChain;
	class SR_ShaderCompiler;

	struct SFeatureSupport
	{
		SFeatureSupport()
			: myEnableConservativeRaster(false)
			, myEnableTiledResources(false)
			, myEnableRaytracing(false)
			, myEnableRenderPass(false)
			, myEnableTypedLoadAdditionalFormats(false)
			, myEnableMeshShaders(false)
			, myEnableVariableRateShading(false)
			, myResourceBindingTier(1)
			, myTiledResourcesTier(0)
			, myResourceHeapTier(1)
			, myConservativeRasterTier(0)
			, myRaytracingTier(0)
			, myRenderPassTier(0)
			, myMeshShaderTier(0)
			, myVariableRateShadingTier(0)
		{}

		uint myResourceBindingTier;
		uint myTiledResourcesTier;
		uint myResourceHeapTier;
		uint myConservativeRasterTier;
		uint myRaytracingTier;
		uint myRenderPassTier;
		uint myMeshShaderTier;
		uint myVariableRateShadingTier;
		bool myEnableTiledResources : 1;
		bool myEnableConservativeRaster : 1;
		bool myEnableRaytracing : 1;
		bool myEnableRenderPass : 1;
		bool myEnableTypedLoadAdditionalFormats : 1;
		bool myEnableMeshShaders : 1;
		bool myEnableVariableRateShading : 1;
	};

	struct SR_GraphicsDeviceStats
	{
		uint myNumDescriptorsUsedCPU;
		uint myNumDescriptorsUsedGPU;
		uint myNumDescriptorsUsedStaged;
		uint myNumTrianglesDrawn;
		uint myNumDrawCalls;
		uint myNumDispatchCalls;
		uint myNumInstancesDrawn;
	};

	class SR_GraphicsDevice
	{
	public:
		static void Destroy();

		SR_GraphicsDevice();
		virtual ~SR_GraphicsDevice();

		virtual const bool Init(SC_Window* aWindow) = 0;
		virtual void BeginFrame() = 0;
		virtual void EndFrame() = 0;

		virtual void Present() = 0;

		virtual SR_GraphicsContext* GetContext(const SR_ContextType& aType) = 0;
		void InactivateContext(SR_GraphicsContext* aCtx);

#if SR_ENABLE_RAYTRACING
		virtual void SetRaytracingInstanceData(SR_RaytracingInstanceData& aOutInstance, SR_Buffer* aMeshBuffer, const SC_Matrix44& aMatrix, uint aInstanceData, uint8 aMask, uint8 aHitGroup, bool aIsOpaque, SR_RasterizerFaceCull aCullMode) = 0;

#endif

		//////////////////////
		// Resources

		// Textures
		virtual SC_Ref<SR_TextureBuffer> CreateTextureBuffer(const SR_TextureBufferDesc& aDesc, const char* aID = nullptr) = 0;

		virtual SC_Ref<SR_Texture> CreateTexture(const SR_TextureDesc& aTextureDesc, SR_TextureBuffer* aTextureBuffer) = 0;
		virtual SC_Ref<SR_Texture> CreateRWTexture(const SR_TextureDesc& aTextureDesc, SR_TextureBuffer* aTextureBuffer) = 0;
		virtual SC_Ref<SR_Texture> GetCreateTexture(const char* aFile) = 0;

		virtual SC_Ref<SR_RenderTarget> CreateRenderTarget(const SR_RenderTargetDesc& aRenderTargetDesc, SR_TextureBuffer* aTextureBuffer) = 0;
		virtual SC_Ref<SR_RenderTarget> CreateDepthStencil(const SR_RenderTargetDesc& aRenderTargetDesc, SR_TextureBuffer* aTextureBuffer) = 0;

		// Buffers
		virtual SC_Ref<SR_Buffer> CreateBuffer(const SR_BufferDesc& aBufferDesc, void* aInitialData = nullptr, const char* aIdentifier = nullptr) = 0;
		virtual SC_Ref<SR_BufferView> CreateBufferView(const SR_BufferViewDesc& aViewDesc, SR_Buffer* aBuffer) = 0;

		// Shaders
		virtual SC_Ref<SR_ShaderState> CreateShaderState(const SR_ShaderStateDesc& aShaderStateDesc) = 0;

		virtual void GenerateMips(SR_TextureBuffer* aTextureBuffer) = 0;

		virtual const SC_Vector2f& GetResolution() const = 0;
		virtual SR_GraphicsDeviceStats GetStats() const = 0;

		SC_Ref<SR_Waitable> PostRenderTask(std::function<void()> aTask, uint aWaitMode = SR_WaitMode_CPU);
		SC_Ref<SR_Waitable> PostComputeTask(std::function<void()> aTask, uint aWaitMode = SR_WaitMode_CPU);
		SC_Ref<SR_Waitable> PostCopyTask(std::function<void()> aTask, uint aWaitMode = SR_WaitMode_CPU);
		bool IsRenderPoolIdle() const;

		bool AddToReleasedResourceCache(SR_Resource* aResource, uint64 aFrameRemoved);


		const SR_GraphicsAPI& APIType() const;
		uint GetDispatchCallCount() const { return myNumDispatchCalls; }

		SR_SwapChain* GetSwapChain() const { return mySwapChain; }
		SR_RenderThreadPool& GetThreadPool() { return myRenderThreadPool; }
		SR_ShaderCompiler* GetShaderCompiler() { return myShaderCompiler; }
		const SFeatureSupport& GetFeatureSupport() { return myFeatureSupport; }

		static SR_GraphicsDevice* GetDevice() { return ourGraphicsDevice; };
		static uint GetUsedVRAM() { return uint(ourGraphicsDevice->myUsedVRAM * 0.000001f); }
		uint GetDrawCallCount() const { return myNumDrawCalls; }
		static uint GetNumTrianglesDrawn() { return ourGraphicsDevice->myNumTrianglesDrawn; }
		static uint64 ourLastCompletedFrame;
	protected:
		virtual const bool PostInit();
		virtual void SetFeatureSupport() = 0;
		virtual void InactivateContextInternal(SR_GraphicsContext* aCtx) = 0;
		void ReleaseResources();

		static SR_GraphicsDevice* ourGraphicsDevice;

		struct RemovedResource 
		{
			RemovedResource(){}
			RemovedResource(SR_Resource* aResource, uint64 aFrameRemoved) : myResource(aResource), myFrameRemoved(aFrameRemoved) {}
			SR_Resource* myResource;
			uint64 myFrameRemoved;

			bool operator==(const RemovedResource& aOther)
			{
				if (myResource == aOther.myResource &&
					myFrameRemoved == aOther.myFrameRemoved)
					return true;

				return false;
			}
		}; 
		SC_Mutex myReleaseResourcesMutex;
		SC_GrowingArray<RemovedResource> myReleasedResourceCache;
		SC_GrowingArray<RemovedResource> myReleasedResourceCacheDupl;

		// Subsystems
		SR_RenderThreadPool myRenderThreadPool;
		SC_UniquePtr<SR_SwapChain> mySwapChain;
		SC_UniquePtr<SR_ShaderCompiler> myShaderCompiler;

		SR_GraphicsAPI myAPIType;
		uint myUsedVRAM;

		uint myNumDescriptorsAllocatedThisFrame;
		volatile uint myNumTrianglesDrawn;
		volatile uint myNumDrawCalls;
		volatile uint myNumDispatchCalls;
		volatile uint myNumInstancesDrawn;

		SFeatureSupport myFeatureSupport;
		bool myEnableDebugging : 1;
		bool myDebugBreakOnError : 1;
		bool myDebugBreakOnWarning : 1;
		bool myIsUsing16BitDepth : 1;
	};
}
