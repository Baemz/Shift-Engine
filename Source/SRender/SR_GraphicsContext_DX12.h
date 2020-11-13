#pragma once
#if ENABLE_DX12
#include "SR_RingBuffer_DX12.h"
#include "SR_RootSignature_DX12.h"
#include "SR_DescriptorHeap_DX12.h"
#include "SR_ResourceBindings_DX12.h"

struct D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS;

namespace Shift
{
	class SR_GraphicsDevice_DX12;
	class SR_CommandList_DX12;
	class SR_Buffer_DX12;

	class SR_GraphicsContext_DX12 final : public SR_GraphicsContext
	{
		friend class SR_GraphicsDevice_DX12;
		friend class SR_QueueManager_DX12;
	public:

		SR_GraphicsContext_DX12(const SR_ContextType& aType, SR_GraphicsDevice_DX12* aDevice);
		~SR_GraphicsContext_DX12();

		void Begin() override;
		void End() override;

		void BeginRecording() override;
		void EndRecording() override;

		void BeginEvent(const char* aID) override;
		void EndEvent() override;

		void InsertWait(SR_Waitable* aEvent) override;

		void Draw(const unsigned int aVertexCount, const unsigned int aStartVertexLocation) override;
		void DrawIndexed(const unsigned int aIndexCount, const unsigned int aStartIndexLocation, const unsigned int aBaseVertexLocation) override;
		void DrawInstanced(const unsigned int aVertexCount, const unsigned int aInstanceCount, const unsigned int aStartVertexLocation, const unsigned int aStartInstanceLocation) override;
		void DrawIndexedInstanced(const unsigned int aIndexCount, const unsigned int aInstanceCount, const unsigned int aStartIndexLocation, const unsigned int aBaseVertexLocation, const unsigned int aStartInstanceLocation) override;

		void Dispatch(const unsigned int aThreadGroupCountX, const unsigned int aThreadGroupCountY = 1, const unsigned int aThreadGroupCountZ = 1) override;

		void ClearRenderTarget(SR_RenderTarget* aTarget, const SC_Vector4f& aClearColor) override;
		void ClearDepthTarget(SR_RenderTarget* aTarget, float aDepthValue, uint8 aStencilValue, uint aClearFlags) override;

		//////////////////////////
		// Constant Buffers
		void BindConstantBuffer(SR_Buffer* aBuffer, const uint aBindingIndex) override;
		void BindConstantBuffer(void* aData, uint aSize, const uint aBindingIndex) override;
		void BindConstantBufferRef(SR_Buffer* aBuffer, const SR_ConstantBufferRef::Enumerated& aRef) override;
		void BindConstantBufferRef(void* aData, uint aSize, const SR_ConstantBufferRef::Enumerated& aRef) override;

		//////////////////////////
		// Buffers
		void BindBuffer(SR_BufferView* aBufferView, const uint aBindingIndex) override;
		void BindBufferRef(SR_BufferView* aBufferView, const SR_BufferRef::Enumerated& aRef) override;
		void BindBufferRW(SR_BufferView* aBufferView, const uint aBindingIndex) override;
		void BindBufferRWRef(SR_BufferView* aBufferView, const SR_BufferRWRef::Enumerated& aRef) override;

		//////////////////////////
		// Textures
		void BindTexture(SR_Texture* aTexture, const uint aBindingIndex) override;
		void BindTextureRef(SR_Texture* aTexture, const SR_TextureRef::Enumerated& aRef) override;

		void BindTextureRW(SR_Texture* aTexture, const uint aBindingIndex) override;
		void BindTextureRWRef(SR_Texture* aTexture, const SR_TextureRWRef::Enumerated& aRef) override;

#if SR_ENABLE_RAYTRACING
		//////////////////////////
		// Acceleration Structure

		SC_Ref<SR_Buffer> CreateRTMesh(SR_Buffer* aBuffer, bool aFastUpdate, SR_Buffer* aVertexBuffer, uint aVertexStride, uint aNumVertices, SR_Buffer* aIndexBuffer, uint aNumIndices, const SC_Matrix44* aTransform) override;
		SC_Ref<SR_BufferView> BuildRTScene(SR_BufferView* aBufferView, const SR_RaytracingInstanceData* aInstances, uint aNumInstances) override;
#endif

		void BindRootSignature(SR_RootSignature_DX12* aRootSignature);

		void Transition(const SR_ResourceState& aTransition, SR_TrackedResource* aResource) override;
		void Transition(const SC_Pair<SR_ResourceState, SR_TrackedResource*>* aPairs, uint aCount) override;
		void BarrierUAV(SR_TrackedResource* aResource) override;
		void BarrierUAV(SR_TrackedResource** aResources, uint aCount) override;
		void BarrierAlias(SR_TrackedResource* aBefore, SR_TrackedResource* aAfter) override;
		void BarrierAlias(const SC_Pair<SR_TrackedResource*, SR_TrackedResource*>* aPairs, uint aCount) override;

		void SetBlendState(SR_BlendState* aBlendState, const SC_Vector4f& aBlendFactor) override;
		void SetDepthState(SR_DepthState* aDepthState, uint aStencilRef) override;
		void SetRasterizerState(SR_RasterizerState* aRasterizerState) override;

		void SetViewport(const SR_Viewport& aViewport) override;
		void SetScissorRect(const SR_ScissorRect& aScissorRect) override;
		void SetTopology(const SR_Topology& aTopology) override;
		void SetVertexBuffer(uint aStartVertex, SR_Buffer* aBuffer) override;
		void SetVertexBuffer(uint aStartVertex, SR_Buffer** aBuffer, uint aCount) override;
		void SetVertexBuffer(uint aStartVertex, SR_Buffer* aVertexBuffer, uint aVertexBufferOffset, SR_Buffer* aInstanceBuffer, uint aInstanceBufferOffset) override;
		void SetIndexBuffer(SR_Buffer* aIndexBuffer) override;
		void SetShaderState(SR_ShaderState* aShaderState) override;
		void SetRenderTargets(SR_RenderTarget** aTargets, uint aNumTargets, SR_RenderTarget* aDepthStencil, uint aDepthAccess) override;

		void CopyBuffer(SR_Buffer* aDestination, SR_Buffer* aSource) override;
		void CopyTexture(SR_TextureBuffer* aDestination, SR_TextureBuffer* aSource) override;
		void UpdateBufferData(uint aByteSize, void* aData, SR_Buffer* aOutBuffer) override;
		void GenerateMips(SR_Texture& aTexture) override;

		void ResolveQuery(const EQueryType& aType, uint aStartIndex, uint aNumQueries, SR_Buffer* aOutBuffer) override;
		void EndQuery(const EQueryType& aType, uint aIndex) override;

		SR_CommandList* GetCommandList() override;

		// DX12 specific
		ID3D12GraphicsCommandList* GetNativeCommandList() const;
	private:

		void InsertFence(const SR_Fence& aFence) override;
		void PreDraw();
		void PreDispatch();
		void SetDescriptorHeaps();
		void SetDescriptorTable(bool aIsCompute, uint aTable, const SR_Descriptor_DX12& aDescriptor);
		void SetResources();

#if SR_ENABLE_RAYTRACING
		SC_Ref<SR_Buffer> CreateAccelerationStructure(SR_Buffer_DX12* aBuffer, const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS& aInputs);
#endif

		template<class ResourceMask>
		void UpdateResourcesInTable(ResourceMask& aResourceMask, ResourceMask& aBoundResourcesInTableMask, ResourceMask& aBoundResourcesSinceLastTableUpdateMask);

		template<typename ResourceMask>
		void SetResourcesInTable(ResourceMask& aResourceMask, ResourceMask& aBoundResourcesInTableMask, ResourceMask& aBoundResourcesSinceLastTableUpdateMask,
			uint aResourceTypeStartIndex, uint aRootTable, SR_ShaderState* aShaderState, SR_DescriptorCPU_DX12* aBoundDescriptors, uint aNumBoundDescriptors, const SR_DescriptorCPU_DX12* aNullDescriptor);

		SR_BoundResources_DX12 myBoundResources;
		enum { Graphics, Compute };
		SR_RootCache_DX12 myRootCache[2];

		uint myResourceDescriptorIncrementSize;
		uint myRenderTargetDescriptorIncrementSize;
		uint myDepthBufferDescriptorIncrementSize;

		// Resource Heaps
		SR_TempMemoryPool_DX12* myTempHeap;
		ID3D12DescriptorHeap* myCurrentResourceHeap;

		// Root Signature
		SR_RootSignature_DX12* myCurrentRootSignature;

		// Command Lists
		SC_Ref<SR_CommandList> myCommandListKeepAlive;
		SR_CommandList_DX12* myCommandList;
		ID3D12GraphicsCommandList* myNativeCommandList;
#if SR_ENABLE_RAYTRACING
		ID3D12GraphicsCommandList4* myNativeCommandList4;
		SC_Ref<SR_Buffer_DX12> myScratchBufferTEMP;
#endif

		SR_GraphicsDevice_DX12* myDevice;

		SC_HashMap<SR_TrackedResource*, SC_Pair<uint, uint>> myIntermediateTransitions;
		bool myIsFirstResourceUpdateSinceBeginRecord : 1;
	};
}
#endif