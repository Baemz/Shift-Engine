#include "SRender_Precompiled.h"
#include "SR_GraphicsDevice.h"

#include "SR_RenderInterface.h"
#include "SR_SwapChain.h"
#include "SR_ShaderCompiler.h"

namespace Shift
{
	SR_GraphicsDevice* SR_GraphicsDevice::ourGraphicsDevice = nullptr;
	uint64 SR_GraphicsDevice::ourLastCompletedFrame = 0;
	static constexpr uint locTargetRenderThreads = 8;

	void SR_GraphicsDevice::Destroy()
	{
		SC_SAFE_DELETE(ourGraphicsDevice);
	}

	SR_GraphicsDevice::SR_GraphicsDevice()
		: myEnableDebugging(false)
		, myDebugBreakOnError(false)
		, myDebugBreakOnWarning(false)
		, myNumDispatchCalls(0)
		, myIsUsing16BitDepth(false)
		, myNumTrianglesDrawn(0)
		, myNumDrawCalls(0)
		, myUsedVRAM(0)
		, myRenderThreadPool(SC_Max(SC_Min(locTargetRenderThreads, std::thread::hardware_concurrency() - 1), 1))
	{
		assert(ourGraphicsDevice == nullptr);
		ourGraphicsDevice = this;
		
		if (SC_CommandLineManager::HasCommand(L"debugrender"))
			myEnableDebugging = true;

		if (SC_CommandLineManager::HasCommand(L"debugrenderbreak"))
		{
			myEnableDebugging = true;
			myDebugBreakOnError = true;
			if (SC_CommandLineManager::HasArgument(L"debugrenderbreak", L"warning"))
				myDebugBreakOnWarning = true;
		}

		if (SC_CommandLineManager::HasCommand(L"bitdepth16"))
			myIsUsing16BitDepth = true;
	}


	SR_GraphicsDevice::~SR_GraphicsDevice()
	{
		SR_QueueManager::Destroy();
		assert(ourGraphicsDevice != nullptr);
		ourGraphicsDevice = nullptr;
	}
	const bool SR_GraphicsDevice::PostInit()
	{
		SR_QueueManager::Init();

		SR_RenderInterface::ourBlack4x4 = GetCreateTexture("DefaultTextures/Black4x4.dds");
		SR_RenderInterface::ourWhite4x4 = GetCreateTexture("DefaultTextures/White4x4.dds");
		SR_RenderInterface::ourGrey4x4 = GetCreateTexture("DefaultTextures/Grey4x4.dds");
		SR_RenderInterface::ourNormal4x4 = GetCreateTexture("DefaultTextures/Normal4x4.dds");

		return true;
	}

	void SR_GraphicsDevice::ReleaseResources()
	{
		{
			SC_MutexLock lock(myReleaseResourcesMutex);
			myReleasedResourceCacheDupl.Swap(myReleasedResourceCache);
		}
		for (int i = myReleasedResourceCacheDupl.Count() - 1; i >= 0; --i)
		{
			auto& resource = myReleasedResourceCacheDupl[i];

			if (resource.myFrameRemoved < ourLastCompletedFrame)
			{
				delete resource.myResource;
				myReleasedResourceCacheDupl.RemoveByIndexCyclic(i);
			}
		}

	}

	SC_Ref<SR_Waitable> SR_GraphicsDevice::PostRenderTask(std::function<void()> aTask, uint aWaitMode)
	{
		return myRenderThreadPool.PostRenderTask(aTask, aWaitMode);
	}

	SC_Ref<SR_Waitable> SR_GraphicsDevice::PostComputeTask(std::function<void()> aTask, uint aWaitMode)
	{
		return myRenderThreadPool.PostComputeTask(aTask, aWaitMode);
	}

	SC_Ref<SR_Waitable> SR_GraphicsDevice::PostCopyTask(std::function<void()> aTask, uint aWaitMode)
	{
		return myRenderThreadPool.PostCopyTask(aTask, aWaitMode);
	}

	bool SR_GraphicsDevice::IsRenderPoolIdle() const
	{
		return myRenderThreadPool.IsIdle();
	}

	bool SR_GraphicsDevice::AddToReleasedResourceCache(SR_Resource* aResource, uint64 aFrameRemoved)
	{
		SC_MutexLock lock(myReleaseResourcesMutex);
		return myReleasedResourceCache.AddUnique(RemovedResource(aResource, aFrameRemoved));
	}

	const SR_GraphicsAPI& SR_GraphicsDevice::APIType() const
	{
		return myAPIType;
	}

	void SR_GraphicsDevice::InactivateContext(SR_GraphicsContext* aCtx)
	{
		SC_Atomic_Add(myNumTrianglesDrawn, aCtx->myNumTrianglesDrawn);
		SC_Atomic_Add(myNumDrawCalls, aCtx->myNumDrawCalls);
		SC_Atomic_Add(myNumDispatchCalls, aCtx->myNumDispatchCalls);
		SC_Atomic_Add(myNumInstancesDrawn, aCtx->myNumInstancesDrawn);
		aCtx->myNumTrianglesDrawn = 0;
		aCtx->myNumDrawCalls = 0;
		aCtx->myNumDispatchCalls = 0;
		aCtx->myNumInstancesDrawn = 0;

		InactivateContextInternal(aCtx);
	}
}