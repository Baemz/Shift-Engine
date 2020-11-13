#include "SRender_Precompiled.h"
#include "SR_GraphicsContext.h"
namespace Shift
{
	thread_local SC_Ref<SR_GraphicsContext> SR_GraphicsContext::ourCurrentContext = nullptr;

	SR_GraphicsContext* SR_GraphicsContext::GetCurrent()
	{
		return ourCurrentContext;
	}

	void SR_GraphicsContext::IncrementTrianglesDrawn(uint aIncrement)
	{
		myNumTrianglesDrawn += aIncrement;
	}

	void SR_GraphicsContext::IncrementDrawCalls()
	{
		++myNumDrawCalls;
	}

	void SR_GraphicsContext::IncrementDispatchCalls()
	{
		++myNumDispatchCalls;
	}

	void SR_GraphicsContext::IncrementInstancesDrawn(uint aIncrement)
	{
		myNumInstancesDrawn += aIncrement;
	}

	SR_GraphicsContext::SR_GraphicsContext(const SR_ContextType& aType)
		: myContextType(aType)
		, myUseDebugDevice(false)
		, myGraphicsDevice(nullptr)
		, myUseDebugMarkers(false)
		, myNeedsFlush(false)
		, myNumCommandsSinceReset(0)
		, myCurrentDepthState(nullptr)
		, myCurrentRasterizerState(nullptr)
		, myCurrentBlendState(nullptr)
		, myCurrentRenderTargetFormats(nullptr)
		, myCurrentShaderState(nullptr)
		, myCurrentTopology(SR_Topology_Unknown)
		, myNumTrianglesDrawn(0)
		, myNumDrawCalls(0)
		, myNumDispatchCalls(0)
		, myNumInstancesDrawn(0)
	{
		if (SC_CommandLineManager::HasCommand("debugrender"))
			myUseDebugDevice = true;

#if !IS_FINAL
		myUseDebugMarkers = true;
		if (SC_CommandLineManager::HasCommand("nodebugmarkers"))
			myUseDebugMarkers = false;
#endif
	}

	bool SR_GraphicsContext::NeedsFlush() const
	{
		return myNeedsFlush;
	}

	void SR_GraphicsContext::SetNeedsFlush()
	{
		myNeedsFlush = true;
	}

	void SR_GraphicsContext::ClearRenderTargets(SR_RenderTarget** aTargets, uint aNumTargets, const SC_Vector4f& aClearColor)
	{
		for (uint i = 0; i < aNumTargets; ++i)
			ClearRenderTarget(aTargets[i], aClearColor);
	}

	void SR_GraphicsContext::SetCurrentContext(SR_GraphicsContext* aCtx)
	{
		ourCurrentContext = aCtx;
	}

	void SR_GraphicsContext::Dispatch(const SC_Vector3ui aNumThreads)
	{
		Dispatch(aNumThreads.x, aNumThreads.y, aNumThreads.z);
	}

	void SR_GraphicsContext::Dispatch(SR_ShaderState* aComputeShader, const unsigned int aNumThreadsX, const unsigned int aNumThreadsY, const unsigned int aNumThreadsZ)
	{
		assert(aComputeShader->IsCompute());
		SetShaderState(aComputeShader);

		SC_Vector3ui numThreads = aComputeShader->myMetaData.myNumThreads;
		Dispatch((aNumThreadsX + numThreads.x - 1) / numThreads.x, (aNumThreadsY + numThreads.y - 1) / numThreads.y, (aNumThreadsZ + numThreads.z - 1) / numThreads.z);
	}

	void SR_GraphicsContext::Dispatch(SR_ShaderState* aComputeShader, const SC_Vector3ui aNumThreads)
	{
		assert(aComputeShader->IsCompute());
		Dispatch(aComputeShader, aNumThreads.x, aNumThreads.y, aNumThreads.z);
	}
}