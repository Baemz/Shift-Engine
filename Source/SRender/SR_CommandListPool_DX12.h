#pragma once
#if ENABLE_DX12
#include "SC_RefCounted.h"
#include "SR_CommandList_DX12.h"

namespace Shift
{
	class SR_CommandListPool_DX12 : public SC_RefCounted
	{
		friend class SR_CommandList_DX12;
		friend class SR_CommandAllocator_DX12;
	public:
		explicit SR_CommandListPool_DX12(D3D12_COMMAND_LIST_TYPE aType, const char* aDebugName);
		~SR_CommandListPool_DX12();

		SC_Ref<SR_CommandList_DX12> GetCommandList(SR_ContextType aType);
		void OpenCommandList(SR_CommandList_DX12* aCmdList);

		void Update();
	private:
		SC_Ref<SR_CommandAllocator_DX12> GetAllocator();

		D3D12_COMMAND_LIST_TYPE myType;
		volatile uint myAllocatorCount;
		volatile uint myCmdListCount;

		SC_Mutex myAllocatorsMutex;
		SC_GrowingArray<SC_Ref<SR_CommandAllocator_DX12>> myActiveAllocators;
		SC_GrowingArray<SC_Ref<SR_CommandAllocator_DX12>> myIdleAllocators;
		SC_DynamicCircularArray<SC_Ref<SR_CommandAllocator_DX12>> myInFlightAllocators;

		SC_Mutex myCmdListsMutex;
		SC_GrowingArray<SC_Ref<SR_CommandList_DX12>> myIdleCmdLists;

		const char* myDebugName;
	};

}
#endif