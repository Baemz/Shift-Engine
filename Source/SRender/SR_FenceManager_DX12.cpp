#include "SRender_Precompiled.h"
#include "SR_FenceManager_DX12.h"

#if ENABLE_DX12

#include "SR_GraphicsDevice_DX12.h"

namespace Shift
{
	SR_FenceManager_DX12::SR_FenceManager_DX12()
	{
		for (uint i = 0; i < SR_ContextType_COUNT; ++i)
		{
			myFenceHandles[i] = CreateEvent(nullptr, true, false, nullptr);

			PerContextFenceData& fenceData = myFenceData[i];
			fenceData.myLastInserted = 1;
			fenceData.myLastKnownCompleted = 1;
			SC_Fill(fenceData.myLastWaited, SR_ContextType_COUNT, uint64(1));

			SR_GraphicsDevice_DX12* device = static_cast<SR_GraphicsDevice_DX12*>(SR_GraphicsDevice::GetDevice());
			HRESULT hr = device->GetNativeDevice()->CreateFence(1, D3D12_FENCE_FLAG_NONE, SR_IID_PPV_ARGS(&fenceData.myFence));
			if (FAILED(hr))
			{
				SC_ERROR_LOG("Could not create fence for Context type [%i]", i);
				continue;
			}

			const char* name = SR_GetName(SR_ContextType(i));
			fenceData.myFence->SetName(ToWString(name).c_str());
		}

	}

	SR_FenceManager_DX12::~SR_FenceManager_DX12()
	{
		for (uint i = 0; i < SR_ContextType_COUNT; ++i)
		{
			if (myFenceData[i].myFence)
				myFenceData[i].myFence->Release();

			CloseHandle(myFenceHandles[i]);
		}
	}

	SR_Fence SR_FenceManager_DX12::InsertFence(SR_GraphicsContext_DX12* aCtx)
	{
		SR_ContextType contextType = aCtx->GetType();
		PerContextFenceData& fenceData = myFenceData[contextType];
		uint64 valueToSignal = 0; 

		for (;;)
		{
			valueToSignal = fenceData.myLastInserted + 1;
			if (SC_Atomic_CompareExchange(fenceData.myLastInserted, valueToSignal, valueToSignal - 1))
				break;
		}

		SR_Fence fence(valueToSignal, contextType);
		return fence;
	}

	bool SR_FenceManager_DX12::IsPending(const SR_Fence& aFence)
	{
		return myFenceData[aFence.myContextType].IsPending(aFence.myFenceValue);
	}

	bool SR_FenceManager_DX12::Wait(const SR_Fence& aFence)
	{
		PerContextFenceData& fenceData = myFenceData[aFence.myContextType];
		if (!fenceData.IsPending(aFence.myFenceValue))
			return true;

		for (;;)
		{
			if (!fenceData.IsPending(aFence.myFenceValue))
				break;

			uint64 nextWaitValue = fenceData.myLastKnownCompleted + 1;
			if (nextWaitValue > aFence.myFenceValue)
			{
				if (!fenceData.IsPending(aFence.myFenceValue))
					break;
				else
					assert(false);
			}

			HANDLE event = myFenceHandles[aFence.myContextType];
			ResetEvent(event);
			fenceData.myFence->SetEventOnCompletion(nextWaitValue, event);

			HRESULT hr = WaitForSingleObject(event, INFINITE);
			assert(!FAILED(hr));
		}

		return true;
	}

	void SR_FenceManager_DX12::InsertWait(SR_GraphicsContext_DX12* aCtx, const SR_Fence& aFence)
	{
		assert(aFence.myFenceValue);

		InsertWaitInternal(aCtx, aFence.myContextType, aFence.myFenceValue, true);
	}

	SR_Fence SR_FenceManager_DX12::GetLastInsertedFence(const SR_ContextType& aContext) const
	{
		SR_Fence fence(myFenceData[aContext].myLastInserted, aContext);
		assert(fence.myFenceValue);
		return fence;
	}

	SR_Fence SR_FenceManager_DX12::GetLastWaitedFence(const SR_ContextType& aSrcContext, const SR_ContextType& aWaitContext) const
	{
		return SR_Fence(myFenceData[aSrcContext].myLastWaited[aWaitContext], aSrcContext);
	}

	void SR_FenceManager_DX12::InsertWaitInternal(SR_GraphicsContext_DX12* aCtx, const SR_ContextType& aContext, uint64 aFenceValue, bool /*aExecContext*/)
	{
		SR_ContextType localCtx = aCtx->GetType();

		if (localCtx == aContext)
			return;

		PerContextFenceData& fenceData = myFenceData[aContext];
		if (!fenceData.IsPending(aFenceValue))
			return;

		uint64& lastWaited = fenceData.myLastWaited[localCtx];
		if (aFenceValue < lastWaited)
			return;

		uint64 lastInserted = fenceData.myLastInserted;
		assert(aFenceValue <= lastInserted);

		lastWaited = aFenceValue;

		//if (aExecContext)
		//	aCtx->Execute();

		SR_QueueManager::InsertWait(SR_Fence(aFenceValue, aContext), localCtx);
	}

	bool SR_FenceManager_DX12::PerContextFenceData::IsPending(uint64 aValue)
	{
		uint64 lastKnownCompleted = myLastKnownCompleted;
		if (aValue <= lastKnownCompleted)
			return false;

		uint64 completed = myFence->GetCompletedValue();
		if (lastKnownCompleted < completed)
		{
			if (completed == SC_UINT64_MAX) // Device removed
			{

#if !IS_FINAL_BUILD
				SR_GraphicsDevice_DX12* device = static_cast<SR_GraphicsDevice_DX12*>(SR_GraphicsDevice::GetDevice());
				HRESULT removedReason = device->GetNativeDevice()->GetDeviceRemovedReason();
				SC_ASSERT(false, "%i", removedReason);
#endif
				return true;
			}

			SC_Atomic_CompareExchange(myLastKnownCompleted, completed, lastKnownCompleted);
		}

		return completed < aValue;
	}
}
#endif