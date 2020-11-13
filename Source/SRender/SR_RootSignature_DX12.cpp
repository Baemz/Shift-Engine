#include "SRender_Precompiled.h"
#include "SR_RootSignature_DX12.h"

#if ENABLE_DX12
#include "SR_ResourceRefs.h"
#include "SR_GraphicsDevice_DX12.h"

#pragma warning(push)
#pragma warning(disable : 6236)

namespace Shift
{
	uint locGetRegisterSpace(const SR_RootSignature_DX12::Tables& aTableIndex)
	{
		switch (aTableIndex)
		{
		case SR_RootSignature_DX12::LocalBuffers:
		case SR_RootSignature_DX12::GlobalBuffers:
		case SR_RootSignature_DX12::LocalRWBuffers:
		case SR_RootSignature_DX12::GlobalRWBuffers:
			return 1;
		default:
			return 0;
		}
	}

	uint locGetBaseShaderRegister(const SR_RootSignature_DX12::Tables& aTableIndex)
	{
		switch (aTableIndex)
		{
		case SR_RootSignature_DX12::LocalConstants:
			return SR_ConstantBufferRef_LocalStart;
		case SR_RootSignature_DX12::GlobalConstants:
			return SR_ConstantBufferRef_GlobalStart;
		case SR_RootSignature_DX12::LocalBuffers:
			return SR_BufferRef_LocalStart;
		case SR_RootSignature_DX12::GlobalBuffers:
			return SR_BufferRef_GlobalStart;
		case SR_RootSignature_DX12::LocalRWBuffers:
			return SR_BufferRWRef_LocalStart;
		case SR_RootSignature_DX12::GlobalRWBuffers:
			return SR_BufferRWRef_GlobalStart;
		case SR_RootSignature_DX12::LocalTextures:
			return SR_TextureRef_LocalStart;
		case SR_RootSignature_DX12::GlobalTextures:
			return SR_TextureRef_GlobalStart;
		case SR_RootSignature_DX12::GlobalRWTextures:
			return SR_TextureRWRef_GlobalStart;
		case SR_RootSignature_DX12::LocalRWTextures:
			return SR_TextureRef_LocalStart;
		default:
			return 0;
		}
	}

	void locGetStaticSamplerDesc(SC_GrowingArray<D3D12_STATIC_SAMPLER_DESC>& aSamplersOut)
	{
		aSamplersOut.PreAllocate(SAMPLERSLOT_COUNT);
		for (uint i = 0; i < SAMPLERSLOT_COUNT; ++i)
			aSamplersOut.Add();


		// Linear Clamp
		aSamplersOut[0].Filter = D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;
		aSamplersOut[0].MaxAnisotropy = 0;
		aSamplersOut[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		aSamplersOut[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		aSamplersOut[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		aSamplersOut[0].BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
		aSamplersOut[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
		aSamplersOut[0].MipLODBias = 0.f;
		aSamplersOut[0].MinLOD = SC_FLT_LOWEST;
		aSamplersOut[0].MaxLOD = SC_FLT_MAX;
		aSamplersOut[0].MipLODBias = 0.f;
		aSamplersOut[0].RegisterSpace = 0;
		aSamplersOut[0].ShaderRegister = SAMPLERSLOT_LINEAR_CLAMP;
		aSamplersOut[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

		// Linear WRAP
		aSamplersOut[1].Filter = D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;
		aSamplersOut[1].MaxAnisotropy = 0;
		aSamplersOut[1].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		aSamplersOut[1].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		aSamplersOut[1].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		aSamplersOut[1].BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
		aSamplersOut[1].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
		aSamplersOut[1].MipLODBias = 0.f;
		aSamplersOut[1].MinLOD = SC_FLT_LOWEST;
		aSamplersOut[1].MaxLOD = SC_FLT_MAX;
		aSamplersOut[1].MipLODBias = 0.f;
		aSamplersOut[1].RegisterSpace = 0;
		aSamplersOut[1].ShaderRegister = SAMPLERSLOT_LINEAR_WRAP;
		aSamplersOut[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

		// Linear MIRROR
		aSamplersOut[2].Filter = D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;
		aSamplersOut[2].MaxAnisotropy = 0;
		aSamplersOut[2].AddressU = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
		aSamplersOut[2].AddressV = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
		aSamplersOut[2].AddressW = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
		aSamplersOut[2].BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
		aSamplersOut[2].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
		aSamplersOut[2].MipLODBias = 0.f;
		aSamplersOut[2].MinLOD = SC_FLT_LOWEST;
		aSamplersOut[2].MaxLOD = SC_FLT_MAX;
		aSamplersOut[2].MipLODBias = 0.f;
		aSamplersOut[2].RegisterSpace = 0;
		aSamplersOut[2].ShaderRegister = SAMPLERSLOT_LINEAR_MIRROR;
		aSamplersOut[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

		// Point Clamp
		aSamplersOut[3].Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
		aSamplersOut[3].MaxAnisotropy = 0;
		aSamplersOut[3].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		aSamplersOut[3].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		aSamplersOut[3].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		aSamplersOut[3].BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
		aSamplersOut[3].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
		aSamplersOut[3].MipLODBias = 0.f;
		aSamplersOut[3].MinLOD = SC_FLT_LOWEST;
		aSamplersOut[3].MaxLOD = SC_FLT_MAX;
		aSamplersOut[3].MipLODBias = 0.f;
		aSamplersOut[3].RegisterSpace = 0;
		aSamplersOut[3].ShaderRegister = SAMPLERSLOT_POINT_CLAMP;
		aSamplersOut[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

		// Linear WRAP
		aSamplersOut[4].Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
		aSamplersOut[4].MaxAnisotropy = 0;
		aSamplersOut[4].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		aSamplersOut[4].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		aSamplersOut[4].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		aSamplersOut[4].BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
		aSamplersOut[4].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
		aSamplersOut[4].MipLODBias = 0.f;
		aSamplersOut[4].MinLOD = SC_FLT_LOWEST;
		aSamplersOut[4].MaxLOD = SC_FLT_MAX;
		aSamplersOut[4].MipLODBias = 0.f;
		aSamplersOut[4].RegisterSpace = 0;
		aSamplersOut[4].ShaderRegister = SAMPLERSLOT_POINT_WRAP;
		aSamplersOut[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

		// Linear MIRROR
		aSamplersOut[5].Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
		aSamplersOut[5].MaxAnisotropy = 0;
		aSamplersOut[5].AddressU = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
		aSamplersOut[5].AddressV = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
		aSamplersOut[5].AddressW = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
		aSamplersOut[5].BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
		aSamplersOut[5].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
		aSamplersOut[5].MipLODBias = 0.f;
		aSamplersOut[5].MinLOD = SC_FLT_LOWEST;
		aSamplersOut[5].MaxLOD = SC_FLT_MAX;
		aSamplersOut[5].MipLODBias = 0.f;
		aSamplersOut[5].RegisterSpace = 0;
		aSamplersOut[5].ShaderRegister = SAMPLERSLOT_POINT_MIRROR;
		aSamplersOut[5].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

		// Anisotropic CLAMP
		aSamplersOut[6].Filter = D3D12_FILTER_ANISOTROPIC;
		aSamplersOut[6].MaxAnisotropy = 16;
		aSamplersOut[6].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		aSamplersOut[6].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		aSamplersOut[6].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		aSamplersOut[6].BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
		aSamplersOut[6].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
		aSamplersOut[6].MipLODBias = 0.f;
		aSamplersOut[6].MinLOD = SC_FLT_LOWEST;
		aSamplersOut[6].MaxLOD = SC_FLT_MAX;
		aSamplersOut[6].MipLODBias = 0.f;
		aSamplersOut[6].RegisterSpace = 0;
		aSamplersOut[6].ShaderRegister = SAMPLERSLOT_ANISO16_CLAMP;
		aSamplersOut[6].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

		// Anisotropic Wrap
		aSamplersOut[7].Filter = D3D12_FILTER_ANISOTROPIC;
		aSamplersOut[7].MaxAnisotropy = 16;
		aSamplersOut[7].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		aSamplersOut[7].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		aSamplersOut[7].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		aSamplersOut[7].BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
		aSamplersOut[7].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
		aSamplersOut[7].MipLODBias = 0.f;
		aSamplersOut[7].MinLOD = SC_FLT_LOWEST;
		aSamplersOut[7].MaxLOD = SC_FLT_MAX;
		aSamplersOut[7].MipLODBias = 0.f;
		aSamplersOut[7].RegisterSpace = 0;
		aSamplersOut[7].ShaderRegister = SAMPLERSLOT_ANISO16_WRAP;
		aSamplersOut[7].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

		// Anisotropic Mirror
		aSamplersOut[8].Filter = D3D12_FILTER_ANISOTROPIC;
		aSamplersOut[8].MaxAnisotropy = 16;
		aSamplersOut[8].AddressU = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
		aSamplersOut[8].AddressV = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
		aSamplersOut[8].AddressW = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
		aSamplersOut[8].BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
		aSamplersOut[8].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
		aSamplersOut[8].MipLODBias = 0.f;
		aSamplersOut[8].MinLOD = SC_FLT_LOWEST;
		aSamplersOut[8].MaxLOD = SC_FLT_MAX;
		aSamplersOut[8].MipLODBias = 0.f;
		aSamplersOut[8].RegisterSpace = 0;
		aSamplersOut[8].ShaderRegister = SAMPLERSLOT_ANISO16_MIRROR;
		aSamplersOut[8].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

		// Linear Cmp Clamp
		aSamplersOut[9].Filter = D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
		aSamplersOut[9].MaxAnisotropy = 16;
		aSamplersOut[9].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		aSamplersOut[9].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		aSamplersOut[9].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		aSamplersOut[9].BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
		aSamplersOut[9].ComparisonFunc = D3D12_COMPARISON_FUNC_GREATER_EQUAL;
		aSamplersOut[9].MipLODBias = 0.f;
		aSamplersOut[9].MinLOD = 0.f;
		aSamplersOut[9].MaxLOD = 3.402823466e+38f;
		aSamplersOut[9].MipLODBias = 0.f;
		aSamplersOut[9].RegisterSpace = 0;
		aSamplersOut[9].ShaderRegister = SAMPLERSLOT_CMP_LINEAR_CLAMP;
		aSamplersOut[9].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

		// Point Cmp Clamp
		aSamplersOut[10].Filter = D3D12_FILTER_COMPARISON_MIN_MAG_MIP_POINT;
		aSamplersOut[10].MaxAnisotropy = 0;
		aSamplersOut[10].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		aSamplersOut[10].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		aSamplersOut[10].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		aSamplersOut[10].BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
		aSamplersOut[10].ComparisonFunc = D3D12_COMPARISON_FUNC_GREATER_EQUAL;
		aSamplersOut[10].MipLODBias = 0.f;
		aSamplersOut[10].MinLOD = 0.f;
		aSamplersOut[10].MaxLOD = 3.402823466e+38f;
		aSamplersOut[10].MipLODBias = 0.f;
		aSamplersOut[10].RegisterSpace = 0;
		aSamplersOut[10].ShaderRegister = SAMPLERSLOT_CMP_POINT_CLAMP;
		aSamplersOut[10].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

		// Anisotropic Cmp CLAMP
		aSamplersOut[11].Filter = D3D12_FILTER_COMPARISON_ANISOTROPIC;
		aSamplersOut[11].MaxAnisotropy = 16;
		aSamplersOut[11].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		aSamplersOut[11].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		aSamplersOut[11].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		aSamplersOut[11].BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
		aSamplersOut[11].ComparisonFunc = D3D12_COMPARISON_FUNC_GREATER_EQUAL;
		aSamplersOut[11].MipLODBias = 0.f;
		aSamplersOut[11].MinLOD = 0.f;
		aSamplersOut[11].MaxLOD = 3.402823466e+38f;
		aSamplersOut[11].MipLODBias = 0.f;
		aSamplersOut[11].RegisterSpace = 0;
		aSamplersOut[11].ShaderRegister = SAMPLERSLOT_CMP_ANISO16_CLAMP;
		aSamplersOut[11].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	}

	D3D12_DESCRIPTOR_RANGE_TYPE locGetRangeType(const SR_RootTableType& aType)
	{
		switch (aType)
		{
		case SR_RootTableType_CBV:
			return D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
		case SR_RootTableType_SRV:
			return D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		case SR_RootTableType_UAV:
			return D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
		}

		assert(false);
		return D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	}

	SR_RootSignature_DX12::~SR_RootSignature_DX12()
	{
		if (myRootSignature)
			myRootSignature->Release();
	}

	void SR_RootSignature_DX12::Init(const SR_RootSignatureDesc_DX12& aDesc, const SR_RootSignatureType_DX12& aType)
	{
		myType = aType;

		SR_GraphicsDevice_DX12* srDevice = static_cast<SR_GraphicsDevice_DX12*>(SR_GraphicsDevice::GetDevice());
		ID3D12Device* device = srDevice->GetNativeDevice();

		D3D12_DESCRIPTOR_RANGE ranges[Tables::Tables_Count] = {};
		D3D12_ROOT_PARAMETER params[Tables::Tables_Count] = {};
		for (uint i = 0; i < Tables::Tables_Count; ++i)
		{
			const SR_RootSignatureDesc_DX12::Table& table = aDesc.myTableDescs[i];
			if (table.myNumDescriptors == 0)
				continue;

			ranges[i].RangeType = locGetRangeType(table.myType);
			ranges[i].NumDescriptors = table.myNumDescriptors;
			ranges[i].BaseShaderRegister = locGetBaseShaderRegister(Tables(i));
			ranges[i].RegisterSpace = locGetRegisterSpace(Tables(i));
			ranges[i].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

			params[i].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			params[i].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
			params[i].DescriptorTable.NumDescriptorRanges = 1;
			params[i].DescriptorTable.pDescriptorRanges = &ranges[i];

			myTableInfos[i].myNumDescriptors = table.myNumDescriptors;
			myTableInfos[i].myTableOffset = myMaxNumDescriptors;
			myMaxNumDescriptors += table.myNumDescriptors;
		}

		SC_GrowingArray<D3D12_STATIC_SAMPLER_DESC> samplers;
		locGetStaticSamplerDesc(samplers);

		D3D12_ROOT_SIGNATURE_FLAGS flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;
		if (myType != SR_RootSignatureType_DX12_Graphics)
			flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;
		else
			flags |= D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

		if (myType != SR_RootSignatureType_DX12_Graphics || !SR_ENABLE_TESSELATION_SHADERS)
			flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS;
		if (myType != SR_RootSignatureType_DX12_Graphics || !SR_ENABLE_GEOMETRY_SHADERS)
			flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

		D3D12_ROOT_SIGNATURE_DESC rootSigDesc = {};
		rootSigDesc.NumParameters = Tables::Tables_Count;
		rootSigDesc.pParameters = params;
		rootSigDesc.NumStaticSamplers = samplers.Count();
		rootSigDesc.pStaticSamplers = samplers.GetBuffer();
		rootSigDesc.Flags = flags;

		//D3D12_VERSIONED_ROOT_SIGNATURE_DESC desc;
		//desc.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
		//desc.Desc_1_1 = rootSigDesc;
		//
		//ID3DBlob* rootSigBlob = nullptr;
		//ID3DBlob* rootSigError = nullptr;
		//HRESULT hr = D3D12SerializeVersionedRootSignature(&desc, &rootSigBlob, &rootSigError);
		ID3DBlob* rootSigBlob = nullptr;
		ID3DBlob* rootSigError = nullptr;
		HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &rootSigBlob, &rootSigError);
		if (FAILED(hr))
		{
			if (rootSigError)
				SC_ERROR_LOG((char*)rootSigError->GetBufferPointer());
			assert(false);
		}
		hr = device->CreateRootSignature(0, rootSigBlob->GetBufferPointer(), rootSigBlob->GetBufferSize(), SR_IID_PPV_ARGS(&myRootSignature));
		assert(SUCCEEDED(hr));

		if (rootSigBlob != nullptr)
			rootSigBlob->Release();
		if (rootSigError != nullptr)
			rootSigError->Release();
	}

	uint SR_RootSignature_DX12::GetActualTableIndex(const Tables& aTableIndex)
	{
		return myTableInfos[aTableIndex].myTableOffset;
	}
}
#pragma warning(pop)

#endif