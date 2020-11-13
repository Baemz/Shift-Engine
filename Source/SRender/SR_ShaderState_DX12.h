#pragma once
#include "SR_ShaderState.h"

#if ENABLE_DX12
struct ID3D12PipelineState;
struct ID3D12StateObject;
namespace Shift
{
	class SR_RootSignature_DX12;
	class SR_Buffer_DX12;
	class SR_ShaderState_DX12 final : public SR_ShaderState
	{
		friend class SR_GraphicsDevice_DX12;
		friend class SR_GraphicsContext_DX12;
	public:
		SR_ShaderState_DX12();
		~SR_ShaderState_DX12();

		uint GetHash() const override;
		ID3D12PipelineState* GetPSO(const SR_PSOKey& aKey);

#if SR_ENABLE_RAYTRACING
		void CreateRaytracingShaderTable(const SR_ShaderStateDesc& aDesc);
		ID3D12StateObject* GetStateObject() const;
#endif

	private:
		/*
			Holds several pipeline states that are keyed on their state properties.
			For example they all use the same shaders, but might have different raster/depth/blend modes.
		*/
		SC_HashMap<SR_PSOKey, ID3D12PipelineState*> myPipelineStates;
		ID3D12PipelineState* myComputePipelineState;
		SC_Ref<SR_RootSignature_DX12> myRootSignature;

#if SR_ENABLE_RAYTRACING
		ID3D12StateObject* myStateObject;
		SC_Ref<SR_Buffer> myRaytracingShaderTable;
		D3D12_DISPATCH_RAYS_DESC myDispatchRaysDesc;
#endif
	};
}
#endif