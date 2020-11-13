#include "SRender_Precompiled.h"
#include "SR_ShaderState_DX12.h"

#if ENABLE_DX12
#include "SR_Buffer_DX12.h"

#include <sstream>

namespace Shift
{
	SR_ShaderState_DX12::SR_ShaderState_DX12()
		: myComputePipelineState(nullptr)
#if SR_ENABLE_RAYTRACING
		, myStateObject(nullptr)
#endif
	{
	}

	SR_ShaderState_DX12::~SR_ShaderState_DX12()
	{
		for (ID3D12PipelineState* pso : myPipelineStates)
			pso->Release();

		if (myComputePipelineState)
			myComputePipelineState->Release();

#if SR_ENABLE_RAYTRACING
		if (myStateObject)
			myStateObject->Release();
#endif
	}

	uint SR_ShaderState_DX12::GetHash() const
	{
		return uint();
	}

	ID3D12PipelineState* SR_ShaderState_DX12::GetPSO(const SR_PSOKey& aKey)
	{
		if (myIsCompute)
			return myComputePipelineState;

		ID3D12PipelineState* ps = nullptr;

		if (myPipelineStates.Find(aKey, ps))
			return ps;

		return nullptr;
	}

#if SR_ENABLE_RAYTRACING
	void SR_ShaderState_DX12::CreateRaytracingShaderTable(const SR_ShaderStateDesc& aDesc)
	{
		ID3D12StateObjectProperties* soProps = nullptr;
		SC_VERIFY(SUCCEEDED(myStateObject->QueryInterface(SR_IID_PPV_ARGS(&soProps))));

		uint tableSize = SC_Align(uint(D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES), uint(D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT));
		uint recordSize = SC_Align(uint(D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES), uint(D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT));

		uint rayGenOffset = 0;
		uint missOffset = 1 * tableSize;
		uint hitGroupsOffset = 2 * tableSize;

		uint numHitGroups = aDesc.myHitGroups.Count();
		uint hitGroupsSize = SC_Align(numHitGroups * recordSize, D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT);

		SR_BufferDesc bufProps;
		bufProps.mySize = 2 * tableSize + hitGroupsSize;
		bufProps.myBindFlag = SR_BindFlag_ConstantBuffer; // not really, but this gives sufficient alignment

		SC_GrowingArray<char> data;
		data.Respace(bufProps.mySize);

		const void* main = soProps->GetShaderIdentifier(L"main");
		assert(main);
		if (main)
			memcpy(data.GetBuffer() + rayGenOffset, main, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);

		const void* miss = soProps->GetShaderIdentifier(L"miss");
		if (miss)
			memcpy(data.GetBuffer() + missOffset, miss, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);

		std::wstringstream name;
		for (uint i = 0; i != numHitGroups; ++i)
		{
			name << L"HitGroup" << i;
			const void* id = soProps->GetShaderIdentifier(name.str().c_str());
			assert(id);
			memcpy(data.GetBuffer() + hitGroupsOffset + i * recordSize, id, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
		}

		myRaytracingShaderTable = SR_GraphicsDevice::GetDevice()->CreateBuffer(bufProps, data.GetBuffer(), "RT Shader Table");
		SR_Buffer_DX12* shaderTableDX12 = static_cast<SR_Buffer_DX12*>(myRaytracingShaderTable.Get());
		D3D12_GPU_VIRTUAL_ADDRESS ptr = shaderTableDX12->myDX12Resource->GetGPUVirtualAddress();

		myDispatchRaysDesc.RayGenerationShaderRecord.StartAddress = ptr + rayGenOffset;
		myDispatchRaysDesc.RayGenerationShaderRecord.SizeInBytes = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
		myDispatchRaysDesc.MissShaderTable.StartAddress = ptr + missOffset;
		myDispatchRaysDesc.MissShaderTable.SizeInBytes = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
		myDispatchRaysDesc.MissShaderTable.StrideInBytes = 0;
		myDispatchRaysDesc.HitGroupTable.StartAddress = ptr + hitGroupsOffset;
		myDispatchRaysDesc.HitGroupTable.SizeInBytes = hitGroupsSize;
		myDispatchRaysDesc.HitGroupTable.StrideInBytes = numHitGroups > 1 ? recordSize : 0; // 0 stride when possible as a hint to the driver, recommended by Intel

		soProps->Release();
	}

	ID3D12StateObject* SR_ShaderState_DX12::GetStateObject() const
	{
		SC_ASSERT(myIsRaytracing, "ShaderState is not raytrace-enabled.");
		return myStateObject;
	}
#endif
}
#endif