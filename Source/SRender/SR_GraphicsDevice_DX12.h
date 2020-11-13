#pragma once
#if ENABLE_DX12
#include "SC_Mutex.h"
#include "SR_GraphicsDevice.h"
#include "SR_DescriptorHeap_DX12.h"
#include "SR_DescriptorRing_DX12.h"
#include "SR_GraphicsContext_DX12.h"

struct ID3D12Device;
struct ID3D12Device5;
struct IDXGISwapChain4;
struct ID3D12CommandQueue;
struct ID3D12DescriptorHeap;
struct ID3D12Resource;
struct ID3D12CommandAllocator;
struct ID3D12GraphicsCommandList;
struct ID3D12GraphicsCommandList4;
struct ID3D12Fence;
struct ID3D12RootSignature;
struct ID3D12ShaderReflection;
struct D3D12_STATIC_SAMPLER_DESC;
struct D3D12_CPU_DESCRIPTOR_HANDLE;
struct D3D12_GPU_DESCRIPTOR_HANDLE;
namespace Shift
{
	struct SR_GlobalResourcesCache_DX12
	{
		void UpdateResources(const SR_ResourceBindings& aBindings, const SR_BoundResources_DX12& aBoundResources)
		{
			for (uint i = SC_FindFirstBit(aBindings.myGlobalConstants); i < SR_ConstantBufferRef_NumGlobals; ++i)
			{
				if (aBindings.myGlobalConstants & (SR_GlobalConstantsMask(1) << i))
					myGlobalConstants[i] = aBoundResources.myGlobalConstants[i];
			}
			for (uint i = SC_FindFirstBit(aBindings.myGlobalTextures); i < SR_TextureRef_NumGlobals; ++i)
			{
				if (aBindings.myGlobalTextures & (SR_GlobalTexturesMask(1) << i))
					myGlobalTextures[i] = aBoundResources.myGlobalTextures[i];
			}
			for (uint i = SC_FindFirstBit(aBindings.myGlobalBuffers); i < SR_BufferRef_NumGlobals; ++i)
			{
				if (aBindings.myGlobalBuffers & (SR_GlobalBuffersMask(1) << i))
					myGlobalBuffers[i] = aBoundResources.myGlobalBuffers[i];
			}
			for (uint i = SC_FindFirstBit(aBindings.myGlobalTextureRWs); i < SR_TextureRWRef_NumGlobals; ++i)
			{
				if (aBindings.myGlobalTextureRWs & (SR_GlobalTexturesRWMask(1) << i))
					myGlobalTextureRWs[i] = aBoundResources.myGlobalTextureRWs[i];
			}
			for (uint i = SC_FindFirstBit(aBindings.myGlobalBufferRWs); i < SR_BufferRWRef_NumGlobals; ++i)
			{
				if (aBindings.myGlobalBufferRWs & (SR_GlobalBuffersRWMask(1) << i))
					myGlobalBufferRWs[i] = aBoundResources.myGlobalBufferRWs[i];
			}
		}

		SR_DescriptorCPU_DX12 myGlobalConstants[SR_ConstantBufferRef_NumGlobals];
		SR_DescriptorCPU_DX12 myGlobalTextures[SR_TextureRef_NumGlobals];
		SR_DescriptorCPU_DX12 myGlobalBuffers[SR_BufferRef_NumGlobals];
		SR_DescriptorCPU_DX12 myGlobalTextureRWs[SR_TextureRWRef_NumGlobals];
		SR_DescriptorCPU_DX12 myGlobalBufferRWs[SR_BufferRWRef_NumGlobals];
	};

	class SC_Window_Win64;
	class SR_RootSignature_DX12;
	class SR_GraphicsDevice_DX12 final : public SR_GraphicsDevice
	{
	public:
		SR_GraphicsDevice_DX12();
		~SR_GraphicsDevice_DX12();

		const bool Init(SC_Window* aWindow) override;
		void BeginFrame() override;
		void EndFrame() override;

		void Present() override;

		SR_GraphicsContext* GetContext(const SR_ContextType& aType) override;

#if SR_ENABLE_RAYTRACING

		void SetRaytracingInstanceData(SR_RaytracingInstanceData& aOutInstance, SR_Buffer* aMeshBuffer, const SC_Matrix44& aMatrix, uint aInstanceData, uint8 aMask, uint8 aHitGroup, bool aIsOpaque, SR_RasterizerFaceCull aCullMode) override;

#endif

		//////////////////////
		// Resources

		// Textures
		SC_Ref<SR_TextureBuffer> CreateTextureBuffer(const SR_TextureBufferDesc& aDesc, const char* aID = nullptr) override; 

		SC_Ref<SR_Texture> CreateTexture(const SR_TextureDesc& aTextureDesc, SR_TextureBuffer* aTextureBuffer) override;
		SC_Ref<SR_Texture> CreateRWTexture(const SR_TextureDesc& aTextureDesc, SR_TextureBuffer* aTextureBuffer) override;
		SC_Ref<SR_Texture> GetCreateTexture(const char* aFile) override;

		SC_Ref<SR_RenderTarget> CreateRenderTarget(const SR_RenderTargetDesc& aRenderTargetDesc, SR_TextureBuffer* aTextureBuffer) override;
		SC_Ref<SR_RenderTarget> CreateDepthStencil(const SR_RenderTargetDesc& aRenderTargetDesc, SR_TextureBuffer* aTextureBuffer) override;

		// Buffers
		SC_Ref<SR_Buffer> CreateBuffer(const SR_BufferDesc& aBufferDesc, void* aInitialData = nullptr, const char* aIdentifier = nullptr) override;
		SC_Ref<SR_BufferView> CreateBufferView(const SR_BufferViewDesc& aViewDesc, SR_Buffer* aBuffer) override;

		   
		// Shaders
		SC_Ref<SR_ShaderState> CreateShaderState(const SR_ShaderStateDesc& aShaderStateDesc) override;
		bool ReflectShaderVertexLayout(ID3D12ShaderReflection* aReflection, SR_VertexLayout* aOutLayout);
		bool ReflectShaderVertexLayout(void* aVertexShaderByteCode, uint aByteCodeSize, SR_VertexLayout* aOutLayout);

		void GenerateMips(SR_TextureBuffer* aTextureBuffer) override;

		const SC_Vector2f& GetResolution() const override; 
		SR_GraphicsDeviceStats GetStats() const override;


		// D3D12 Specific
		SR_Descriptor_DX12 AllocateDescriptorRange(uint aNumDescriptors, bool aIsStagingRange, SR_GraphicsContext* aCtx);
		void GetLatestGlobalResources(SR_GraphicsContext_DX12* aCtx);
		SR_DescriptorHeap_DX12& GetGPUVisibleDescriptorHeap();

		static SR_GraphicsDevice_DX12* GetDX12Device();


		SR_DescriptorCPU_DX12 GetStagedHandle() const;
		void CopyDescriptors(uint aNumDescriptors, D3D12_CPU_DESCRIPTOR_HANDLE aHandleDest, D3D12_CPU_DESCRIPTOR_HANDLE aHandleSrc);
		ID3D12Device* GetNativeDevice();
		ID3D12Device5* GetNativeDevice5();
		SR_RootSignature_DX12* GetGraphicsRootSignature();
		SR_RootSignature_DX12* GetComputeRootSignature();
		ID3D12GraphicsCommandList* GetCommandList();
		ID3D12QueryHeap* GetQueryHeap();

		SR_DescriptorCPU_DX12 myNullCBV;
		SR_DescriptorCPU_DX12 myNullBufferUAV;
		SR_DescriptorCPU_DX12 myNullTexture2DUAV;
		SR_DescriptorCPU_DX12 myNullTexture2DSRV;
		SR_DescriptorCPU_DX12 myNullByteBufferSRV;
	private:
		const bool PostInit() override;
		void SetFeatureSupport() override;
		void InactivateContextInternal(SR_GraphicsContext* aCtx) override;
		bool CreateResourceHeaps();
		void GetStaticSamplerDesc(D3D12_STATIC_SAMPLER_DESC (&aSamplersOut)[SAMPLERSLOT_COUNT]) const;
		void CreateDefaultRootSignatures();
		void InitNullDescriptors();

		static SR_GraphicsDevice_DX12* ourGraphicsDevice_DX12;

		// Contexts
		SC_Mutex myContextsMutex;
		SC_GrowingArray<SC_Ref<SR_GraphicsContext_DX12>> myInactiveContexts[SR_ContextType::SR_ContextType_COUNT];

		SC_Mutex myGlobalResourcesMutex;
		SR_GlobalResourcesCache_DX12 myGlobalResourcesCache;

		SC_Future<void> myReleaseResourcesFuture;
		SC_GrowingArray<SC_Pair<SC_GrowingArray<ID3D12Resource*>, SR_Fence>> myDeletedResources;
		SC_Ref<SR_GraphicsContext> myContext_KeepAlive;
		SR_GraphicsContext_DX12* myContext;

		SC_Window_Win64* myWindow;

		D3D_FEATURE_LEVEL myFeatureLevel;
		ID3D12Device* myDevice;
		ID3D12Device5* myDevice5;

		SC_Ref<SR_RootSignature_DX12> myRootSignatures[SR_RootSignatureType_DX12_COUNT];

		// Resource Heap
		SR_DescriptorHeap_DX12 myResourceHeapCPU;
		SR_DescriptorHeap_DX12 myResourceHeapStagedCPU;
		SR_DescriptorHeap_DX12 myResourceHeapGPU;
		SR_DescriptorRing_DX12 myDescriptorRingGPU;
		SR_DescriptorRing_DX12 myDescriptorRingCPU;
		mutable SC_Mutex myGPUDescriptorsLock;
		mutable SC_Mutex myCPUStagingDescriptorsLock;
		uint myGPUDescriptorsOffset;
		uint myCPUStagingDescriptorsOffset;

		SR_DescriptorHeap_DX12 myRTVHeap;
		SR_DescriptorHeap_DX12 myDSVHeap;

		// Query
		static constexpr uint ourNumQueriesPerContext = 128;
		ID3D12QueryHeap* myQueryHeap;

		// Debug Interfaces
		ID3D12InfoQueue* myInfoQueue;

		uint myResourceDescriptorIncrementSize;
		uint mySamplerDescriptorIncrementSize;

		bool myIsUsingPlacementHeaps;
		bool myForceCompileDXBC : 1;
	};
}

#endif