#pragma once
#include "SC_RefCounted.h"
#include "SC_Pair.h"
#include "SR_GraphicsDefinitions.h"
#include "SR_GraphicsEngineEnums.h"
#include "SR_GraphicsResources.h"
#include "SR_ResourceRefs.h"
#include "SR_RenderStates.h"
#include "SR_ResourceBindings.h"


#define SR_INTERNAL_SCOPED_PROFILER_FUNCTION(aVarName, aID) CGraphicsProfilerScope aVarName(aID)

#define SR_PROFILER_FUNCTION() SR_INTERNAL_SCOPED_PROFILER_FUNCTION(__internalScopedProfilerVariable, __FUNCTION__) /* TODO: Integrate timestamps into this define.*/

#define SR_PROFILER_FUNCTION_TAGGED(aID) SR_INTERNAL_SCOPED_PROFILER_FUNCTION(__internalScopedProfilerVariableTagged, aID)

namespace Shift
{
	class SR_GraphicsDevice;
	class SR_CommandList;

	// CGraphicsContext is the primary interface used to record GPU-commands.
	// The context is separated from the actual CommandList to allow for better threading-structure.
	// One thread only holds a single context, but the context may change between frames.
	// It internally manages descriptor placements and temporary-resource descriptor-rings.
	class SR_GraphicsContext : public SC_RefCounted
	{
		friend class SR_GraphicsDevice;
		friend class SR_QueueManager;
		friend class SR_RenderThreadPool;
	public:
		SR_GraphicsContext(const SR_ContextType& aType);
		virtual ~SR_GraphicsContext() {}

		virtual void Begin() = 0;	// Starts the scope of this context
		virtual void End() = 0;		// Ends the scope of this context

		virtual void BeginRecording() = 0;
		virtual void EndRecording() = 0;
		bool NeedsFlush() const;
		void SetNeedsFlush();

		virtual void InsertWait(SR_Waitable* aEvent) = 0;

		virtual void BeginEvent(const char* aID) = 0;
		virtual void EndEvent() = 0;

		static void SetCurrentContext(SR_GraphicsContext* aCtx);

		virtual void Draw(const unsigned int aVertexCount, const unsigned int aStartVertexLocation) = 0;
		virtual void DrawIndexed(const unsigned int aIndexCount, const unsigned int aStartIndexLocation, const unsigned int aBaseVertexLocation) = 0;
		virtual void DrawInstanced(const unsigned int aVertexCount, const unsigned int aInstanceCount, const unsigned int aStartVertexLocation, const unsigned int aStartInstanceLocation) = 0;
		virtual void DrawIndexedInstanced(const unsigned int aIndexCount, const unsigned int aInstanceCount, const unsigned int aStartIndexLocation, const unsigned int aBaseVertexLocation, const unsigned int aStartInstanceLocation) = 0;

		virtual void Dispatch(const unsigned int aThreadGroupCountX, const unsigned int aThreadGroupCountY = 1, const unsigned int aThreadGroupCountZ = 1) = 0;
		void Dispatch(const SC_Vector3ui aNumThreads = SC_Vector3ui(1));
		void Dispatch(SR_ShaderState* aComputeShader, const unsigned int aNumThreadsX = 1, const unsigned int aNumThreadsY = 1, const unsigned int aNumThreadsZ = 1);
		void Dispatch(SR_ShaderState* aComputeShader, const SC_Vector3ui aNumThreads = SC_Vector3ui(1));

		virtual void ClearRenderTarget(SR_RenderTarget* aTarget, const SC_Vector4f& aClearColor) = 0;
		void ClearRenderTargets(SR_RenderTarget** aTargets, uint aNumTargets, const SC_Vector4f& aClearColor);
		virtual void ClearDepthTarget(SR_RenderTarget* aTarget, float aDepthValue, uint8 aStencilValue, uint aClearFlags) = 0;

		//////////////////////////
		// Constant Buffers
		virtual void BindConstantBuffer(SR_Buffer* aBuffer, const uint aBindingIndex) = 0;
		virtual void BindConstantBuffer(void* aData, uint aSize, const uint aBindingIndex) = 0;
		virtual void BindConstantBufferRef(SR_Buffer* aBuffer, const SR_ConstantBufferRef::Enumerated& aRef) = 0;
		virtual void BindConstantBufferRef(void* aData, uint aSize, const SR_ConstantBufferRef::Enumerated& aRef) = 0;

		//////////////////////////
		// Buffers
		virtual void BindBuffer(SR_BufferView* aBufferView, const uint aBindingIndex) = 0;
		virtual void BindBufferRef(SR_BufferView* aBufferView, const SR_BufferRef::Enumerated& aRef) = 0;

		virtual void BindBufferRW(SR_BufferView* aBufferView, const uint aBindingIndex) = 0;
		virtual void BindBufferRWRef(SR_BufferView* aBufferView, const SR_BufferRWRef::Enumerated& aRef) = 0;

		//////////////////////////
		// Textures
		virtual void BindTexture(SR_Texture* aTexture, const uint aBindingIndex) = 0;
		virtual void BindTextureRef(SR_Texture* aTexture, const SR_TextureRef::Enumerated& aRef) = 0;

		virtual void BindTextureRW(SR_Texture* aTexture, const uint aBindingIndex) = 0;
		virtual void BindTextureRWRef(SR_Texture* aTexture, const SR_TextureRWRef::Enumerated& aRef) = 0;

#if SR_ENABLE_RAYTRACING
		virtual SC_Ref<SR_Buffer> CreateRTMesh(SR_Buffer* aBuffer, bool aFastUpdate, SR_Buffer* aVertexBuffer, uint aVertexStride, uint aNumVertices, SR_Buffer* aIndexBuffer, uint aNumIndices, const SC_Matrix44* aTransform) = 0;
		virtual SC_Ref<SR_BufferView> BuildRTScene(SR_BufferView* aBufferView, const SR_RaytracingInstanceData* aInstances, uint aNumInstances) = 0;
#endif

		virtual void Transition(const SR_ResourceState& aTransition, SR_TrackedResource* aResource) = 0;
		virtual void Transition(const SC_Pair<SR_ResourceState, SR_TrackedResource*>* aPairs, uint aCount) = 0;
		virtual void BarrierUAV(SR_TrackedResource* aResource) = 0;
		virtual void BarrierUAV(SR_TrackedResource** aResources, uint aCount) = 0;
		virtual void BarrierAlias(SR_TrackedResource* aBefore, SR_TrackedResource* aAfter) = 0;
		virtual void BarrierAlias(const SC_Pair<SR_TrackedResource*, SR_TrackedResource*>* aPairs, uint aCount) = 0;

		virtual void SetBlendState(SR_BlendState* aBlendState, const SC_Vector4f& aBlendFactor) = 0;
		virtual void SetDepthState(SR_DepthState* aDepthState, uint aStencilRef) = 0;
		virtual void SetRasterizerState(SR_RasterizerState* aRasterizerState) = 0;

		virtual void SetViewport(const SR_Viewport& aViewport) = 0;
		virtual void SetScissorRect(const SR_ScissorRect& aScissorRect) = 0;
		virtual void SetTopology(const SR_Topology& aTopology) = 0;
		virtual void SetVertexBuffer(uint aStartVertex, SR_Buffer* aBuffer) = 0;
		virtual void SetVertexBuffer(uint aStartVertex, SR_Buffer** aBuffer, uint aCount) = 0;
		virtual void SetVertexBuffer(uint aStartVertex, SR_Buffer* aVertexBuffer, uint aVertexBufferOffset, SR_Buffer* aInstanceBuffer, uint aInstanceBufferOffset) = 0;
		virtual void SetIndexBuffer(SR_Buffer* aIndexBuffer) = 0;
		virtual void SetShaderState(SR_ShaderState* aShaderState) = 0;
		virtual void SetRenderTargets(SR_RenderTarget** aTargets, uint aNumTargets, SR_RenderTarget* aDepthStencil, uint aDepthAccess) = 0;

		virtual void CopyBuffer(SR_Buffer* aDestination, SR_Buffer* aSource) = 0;
		virtual void CopyTexture(SR_TextureBuffer* aDestination, SR_TextureBuffer* aSource) = 0;
		virtual void UpdateBufferData(uint aByteSize, void* aData, SR_Buffer* aOutBuffer) = 0;

		virtual void GenerateMips(SR_Texture& aTexture) = 0;

		virtual void ResolveQuery(const EQueryType& aType, uint aStartIndex, uint aNumQueries, SR_Buffer* aOutBuffer) = 0;
		virtual void EndQuery(const EQueryType& aType, uint aIndex) = 0;

		virtual SR_CommandList* GetCommandList() = 0;

		SR_GraphicsDevice* GetDevice() { return myGraphicsDevice; };
		const SR_ContextType& GetType() { return myContextType; }
		const SR_Fence& GetCurrentFence() { return myLastFence; }

		static SR_GraphicsContext* GetCurrent();

		void SetId(const char* aId) { myId = aId; }
		const char* GetId() const { return myId.c_str(); }

	protected:
		virtual void InsertFence(const SR_Fence& aFence) = 0;
		void IncrementTrianglesDrawn(uint aIncrement);
		void IncrementDrawCalls();

		void IncrementDispatchCalls();

		void IncrementInstancesDrawn(uint aIncrement);

		static thread_local SC_Ref<SR_GraphicsContext> ourCurrentContext;

		std::string myId;

		SC_GrowingArray<SC_Ref<SR_Waitable>> myInsertedWaitables;

		// Resources
		SR_ResourceBindings myResources;

		// Render States
		SC_Vector4f myCurrentBlendFactor;
		uint myCurrentStencilRef;
		SR_DepthState* myCurrentDepthState;
		SR_RasterizerState* myCurrentRasterizerState;
		SR_BlendState* myCurrentBlendState;
		SR_RenderTargetFormatState* myCurrentRenderTargetFormats;
		SR_Topology myCurrentTopology;
		SR_PSOKey myCurrentPSOKey;
		SR_ShaderState* myCurrentShaderState;

		SR_ContextType myContextType;
		SR_GraphicsDevice* myGraphicsDevice;
		SR_Fence myLastFence;
		uint myNumCommandsSinceReset;

		uint myNumTrianglesDrawn;
		uint myNumDrawCalls;
		uint myNumDispatchCalls;
		uint myNumInstancesDrawn;

		bool myUseDebugDevice;
		bool myUseDebugMarkers;
		bool myNeedsFlush;
	};


	class CGraphicsProfilerScope
	{
	public:
		explicit CGraphicsProfilerScope(const char* aID)
			: myContext(nullptr)
		{
			SR_GraphicsContext* ctx = SR_GraphicsContext::GetCurrent();
			if (ctx)
			{
				myContext = ctx;
				myContext->BeginEvent(aID);
			}
		}

		~CGraphicsProfilerScope()
		{
			if (myContext)
				myContext->EndEvent();
		}

	private:
		SR_GraphicsContext* myContext;
	};
}
