#pragma once
#if ENABLE_DX12

#include "SR_ResourceBindings.h"
#include "SR_Descriptor_DX12.h"
#include "SR_RootSignature_DX12.h"

namespace Shift
{
	struct SR_BoundResources_DX12
	{
		SR_BoundResources_DX12()
		{
			Clear();
		}

		void Clear()
		{
			SC_Fill(myLocalConstants, SR_ConstantBufferRef_NumLocals, SR_DescriptorCPU_DX12());
			SC_Fill(myLocalTextures, SR_TextureRef_NumLocals, SR_DescriptorCPU_DX12());
			SC_Fill(myLocalBuffers, SR_BufferRef_NumLocals, SR_DescriptorCPU_DX12());
			SC_Fill(myLocalTextureRWs, SR_TextureRWRef_NumLocals, SR_DescriptorCPU_DX12());
			SC_Fill(myLocalBufferRWs, SR_BufferRWRef_NumLocals, SR_DescriptorCPU_DX12());
			SC_Fill(myGlobalConstants, SR_ConstantBufferRef_NumGlobals, SR_DescriptorCPU_DX12());
			SC_Fill(myGlobalTextures, SR_TextureRef_NumGlobals, SR_DescriptorCPU_DX12());
			SC_Fill(myGlobalBuffers, SR_BufferRef_NumGlobals, SR_DescriptorCPU_DX12());
			SC_Fill(myGlobalTextureRWs, SR_TextureRWRef_NumGlobals, SR_DescriptorCPU_DX12());
			SC_Fill(myGlobalBufferRWs, SR_BufferRWRef_NumGlobals, SR_DescriptorCPU_DX12());
		}

		SR_DescriptorCPU_DX12 myLocalConstants[SR_ConstantBufferRef_NumLocals];
		SR_DescriptorCPU_DX12 myLocalTextures[SR_TextureRef_NumLocals];
		SR_DescriptorCPU_DX12 myLocalBuffers[SR_BufferRef_NumLocals];
		SR_DescriptorCPU_DX12 myLocalTextureRWs[SR_TextureRWRef_NumLocals];
		SR_DescriptorCPU_DX12 myLocalBufferRWs[SR_BufferRWRef_NumLocals];
		SR_DescriptorCPU_DX12 myGlobalConstants[SR_ConstantBufferRef_NumGlobals];
		SR_DescriptorCPU_DX12 myGlobalTextures[SR_TextureRef_NumGlobals];
		SR_DescriptorCPU_DX12 myGlobalBuffers[SR_BufferRef_NumGlobals];
		SR_DescriptorCPU_DX12 myGlobalTextureRWs[SR_TextureRWRef_NumGlobals];
		SR_DescriptorCPU_DX12 myGlobalBufferRWs[SR_BufferRWRef_NumGlobals];
	};

	struct SR_RootCache_DX12
	{
		SR_Descriptor_DX12 myTables[SR_RootSignature_DX12::Tables_Count];
		SR_ResourceBindings myBoundResourcesInTable;
		SR_ResourceBindings myBoundResourcesSinceLastTableUpdate;

		bool myIsFirstUpdateSinceReset : 1;

		void Clear() { memset(this, 0, sizeof(*this)); }

	};
}
#endif