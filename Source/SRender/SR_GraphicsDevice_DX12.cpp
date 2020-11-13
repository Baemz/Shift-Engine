#include "SRender_Precompiled.h"
#if ENABLE_DX12

#include "SR_GraphicsDevice_DX12.h"
#include "SR_GraphicsContext_DX12.h"
#include "SR_QueueManager_DX12.h"
#include "SR_SwapChain_DX12.h"
#include "SR_Converters_DX12.h"
#include "SR_Buffer_DX12.h"
#include "SR_BufferView_DX12.h"
#include "SR_TextureBuffer_DX12.h"
#include "SR_Texture_DX12.h"
#include "SR_RenderTarget_DX12.h"
#include "SR_ShaderState_DX12.h"
#include "SR_CommandList_DX12.h"
#include "SR_DDSTextureLoader_DX12.h"
#include "SR_WICTextureLoader_DX12.h"
#include "SR_RootSignature_DX12.h"
#include "SR_ShaderCompiler_DX12.h"
#include "DirectXTex.h"
#include "DirectXTex.inl"

#include "SC_Window_Win64.h"

#include "d3dx12.h"

#include <sstream>


namespace Shift
{
	static std::string globalShaderModel = "_5_1";
	SR_GraphicsDevice_DX12* SR_GraphicsDevice_DX12::ourGraphicsDevice_DX12 = nullptr;

	const char* GetFeatureLevelString(const D3D_FEATURE_LEVEL& aFeatureLevel)
	{
		switch (aFeatureLevel)
		{
		case D3D_FEATURE_LEVEL_12_1:
			return "12.1";
		case D3D_FEATURE_LEVEL_12_0:
			return "12.0";
		case D3D_FEATURE_LEVEL_11_1:
			return "11.1";
		case D3D_FEATURE_LEVEL_11_0:
			return "11.0";
		}
		return "<not supported>";
	}

	void locFillPipelineStateDesc(const SR_ShaderStateDesc::PSOStruct& aPSO, D3D12_GRAPHICS_PIPELINE_STATE_DESC& aOutPSODesc)
	{
		switch (aPSO.myTopology)
		{
		default:
		case SR_Topology_TriangleList:
			aOutPSODesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
			break;
		case SR_Topology_PointList:
			aOutPSODesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
			break;
		case SR_Topology_LineList:
			aOutPSODesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
			break;
		case SR_Topology_Patch:
			aOutPSODesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
			break;
		}
		aOutPSODesc.NumRenderTargets = aPSO.myRenderTargetFormats.myNumColorFormats;
		aOutPSODesc.DSVFormat = SR_ConvertFormat_DX12(aPSO.myRenderTargetFormats.myDepthFormat);
		for (uint i = 0; i < SR_MAX_RENDER_TARGETS; ++i)
		{
			aOutPSODesc.RTVFormats[i] = SR_ConvertFormat_DX12(aPSO.myRenderTargetFormats.myColorFormats[i]);
		}

		const SR_DepthStateDesc& depthStateDesc = aPSO.myDepthStateDesc;
		D3D12_DEPTH_STENCIL_DESC depthDesc;
		depthDesc.DepthEnable = (depthStateDesc.myDepthComparisonFunc != SR_ComparisonFunc_Always || depthStateDesc.myWriteDepth) ? true : false;
		depthDesc.DepthWriteMask = (depthStateDesc.myWriteDepth) ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
		depthDesc.DepthFunc = GetComparisonFunc_DX12(depthStateDesc.myDepthComparisonFunc);
		depthDesc.StencilEnable = depthStateDesc.myEnableStencil;
		depthDesc.StencilReadMask = depthStateDesc.myStencilReadMask;
		depthDesc.StencilWriteMask = depthStateDesc.myStencilWriteMask;
		depthDesc.FrontFace.StencilFailOp = GetStencilOperator_DX12(depthStateDesc.myFrontFace.myFail);
		depthDesc.FrontFace.StencilDepthFailOp = GetStencilOperator_DX12(depthStateDesc.myFrontFace.myDepthFail);
		depthDesc.FrontFace.StencilPassOp = GetStencilOperator_DX12(depthStateDesc.myFrontFace.myPass);
		depthDesc.FrontFace.StencilFunc = GetComparisonFunc_DX12(depthStateDesc.myFrontFace.myStencilComparisonFunc);
		depthDesc.BackFace.StencilFailOp = GetStencilOperator_DX12(depthStateDesc.myBackFace.myFail);
		depthDesc.BackFace.StencilDepthFailOp = GetStencilOperator_DX12(depthStateDesc.myBackFace.myDepthFail);
		depthDesc.BackFace.StencilPassOp = GetStencilOperator_DX12(depthStateDesc.myBackFace.myPass);
		depthDesc.BackFace.StencilFunc = GetComparisonFunc_DX12(depthStateDesc.myBackFace.myStencilComparisonFunc);
		aOutPSODesc.DepthStencilState = depthDesc;

		aOutPSODesc.SampleDesc = DXGI_SAMPLE_DESC{ 1, 0 };
		aOutPSODesc.SampleMask = 0xffffffff;

		const SR_RasterizerStateDesc& rasteriserStateDesc = aPSO.myRasterizerStateDesc;
		D3D12_RASTERIZER_DESC rasterDesc;
		rasterDesc.FillMode = (rasteriserStateDesc.myWireframe) ? D3D12_FILL_MODE_WIREFRAME : D3D12_FILL_MODE_SOLID;
		//rasterDesc.FillMode = D3D12_FILL_MODE_WIREFRAME;
		rasterDesc.CullMode = GetRasterizerFaceCull_DX12(rasteriserStateDesc.myFaceCull);
		rasterDesc.FrontCounterClockwise = false;
		rasterDesc.DepthBias = rasteriserStateDesc.myDepthBias;
		rasterDesc.DepthBiasClamp = rasteriserStateDesc.myDepthBiasClamp;
		rasterDesc.SlopeScaledDepthBias = rasteriserStateDesc.mySlopedScaleDepthBias;
		rasterDesc.DepthClipEnable = rasteriserStateDesc.myEnableDepthClip;
		rasterDesc.MultisampleEnable = false;
		rasterDesc.AntialiasedLineEnable = false;
		rasterDesc.ForcedSampleCount = 0;
		rasterDesc.ConservativeRaster = (rasteriserStateDesc.myConservativeRaster && SR_GraphicsDevice::GetDevice()->GetFeatureSupport().myEnableConservativeRaster) ? D3D12_CONSERVATIVE_RASTERIZATION_MODE_ON : D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
		aOutPSODesc.RasterizerState = rasterDesc;

		const SR_BlendStateDesc& blendStateDesc = aPSO.myBlendStateDesc;
		D3D12_BLEND_DESC blendDesc = {};
		blendDesc.AlphaToCoverageEnable = blendStateDesc.myAlphaToCoverage;
		blendDesc.IndependentBlendEnable = (blendStateDesc.myNumRenderTargets > 0);
		for (uint rtIndex = 0; rtIndex < SR_MAX_RENDER_TARGETS; ++rtIndex) // TODO: CHANGE TO MAX_RENDER_TARGETS DEFINE
		{
			D3D12_RENDER_TARGET_BLEND_DESC& rtBlendDescDst = blendDesc.RenderTarget[rtIndex];
			const SR_RenderTargetBlendDesc& rtBlendDesc = blendStateDesc.myRenderTagetBlendDescs[rtIndex];
			rtBlendDescDst.BlendEnable = rtBlendDesc.myEnableBlend;
			rtBlendDescDst.BlendOp = GetBlendFunc_DX12(rtBlendDesc.myBlendFunc);
			rtBlendDescDst.BlendOpAlpha = GetBlendFunc_DX12(rtBlendDesc.myBlendFuncAlpha);
			rtBlendDescDst.SrcBlend = GetBlendMode_DX12(rtBlendDesc.mySrcBlend);
			rtBlendDescDst.SrcBlendAlpha = GetBlendMode_DX12(rtBlendDesc.mySrcBlendAlpha);
			rtBlendDescDst.DestBlend = GetBlendMode_DX12(rtBlendDesc.myDstBlend);
			rtBlendDescDst.DestBlendAlpha = GetBlendMode_DX12(rtBlendDesc.myDstBlendAlpha);
			rtBlendDescDst.RenderTargetWriteMask = GetColorWriteMask_DX12(rtBlendDesc.myWriteMask);
			rtBlendDescDst.LogicOp = D3D12_LOGIC_OP_NOOP;
			rtBlendDescDst.LogicOpEnable = false;

			if (!(rtBlendDescDst.RenderTargetWriteMask & D3D12_COLOR_WRITE_ENABLE_ALPHA))
			{
				rtBlendDescDst.SrcBlendAlpha = D3D12_BLEND_ZERO;
				rtBlendDescDst.DestBlendAlpha = D3D12_BLEND_ONE;
			}
		}
		aOutPSODesc.BlendState = blendDesc;
	}

	SR_GraphicsDevice_DX12::SR_GraphicsDevice_DX12()
		: SR_GraphicsDevice()
		, myDevice(nullptr)
		, myInfoQueue(nullptr)
		, myResourceDescriptorIncrementSize(0)
		, mySamplerDescriptorIncrementSize(0)
		, myFeatureLevel(D3D_FEATURE_LEVEL_11_0)
		, myIsUsingPlacementHeaps(false)
		, myGPUDescriptorsOffset(0)
		, myCPUStagingDescriptorsOffset(0)
		, myForceCompileDXBC(false)
	{
		myAPIType = SR_GraphicsAPI::DirectX12;
		ourGraphicsDevice_DX12 = this;

		if (SC_CommandLineManager::HasCommand(L"renderdoc") || SC_CommandLineManager::HasCommand(L"usedxbc"))
			myForceCompileDXBC = true;
	}


	SR_GraphicsDevice_DX12::~SR_GraphicsDevice_DX12()
	{
		ourGraphicsDevice_DX12 = nullptr;
		if (myQueryHeap)
			myQueryHeap->Release();
		if (myInfoQueue)
			myInfoQueue->Release();
		if (myDevice)
			myDevice->Release();
		if (myDevice5)
			myDevice5->Release();
	}

	SR_GraphicsDevice_DX12* SR_GraphicsDevice_DX12::GetDX12Device()
	{
		return ourGraphicsDevice_DX12;
	}

	SR_DescriptorCPU_DX12 SR_GraphicsDevice_DX12::GetStagedHandle() const
	{
		return myResourceHeapStagedCPU.GetStartHandleCPU();
	}

	void SR_GraphicsDevice_DX12::CopyDescriptors(uint aNumDescriptors, D3D12_CPU_DESCRIPTOR_HANDLE aHandleDest, D3D12_CPU_DESCRIPTOR_HANDLE aHandleSrc)
	{
		myDevice->CopyDescriptorsSimple(aNumDescriptors, aHandleDest, aHandleSrc, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}

	ID3D12Device * SR_GraphicsDevice_DX12::GetNativeDevice()
	{
		return myDevice;
	}

	ID3D12Device5* SR_GraphicsDevice_DX12::GetNativeDevice5()
	{
		return myDevice5;
	}

	SR_RootSignature_DX12* SR_GraphicsDevice_DX12::GetGraphicsRootSignature()
	{
		return myRootSignatures[SR_RootSignatureType_DX12_Graphics];
	}

	SR_RootSignature_DX12* SR_GraphicsDevice_DX12::GetComputeRootSignature()
	{
		return myRootSignatures[SR_RootSignatureType_DX12_Compute];
	}

	ID3D12GraphicsCommandList* SR_GraphicsDevice_DX12::GetCommandList()
	{
		return myContext->myNativeCommandList;
	}

	ID3D12QueryHeap* SR_GraphicsDevice_DX12::GetQueryHeap()
	{
		return myQueryHeap;
	}

	void SR_GraphicsDevice_DX12::BeginFrame()
	{
		myReleaseResourcesFuture.Wait();
		myNumTrianglesDrawn = 0;
		myNumDrawCalls = 0;
		myNumDispatchCalls = 0;
		myNumInstancesDrawn = 0; 
		myNumDescriptorsAllocatedThisFrame = 0;

		myReleaseResourcesFuture = SC_CreateFrameTask(this, &SR_GraphicsDevice_DX12::ReleaseResources);
	}

	void SR_GraphicsDevice_DX12::EndFrame()
	{
	}

	void SR_GraphicsDevice_DX12::Present()
	{
		mySwapChain->Present(false);
	}

	SR_GraphicsContext* SR_GraphicsDevice_DX12::GetContext(const SR_ContextType& aType)
	{
		SC_Ref<SR_GraphicsContext_DX12> ctx;
		if (myInactiveContexts[aType].Count())
		{
			SC_MutexLock lock(myContextsMutex);
			if (myInactiveContexts[aType].Count())
			{
				ctx = myInactiveContexts[aType].GetLast();
				myInactiveContexts[aType].RemoveLast();
			}
		}

		if (!ctx)
			ctx = new SR_GraphicsContext_DX12(aType, this);

		SR_GraphicsContext::SetCurrentContext(ctx);
		return ctx;
	}

#if SR_ENABLE_RAYTRACING
	void SR_GraphicsDevice_DX12::SetRaytracingInstanceData(SR_RaytracingInstanceData& aOutInstance, SR_Buffer* aMeshBuffer, const SC_Matrix44& aMatrix, uint aInstanceData, uint8 aMask, uint8 aHitGroup, bool aIsOpaque, SR_RasterizerFaceCull aCullMode)
	{
		assert(aMeshBuffer);

		D3D12_RAYTRACING_INSTANCE_DESC desc = {};
		for (uint row = 0; row < 3; ++row)
			for (uint col = 0; col < 4; ++col)
				desc.Transform[row][col] = aMatrix.GetRowCol(col, row);
		desc.InstanceID = aInstanceData;
		desc.InstanceMask = aMask;
		desc.InstanceContributionToHitGroupIndex = aHitGroup;
		desc.AccelerationStructure = static_cast<SR_Buffer_DX12*>(aMeshBuffer)->myDX12Resource->GetGPUVirtualAddress();

		if (aIsOpaque)
			desc.Flags |= D3D12_RAYTRACING_INSTANCE_FLAG_FORCE_OPAQUE;

		switch (aCullMode)
		{
		case SR_RasterizerFaceCull_Front:
			desc.Flags |= D3D12_RAYTRACING_INSTANCE_FLAG_TRIANGLE_FRONT_COUNTERCLOCKWISE;
			break;
		case SR_RasterizerFaceCull_Back:
			break;
		case SR_RasterizerFaceCull_None:
		case SR_RasterizerFaceCull_COUNT:
			desc.Flags |= D3D12_RAYTRACING_INSTANCE_FLAG_TRIANGLE_CULL_DISABLE;
			break;
		}

		assert(desc.AccelerationStructure && (desc.AccelerationStructure& (D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BYTE_ALIGNMENT - 1)) == 0);
		static_assert(sizeof(aOutInstance) == sizeof(desc));
		SC_Memcpy(&aOutInstance, &desc, sizeof(desc));
	}
#endif

	SC_Ref<SR_TextureBuffer> SR_GraphicsDevice_DX12::CreateTextureBuffer(const SR_TextureBufferDesc& aDesc, const char* aID)
	{
		bool isRenderTarget = (aDesc.myFlags & SR_ResourceFlag_AllowRenderTarget);
		bool isDepth = (aDesc.myFlags & SR_ResourceFlag_AllowDepthStencil);

		D3D12_RESOURCE_DESC texDesc = {};
		texDesc.Width = static_cast<UINT64>(aDesc.myWidth);
		texDesc.Height = static_cast<UINT64>(aDesc.myHeight);
		texDesc.Format = SR_ConvertFormat_DX12(aDesc.myFormat);
		texDesc.MipLevels = aDesc.myMips;
		texDesc.DepthOrArraySize = UINT16(SC_Max(aDesc.myArraySize, 1U));
		texDesc.Dimension = static_cast<D3D12_RESOURCE_DIMENSION>(aDesc.myDimension);
		if (aDesc.myDimension == SR_Dimension_Texture3D)
			texDesc.DepthOrArraySize = UINT16(aDesc.myDepth);
		else if (aDesc.myDimension == SR_Dimension_TextureCube)
		{
			texDesc.DepthOrArraySize = aDesc.myArraySize * 6;
			texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		}
		texDesc.SampleDesc.Count = 1;
		texDesc.SampleDesc.Quality = 0;
		texDesc.Flags = static_cast<D3D12_RESOURCE_FLAGS>(aDesc.myFlags);

		SC_Ref<SR_TextureBuffer_DX12> textureBuffer(new SR_TextureBuffer_DX12());

		HRESULT result = S_OK;
		if (myIsUsingPlacementHeaps)
		{
			// Use Placed Resources instead of allocating new space for each resource.
		}
		else
		{
			D3D12_HEAP_PROPERTIES heapProps = {};
			heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
			heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
			heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
			heapProps.CreationNodeMask = 1;
			heapProps.VisibleNodeMask = 1;

			D3D12_CLEAR_VALUE* clearValue = nullptr;
			if (isRenderTarget || isDepth)
			{
				D3D12_CLEAR_VALUE _clearValue;
				_clearValue.Color[0] = 0.0f;
				_clearValue.Color[1] = 0.0f;
				_clearValue.Color[2] = 0.0f;
				_clearValue.Color[3] = 0.0f;
				_clearValue.Format = isDepth ? GetDepthFormatFromTypelessDX(aDesc.myFormat) : texDesc.Format;
				clearValue = &_clearValue;
			}

			result = myDevice->CreateCommittedResource(
				&heapProps,
				D3D12_HEAP_FLAG_NONE,
				&texDesc,
				(isDepth) ? D3D12_RESOURCE_STATE_DEPTH_WRITE : D3D12_RESOURCE_STATE_COMMON,
				clearValue,
				SR_IID_PPV_ARGS(&textureBuffer->myDX12Resource));


		}
		if (FAILED(result))
		{
			SC_ERROR_LOG("Could not create texture with ID [%s]", aID);
			return nullptr;
		}

		if (aID)
		{
			textureBuffer->myDX12Resource->SetName(ToWString(aID).c_str());
#if !IS_FINAL_BUILD
			textureBuffer->myName = aID;
#endif
		}

		textureBuffer->myProperties = aDesc;

		if (textureBuffer->myProperties.myDimension == SR_Dimension_TextureCube)
			textureBuffer->myProperties.myArraySize = 6;

		textureBuffer->myDX12Tracking.myState = (isDepth) ? SR_ResourceState_DepthWrite : SR_ResourceState_Common;

		uint bytesFromFormat = GetFormatByteSize_DX12(texDesc.Format);
		uint allocatedVRAM = 0;
		allocatedVRAM = (uint)texDesc.Width * texDesc.Height * (uint)texDesc.DepthOrArraySize * bytesFromFormat;
		myUsedVRAM += allocatedVRAM;

		return textureBuffer;
	}
	SC_Ref<SR_Texture> SR_GraphicsDevice_DX12::CreateTexture(const SR_TextureDesc& aTextureDesc, SR_TextureBuffer* aTextureBuffer)
	{
		SR_TextureBuffer_DX12* textureBuffer = static_cast<SR_TextureBuffer_DX12*>(aTextureBuffer);
		const SR_TextureBufferDesc& bufferProps = textureBuffer->GetProperties();

		bool isArray = bufferProps.myArraySize > 1;

		uint bufferArraySize = bufferProps.myDimension == SR_Dimension_Texture3D ? (uint)bufferProps.myDepth : SC_Max(bufferProps.myArraySize, 1U);
		uint arraySize = aTextureDesc.myArraySize ? aTextureDesc.myArraySize : bufferArraySize - aTextureDesc.myFirstArrayIndex;
		assert(arraySize <= bufferArraySize);

		SC_Ref<SR_Texture_DX12> texture(new SR_Texture_DX12(&myResourceHeapCPU));
		texture->myTextureBuffer = aTextureBuffer;
		texture->myDescriptorCPU = myResourceHeapCPU.AllocateCPU();
		texture->myDescription = aTextureDesc;

		uint mip = aTextureDesc.myMostDetailedMip;
		uint mipLevels = aTextureDesc.myMipLevels ? aTextureDesc.myMipLevels : bufferProps.myMips - mip;
		assert(mip + mipLevels <= bufferProps.myMips);

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = SR_ConvertFormat_DX12(aTextureDesc.myFormat);
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

		switch (bufferProps.myDimension)
		{
		case SR_Dimension_Texture1D:
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1D;
			srvDesc.Texture1D.MostDetailedMip = mip;
			srvDesc.Texture1D.MipLevels = mipLevels;
			break;

		case SR_Dimension_Texture2D:
		{	
			if (isArray)
			{
				srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
				srvDesc.Texture2DArray.MostDetailedMip = mip;
				srvDesc.Texture2DArray.MipLevels = mipLevels;
				srvDesc.Texture2DArray.FirstArraySlice = aTextureDesc.myFirstArrayIndex;
				srvDesc.Texture2DArray.ArraySize = arraySize;
			}
			else
			{
				srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
				srvDesc.Texture2D.MostDetailedMip = mip;
				srvDesc.Texture2D.MipLevels = mipLevels;
			}
			break;
		}
		case SR_Dimension_Texture3D:
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
			srvDesc.Texture3D.MostDetailedMip = mip;
			srvDesc.Texture3D.MipLevels = mipLevels;
			break;

		case SR_Dimension_TextureCube:
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
			srvDesc.TextureCube.MipLevels = mipLevels;
			srvDesc.TextureCube.MostDetailedMip = mip;
			break;
		}

		myDevice->CreateShaderResourceView(textureBuffer->myDX12Resource, &srvDesc, texture->myDescriptorCPU);

		return texture;
	}

	SC_Ref<SR_Texture> SR_GraphicsDevice_DX12::CreateRWTexture(const SR_TextureDesc& aTextureDesc, SR_TextureBuffer* aTextureBuffer)
	{
		SR_TextureBuffer_DX12* textureBuffer = static_cast<SR_TextureBuffer_DX12*>(aTextureBuffer);
		const SR_TextureBufferDesc& bufferProps = textureBuffer->GetProperties();

		const bool isArray = bufferProps.myArraySize > 1;
		assert(!aTextureDesc.myMipLevels || aTextureDesc.myMipLevels == 1);
		uint bufferArraySize = bufferProps.myDimension == SR_Dimension_Texture3D ? (uint)bufferProps.myDepth : SC_Max(bufferProps.myArraySize, 1U);
		uint arraySize = aTextureDesc.myArraySize ? aTextureDesc.myArraySize : bufferArraySize - aTextureDesc.myFirstArrayIndex;
		assert(arraySize <= bufferArraySize);

		SC_Ref<SR_Texture_DX12> texture(new SR_Texture_DX12(&myResourceHeapCPU));
		texture->myTextureBuffer = aTextureBuffer;
		texture->myDescriptorCPU = myResourceHeapCPU.AllocateCPU();
		texture->myDescription = aTextureDesc;

		D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
		uavDesc.Format = SR_ConvertFormat_DX12(bufferProps.myFormat);

		switch (bufferProps.myDimension)
		{
		case SR_Dimension_Texture1D:
			uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1D;
			uavDesc.Texture1D.MipSlice = aTextureDesc.myMostDetailedMip;
			break;

		case SR_Dimension_Texture2D:
		{
			if (isArray)
			{
				uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
				uavDesc.Texture2DArray.MipSlice = aTextureDesc.myMostDetailedMip;
				uavDesc.Texture2DArray.FirstArraySlice = aTextureDesc.myFirstArrayIndex;
				uavDesc.Texture2DArray.ArraySize = arraySize;
			}
			else
			{
				uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
				uavDesc.Texture2D.MipSlice = aTextureDesc.myMostDetailedMip;
			}
			break;
		}
		case SR_Dimension_Texture3D:
			uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
			uavDesc.Texture3D.MipSlice = aTextureDesc.myMostDetailedMip;
			uavDesc.Texture3D.FirstWSlice = aTextureDesc.myFirstArrayIndex;
			uavDesc.Texture3D.WSize = arraySize;
			break;

		case SR_Dimension_TextureCube:
			uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
			uavDesc.Texture2DArray.MipSlice = aTextureDesc.myMostDetailedMip;
			uavDesc.Texture2DArray.FirstArraySlice = aTextureDesc.myFirstArrayIndex * 6;
			uavDesc.Texture2DArray.ArraySize = SC_Max(1U, arraySize) * 6;
			break;
		}

		myDevice->CreateUnorderedAccessView(textureBuffer->myDX12Resource, nullptr, &uavDesc, texture->myDescriptorCPU);

		return texture;
	}

	SC_Ref<SR_Texture> SR_GraphicsDevice_DX12::GetCreateTexture(const char* aFile)
	{
		if (!aFile)
		{
			SC_ERROR_LOG("No texture-path provided.");
			return nullptr;
		}
		SC_Ref<SR_TextureBuffer_DX12> textureBuffer(new SR_TextureBuffer_DX12());

		std::unique_ptr<uint8_t[]> ddsData;
		std::vector<D3D12_SUBRESOURCE_DATA> subresources;

		bool isCube = false; 
		HRESULT result = S_OK;
		if (GetFileExtension(aFile) == "dds")
			result = DirectX::LoadDDSTextureFromFile(myDevice, ToWString(aFile).c_str(), &textureBuffer->myDX12Resource, ddsData, subresources, 0ui64, nullptr, &isCube);
		else
		{
			subresources.resize(1);
			result = DirectX::LoadWICTextureFromFile(myDevice, ToWString(aFile).c_str(), &textureBuffer->myDX12Resource, ddsData, subresources[0]);
		}
		if (FAILED(result))
		{
			SC_ERROR_LOG("Could not load texture [%s]", aFile);
			return nullptr;
		}
		
		const UINT64 uploadBufferSize = GetRequiredIntermediateSize(textureBuffer->myDX12Resource, 0, static_cast<UINT>(subresources.size()));

		// Create the GPU upload buffer.
		ID3D12Resource* uploadHeap;
		auto heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
		auto resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);
		result = myDevice->CreateCommittedResource(
			&heapProps,
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			SR_IID_PPV_ARGS(&uploadHeap));

		if (FAILED(result))
		{
			SC_ERROR_LOG("Could not load texture [%s]", aFile);
			return nullptr;
		}

		textureBuffer->myDX12Tracking.myState = SR_ResourceState_CopyDest;

		SR_GraphicsContext_DX12* ctx = static_cast<SR_GraphicsContext_DX12*>(SR_GraphicsDevice::GetDevice()->GetContext(SR_ContextType_Render));
		ctx->Begin();
		ctx->BeginRecording();

		ctx->Transition(SR_ResourceState_CopyDest, textureBuffer);
		UpdateSubresources(ctx->GetNativeCommandList(), textureBuffer->myDX12Resource, uploadHeap, 0, 0, static_cast<UINT>(subresources.size()), subresources.data());
		ctx->myNumCommandsSinceReset++;
		ctx->Transition(SR_ResourceState(SR_ResourceState_PixelSRV | SR_ResourceState_NonPixelSRV), textureBuffer);
		ctx->EndRecording();

		SC_GrowingArray<SR_CommandList*> cls;
		cls.Add(ctx->GetCommandList());
		SR_Fence fence = ctx->GetCurrentFence();
		SR_QueueManager::ExecuteCommandLists(cls, SR_ContextType_Render, fence.myFenceValue);
		ctx->End();
		//SR_GraphicsQueueManager_DX12::GetInstance()->GetFenceManager().Wait(fence);
		
		D3D12_RESOURCE_DESC desc = textureBuffer->myDX12Resource->GetDesc();
		SR_TextureBufferDesc& textureBufferDesc = textureBuffer->myProperties;

		textureBufferDesc.myWidth = static_cast<float>(desc.Width);
		textureBufferDesc.myHeight = static_cast<float>(desc.Height);
		textureBufferDesc.myMips = desc.MipLevels;
		textureBufferDesc.myFormat = SR_ConvertFormatFrom_DX12(desc.Format);
		textureBufferDesc.myFlags = desc.Flags;
		textureBufferDesc.myDimension = isCube ? SR_Dimension_TextureCube : static_cast<SR_Dimension>(desc.Dimension);
		textureBufferDesc.myIsCube = isCube;

		SR_TextureDesc textureDesc;
		textureDesc.myFormat = textureBufferDesc.myFormat;
		textureDesc.myMipLevels = textureBufferDesc.myMips;

		SC_Ref<SR_Texture> texture = CreateTexture(textureDesc, textureBuffer);
		return texture;
	}

	SC_Ref<SR_RenderTarget> SR_GraphicsDevice_DX12::CreateRenderTarget(const SR_RenderTargetDesc& aRenderTargetDesc, SR_TextureBuffer* aTextureBuffer)
	{
		SR_TextureBuffer_DX12* textureBuffer = static_cast<SR_TextureBuffer_DX12*>(aTextureBuffer);
		const SR_TextureBufferDesc& bufferProps = textureBuffer->GetProperties();

		SC_Ref<SR_RenderTarget_DX12> renderTarget(new SR_RenderTarget_DX12(&myRTVHeap));
		renderTarget->myTextureBuffer = aTextureBuffer;
		renderTarget->myRenderTargetView = myRTVHeap.AllocateCPU();

		D3D12_RENDER_TARGET_VIEW_DESC rtvdesc = {};
		rtvdesc.Format = SR_ConvertFormat_DX12(aRenderTargetDesc.myFormat == SR_Format_Unknown ? bufferProps.myFormat : aRenderTargetDesc.myFormat);

		switch (bufferProps.myDimension)
		{
		case SR_Dimension_Texture1D:

			rtvdesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE1D;
			break;
		case SR_Dimension_Texture2D:

			rtvdesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
			break;
		case SR_Dimension_Texture3D:

			rtvdesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE3D;
			break;
		}

		myDevice->CreateRenderTargetView(textureBuffer->myDX12Resource, &rtvdesc, renderTarget->myRenderTargetView);

		return renderTarget;
	}

	SC_Ref<SR_RenderTarget> SR_GraphicsDevice_DX12::CreateDepthStencil(const SR_RenderTargetDesc& aRenderTargetDesc, SR_TextureBuffer* aTextureBuffer)
	{
		SR_TextureBuffer_DX12* textureBuffer = static_cast<SR_TextureBuffer_DX12*>(aTextureBuffer);
		const SR_TextureBufferDesc& bufferProps = textureBuffer->GetProperties();

		bool isArray = bufferProps.myArraySize > 1;

		SC_Ref<SR_RenderTarget_DX12> renderTarget(new SR_RenderTarget_DX12(&myDSVHeap));
		renderTarget->myTextureBuffer = aTextureBuffer;
		renderTarget->myDepthStencilView = myDSVHeap.AllocateCPU();

		D3D12_DEPTH_STENCIL_VIEW_DESC dsvdesc = {};
		dsvdesc.Format = SR_ConvertFormat_DX12(aRenderTargetDesc.myFormat);

		switch (bufferProps.myDimension)
		{
		case SR_Dimension_Texture1D:
			dsvdesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE1D;
			break;
		case SR_Dimension_Texture2D:
		{
			if (isArray)
			{
				dsvdesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
				dsvdesc.Texture2DArray.FirstArraySlice = aRenderTargetDesc.myArrayIndex;
				dsvdesc.Texture2DArray.ArraySize = aRenderTargetDesc.myArraySize;
				dsvdesc.Texture2DArray.MipSlice = aRenderTargetDesc.myMipLevel;
			}
			else
				dsvdesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
			break;
		}
		default:
			assert(false && "Dimension not supported.");
			break;
		}

		myDevice->CreateDepthStencilView(textureBuffer->myDX12Resource, &dsvdesc, renderTarget->myDepthStencilView);

		return renderTarget;
	}

	SC_Ref<SR_Buffer> SR_GraphicsDevice_DX12::CreateBuffer(const SR_BufferDesc& aBufferDesc, void* aInitialData, const char* aIdentifier)
	{
		SC_UNUSED(aInitialData);
		SC_Ref<SR_Buffer_DX12> buffer(new SR_Buffer_DX12());

		D3D12_RESOURCE_DESC bufferDesc = {};
		bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		bufferDesc.Width = aBufferDesc.mySize;
		bufferDesc.Height = 1;
		bufferDesc.DepthOrArraySize = 1;
		bufferDesc.MipLevels = 1;
		bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
		bufferDesc.SampleDesc.Count = 1;
		bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		bufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

		if (aBufferDesc.myIsWritable)
			bufferDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

		D3D12_RESOURCE_STATES initialState = D3D12_RESOURCE_STATE_COMMON;
		D3D12_HEAP_PROPERTIES heapProps = {};
		GetResourceHeapDesc_DX12(heapProps, initialState, aBufferDesc.myCPUAccess, aBufferDesc.myMemoryAccess);

		if (aBufferDesc.myBindFlag & SR_BindFlag_RaytracingBuffer)
			initialState = D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;

		if (aBufferDesc.myBindFlag == SR_BindFlag_Buffer && aBufferDesc.myIsWritable)
			initialState = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;

		//uint alignment = SR_GetBufferAlignment_DX12(aBufferDesc);

		HRESULT result = S_OK;
		if (myIsUsingPlacementHeaps)
		{
			// Use Placed Resources instead of allocating new space for each resource.
		}
		else
		{
			result = myDevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &bufferDesc, initialState, nullptr, SR_IID_PPV_ARGS(&buffer->myDX12Resource));
			buffer->myDX12Tracking.myState = SR_ResourceState(initialState);
		}

		if (FAILED(result))
		{
			SC_ERROR_LOG("Could not create buffer with id: %s - %i", aIdentifier);
			return nullptr;
		}

#if !IS_XBOX_ONE_PLATFORM
		if (heapProps.Type == D3D12_HEAP_TYPE_UPLOAD)
		{
			D3D12_RANGE readRange = {};
			HRESULT hr = buffer->myDX12Resource->Map(0, &readRange, (void**)& buffer->myData);
			if (FAILED(hr))
				assert(false);
		}
#endif
		//
		// TODO: UPDATE INITIAL DATA HERE
		//
		//myContext->UpdateBufferData();
		if (aInitialData)
		{
			SR_BufferDesc uploadDesc(aBufferDesc);
			uploadDesc.myCPUAccess = SR_AccessCPU_Map_Write;

			SC_Ref<SR_Buffer> uploadBuffer = CreateBuffer(uploadDesc);

			SR_GraphicsContext_DX12* ctx = static_cast<SR_GraphicsContext_DX12*>(SR_GraphicsDevice::GetDevice()->GetContext(SR_ContextType_Render));
			ctx->Begin();
			ctx->BeginRecording();

			D3D12_SUBRESOURCE_DATA vertexData = {};
			vertexData.pData = reinterpret_cast<BYTE*>(aInitialData);
			vertexData.RowPitch = aBufferDesc.mySize;
			vertexData.SlicePitch = aBufferDesc.mySize;
			UpdateSubresources(ctx->GetNativeCommandList(), buffer->myDX12Resource, uploadBuffer->myDX12Resource, 0, 0, 1, &vertexData);
			ctx->myNumCommandsSinceReset++;

			ctx->Transition(SR_ResourceState_Common, buffer);

			ctx->EndRecording();

			SC_GrowingArray<SR_CommandList*> cls;
			cls.Add(ctx->GetCommandList());
			SR_Fence fence = ctx->GetCurrentFence();
			SR_QueueManager::ExecuteCommandLists(cls, SR_ContextType_Render, fence.myFenceValue);
			ctx->End();
		}

		if (aIdentifier)
			buffer->myDX12Resource->SetName(ToWString(aIdentifier).c_str());

		buffer->myDescription = aBufferDesc;
		return buffer;
	}

	SC_Ref<SR_BufferView> SR_GraphicsDevice_DX12::CreateBufferView(const SR_BufferViewDesc& aViewDesc, SR_Buffer* aBuffer)
	{
		SC_UNUSED(aViewDesc);
		SC_UNUSED(aBuffer);

		const SR_Buffer_DX12* buffer = static_cast<const SR_Buffer_DX12*>(aBuffer);
		const SR_BufferDesc& bufferProps = buffer->GetProperties();
		SC_UNUSED(bufferProps);

		SC_Ref<SR_BufferView_DX12> bufferView(new SR_BufferView_DX12(&myResourceHeapCPU));
		bufferView->myBuffer = aBuffer; 
		bufferView->myDescriptorCPU = myResourceHeapCPU.AllocateCPU();

		ID3D12Resource* resource = buffer ? buffer->myDX12Resource : nullptr;
		if (aViewDesc.myIsShaderWritable)
		{
			D3D12_UNORDERED_ACCESS_VIEW_DESC uavdesc = {};
			uavdesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;

			if (aViewDesc.myType == SR_BufferViewType_Bytes)
			{
				assert(!(aViewDesc.myFirstElement & 3));
				assert(!(aViewDesc.myNumElements & 3));

				uavdesc.Format = DXGI_FORMAT_R32_TYPELESS;
				uavdesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_RAW;
				uavdesc.Buffer.FirstElement = aViewDesc.myFirstElement / 4;
				uavdesc.Buffer.NumElements = aViewDesc.myNumElements / 4;
			}
			else
			{
				if (aViewDesc.myType == SR_BufferViewType_Structured)
				{
					uavdesc.Format = DXGI_FORMAT_UNKNOWN;
					uavdesc.Buffer.StructureByteStride = buffer->GetProperties().myStructSize;
				}
				else
					uavdesc.Format = SR_ConvertFormat_DX12(aViewDesc.myFormat);

				uavdesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
				uavdesc.Buffer.FirstElement = aViewDesc.myFirstElement;
				uavdesc.Buffer.NumElements = aViewDesc.myNumElements;
			}

			myDevice->CreateUnorderedAccessView(buffer->myDX12Resource, nullptr, &uavdesc, bufferView->myDescriptorCPU);
		}
		else
		{
			D3D12_SHADER_RESOURCE_VIEW_DESC srvdesc = {};
			srvdesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			srvdesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
			
			if (aViewDesc.myType == SR_BufferViewType_Bytes)
			{
				assert(!(aViewDesc.myFirstElement & 3));
				assert(!(aViewDesc.myNumElements & 3));

				srvdesc.Format = DXGI_FORMAT_R32_TYPELESS;
				srvdesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW;
				srvdesc.Buffer.FirstElement = aViewDesc.myFirstElement / 4;
				srvdesc.Buffer.NumElements = aViewDesc.myNumElements / 4;
			}
			else if (aViewDesc.myType == SR_BufferViewType_RaytracingScene)
			{
				assert(buffer);
				assert(buffer->GetProperties().myBindFlag & SR_BindFlag_RaytracingBuffer);

#if SR_ENABLE_RAYTRACING
				srvdesc.ViewDimension = D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE;
				srvdesc.RaytracingAccelerationStructure.Location = buffer->myDX12Resource->GetGPUVirtualAddress();
				resource = nullptr;
#else
				assert(false && "Cannot use acceleration structures when Raytracing is disabled");
#endif
			}
			else
			{
				srvdesc.Buffer.NumElements = aViewDesc.myNumElements;

				if (aViewDesc.myType == SR_BufferViewType_Structured)
				{
					srvdesc.Format = DXGI_FORMAT_UNKNOWN;
					srvdesc.Buffer.StructureByteStride = buffer->GetProperties().myStructSize;
				}
				else
					srvdesc.Format = SR_ConvertFormat_DX12(aViewDesc.myFormat);

				srvdesc.Buffer.FirstElement = aViewDesc.myFirstElement;
			}

			myDevice->CreateShaderResourceView(resource, &srvdesc, bufferView->myDescriptorCPU);
		}

		return bufferView;
	}

	SC_Ref<SR_ShaderState> SR_GraphicsDevice_DX12::CreateShaderState(const SR_ShaderStateDesc& aShaderStateDesc)
	{
		SC_Ref<SR_ShaderState_DX12> shaderState = new SR_ShaderState_DX12();
		shaderState->SetState(SC_LoadState::Loading);

		ID3D12PipelineState* pipelineState = nullptr;
		if (aShaderStateDesc.myShaderByteCodes[SR_ShaderType_Compute])
		{
			D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc = {};
			psoDesc.CS.BytecodeLength = (SIZE_T)aShaderStateDesc.myShaderByteCodes[SR_ShaderType_Compute]->GetByteCodeSize();
			psoDesc.CS.pShaderBytecode = aShaderStateDesc.myShaderByteCodes[SR_ShaderType_Compute]->GetByteCode();
			psoDesc.pRootSignature = myRootSignatures[SR_RootSignatureType_DX12_Compute]->myRootSignature;

			HRESULT hr = myDevice->CreateComputePipelineState(&psoDesc, SR_IID_PPV_ARGS(&pipelineState));
			if (FAILED(hr))
			{
				SC_ERROR_LOG("Could not create PSO");
				return nullptr;
			}

			if (aShaderStateDesc.myShaderByteCodes[SR_ShaderType_Compute]->myReflectionData)
			{
				ID3D12ShaderReflection* reflection = static_cast<ID3D12ShaderReflection*>(aShaderStateDesc.myShaderByteCodes[SR_ShaderType_Compute]->myReflectionData);
				SC_Vector3ui& numThreads = shaderState->myMetaData.myNumThreads;
				reflection->GetThreadGroupSize(&numThreads.x, &numThreads.y, &numThreads.z);
			}

			shaderState->myRootSignature = myRootSignatures[SR_RootSignatureType_DX12_Compute];
			shaderState->myComputePipelineState = pipelineState;
			shaderState->myIsCompute = true;
		}
#if SR_ENABLE_RAYTRACING
		else if (aShaderStateDesc.myShaderByteCodes[SR_ShaderType_Raytracing])
		{
			SC_HybridArray<D3D12_STATE_SUBOBJECT, 8> subObjects;

			D3D12_DXIL_LIBRARY_DESC libDesc = {};
			SR_ShaderByteCode* rtShaderByteCode = aShaderStateDesc.myShaderByteCodes[SR_ShaderType_Raytracing];
			libDesc.DXILLibrary = { rtShaderByteCode->GetByteCode(), (SIZE_T)rtShaderByteCode->GetByteCodeSize() };
			subObjects.Add({ D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY, &libDesc });

			D3D12_GLOBAL_ROOT_SIGNATURE globalRootSig = { myRootSignatures[SR_RootSignatureType_DX12_Raytracing]->myRootSignature };
			if (globalRootSig.pGlobalRootSignature)
				subObjects.Add({ D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE, &globalRootSig });

			D3D12_RAYTRACING_SHADER_CONFIG shaderConfig = {};
			shaderConfig.MaxAttributeSizeInBytes =  D3D12_RAYTRACING_MAX_ATTRIBUTE_SIZE_IN_BYTES;
			shaderConfig.MaxPayloadSizeInBytes = 32;
			subObjects.Add({ D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG, &shaderConfig });

			D3D12_RAYTRACING_PIPELINE_CONFIG pipelineConfig = {};
			pipelineConfig.MaxTraceRecursionDepth = 1;
			subObjects.Add({ D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG, &pipelineConfig });

			SC_HybridArray<D3D12_HIT_GROUP_DESC, 8> hitGroups;
			hitGroups.PreAllocate(aShaderStateDesc.myHitGroups.Count());

			SC_HybridArray<std::wstring, 8> strings;
			for (uint i = 0, count = aShaderStateDesc.myHitGroups.Count(); i < count; ++i)
			{
				std::wstringstream name;

				D3D12_HIT_GROUP_DESC hitGroup = {};
				hitGroup.Type = (aShaderStateDesc.myHitGroups[i].myHasIntersection) ? D3D12_HIT_GROUP_TYPE_PROCEDURAL_PRIMITIVE : D3D12_HIT_GROUP_TYPE_TRIANGLES;

				name << L"HitGroup" << i;
				hitGroup.HitGroupExport = strings.Add(name.str()).c_str(); 
				name.str(std::wstring());

				if (aShaderStateDesc.myHitGroups[i].myHasClosestHit)
				{
					name << L"HitGroup" << i << L"_ClosestHit";
					hitGroup.ClosestHitShaderImport = strings.Add(name.str()).c_str();
					name.str(std::wstring());
				}

				if (aShaderStateDesc.myHitGroups[i].myHasAnyHit)
				{
					name << L"HitGroup" << i << L"_AnyHit";
					hitGroup.AnyHitShaderImport = strings.Add(name.str()).c_str();
					name.str(std::wstring());
				}
				
				if (aShaderStateDesc.myHitGroups[i].myHasIntersection)
				{
					name << L"HitGroup" << i << L"_Intersection";
					hitGroup.IntersectionShaderImport = strings.Add(name.str()).c_str();
					name.str(std::wstring());
				}

				hitGroups.Add(hitGroup);
				subObjects.Add({ D3D12_STATE_SUBOBJECT_TYPE_HIT_GROUP, &hitGroups[i] });
			}

			D3D12_STATE_OBJECT_DESC desc = {};
			desc.Type = D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE;
			desc.pSubobjects = subObjects.GetBuffer();
			desc.NumSubobjects = subObjects.Count();

			ID3D12StateObject* stateObject = nullptr;
			HRESULT hr = myDevice5->CreateStateObject(&desc, SR_IID_PPV_ARGS(&stateObject));
			if (FAILED(hr))
			{
				SC_ERROR_LOG("Could not create State Object");
				return nullptr;
			}
			shaderState->myStateObject = stateObject;
			shaderState->CreateRaytracingShaderTable(aShaderStateDesc);
			shaderState->myRootSignature = myRootSignatures[SR_RootSignatureType_DX12_Raytracing];
			shaderState->myIsRaytracing = true;
		}
#endif
		else
		{
			D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
			SC_HybridArray<D3D12_INPUT_ELEMENT_DESC, 32> inputElements;
			SR_VertexLayout vertexLayout;

			// Add Input Layout
			if (aShaderStateDesc.myShaderByteCodes[SR_ShaderType_Vertex])
			{
				void* vsByteCode = aShaderStateDesc.myShaderByteCodes[SR_ShaderType_Vertex]->GetByteCode();
				uint vsByteCodeSize = aShaderStateDesc.myShaderByteCodes[SR_ShaderType_Vertex]->GetByteCodeSize();
				psoDesc.VS.BytecodeLength = (SIZE_T)vsByteCodeSize;
				psoDesc.VS.pShaderBytecode = vsByteCode;

				if (aShaderStateDesc.myShaderByteCodes[SR_ShaderType_Vertex]->myReflectionData) 
					ReflectShaderVertexLayout((ID3D12ShaderReflection*)aShaderStateDesc.myShaderByteCodes[SR_ShaderType_Vertex]->myReflectionData, &vertexLayout); // DXIL
				else
					ReflectShaderVertexLayout(vsByteCode, vsByteCodeSize, &vertexLayout); // DXBC

				SC_GrowingArray<SR_VertexLayoutElement>& ilData = vertexLayout.myElements;
				inputElements.PreAllocate(ilData.Count());
				for (SR_VertexLayoutElement& element : ilData)
				{
					D3D12_INPUT_ELEMENT_DESC& elementDesc = inputElements.Add();
					elementDesc.SemanticName = element.mySemanticName.c_str();
					elementDesc.SemanticIndex = element.mySemanticIndex;
					elementDesc.InputSlot = element.myInputSlot;
					elementDesc.AlignedByteOffset = element.myAlignedByteOffset;
					elementDesc.InputSlotClass = _ConvertInputClassificationDX12(element.myInputSlotClass);
					elementDesc.Format = SR_ConvertFormat_DX12(element.myFormat);
					elementDesc.InstanceDataStepRate = element.myInstanceDataStepRate;
				}
				psoDesc.InputLayout.NumElements = inputElements.Count();
				psoDesc.InputLayout.pInputElementDescs = inputElements.GetBuffer();
			}
#if SR_ENABLE_TESSELATION_SHADERS
			if (aShaderStateDesc.myShaderByteCodes[SR_ShaderType_Hull])
			{
				psoDesc.HS.BytecodeLength = (SIZE_T)aShaderStateDesc.myShaderByteCodes[SR_ShaderType_Hull]->GetByteCodeSize();
				psoDesc.HS.pShaderBytecode = aShaderStateDesc.myShaderByteCodes[SR_ShaderType_Hull]->GetByteCode();
			}
			if (aShaderStateDesc.myShaderByteCodes[SR_ShaderType_Domain])
			{
				psoDesc.DS.BytecodeLength = (SIZE_T)aShaderStateDesc.myShaderByteCodes[SR_ShaderType_Domain]->GetByteCodeSize();
				psoDesc.DS.pShaderBytecode = aShaderStateDesc.myShaderByteCodes[SR_ShaderType_Domain]->GetByteCode();
			}
#endif
#if SR_ENABLE_GEOMETRY_SHADERS
			if (aShaderStateDesc.myShaderByteCodes[SR_ShaderType_Geometry])
			{
				psoDesc.GS.BytecodeLength = (SIZE_T)aShaderStateDesc.myShaderByteCodes[SR_ShaderType_Geometry]->GetByteCodeSize();
				psoDesc.GS.pShaderBytecode = aShaderStateDesc.myShaderByteCodes[SR_ShaderType_Geometry]->GetByteCode();
			}
#endif
			if (aShaderStateDesc.myShaderByteCodes[SR_ShaderType_Pixel])
			{
				psoDesc.PS.BytecodeLength = (SIZE_T)aShaderStateDesc.myShaderByteCodes[SR_ShaderType_Pixel]->GetByteCodeSize();
				psoDesc.PS.pShaderBytecode = aShaderStateDesc.myShaderByteCodes[SR_ShaderType_Pixel]->GetByteCode();
			}

			psoDesc.pRootSignature = myRootSignatures[SR_RootSignatureType_DX12_Graphics]->myRootSignature;

			psoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

			locFillPipelineStateDesc(aShaderStateDesc.myMainPSO, psoDesc);
			HRESULT hr = myDevice->CreateGraphicsPipelineState(&psoDesc, SR_IID_PPV_ARGS(&pipelineState));
			if (FAILED(hr))
			{
				SC_ERROR_LOG("Could not create PSO");
				return nullptr;
			}
			shaderState->myRootSignature = myRootSignatures[SR_RootSignatureType_DX12_Graphics];

			SR_PSOKey key;
			key.myBlendStateKey = SC_Hash(aShaderStateDesc.myMainPSO.myBlendStateDesc);
			key.myDepthStateKey = SC_Hash(aShaderStateDesc.myMainPSO.myDepthStateDesc);
			key.myRasterStateKey = SC_Hash(aShaderStateDesc.myMainPSO.myRasterizerStateDesc);
			key.myRenderTargetsKey = SC_Hash(aShaderStateDesc.myMainPSO.myRenderTargetFormats);
			key.myTopologyKey = SC_Hash(aShaderStateDesc.myMainPSO.myTopology);

			shaderState->myPipelineStates[key] = pipelineState;

			for (uint i = 0, count = aShaderStateDesc.myAdditionalPSOs.Count(); i < count; ++i)
			{
				locFillPipelineStateDesc(aShaderStateDesc.myAdditionalPSOs[i], psoDesc);
				ID3D12PipelineState* additionalPSO = nullptr;
				hr = myDevice->CreateGraphicsPipelineState(&psoDesc, SR_IID_PPV_ARGS(&additionalPSO));
				if (FAILED(hr))
				{
					SC_ERROR_LOG("Could not create PSO");
					return nullptr;
				}

				SR_PSOKey key2;
				key2.myBlendStateKey = SC_Hash(aShaderStateDesc.myAdditionalPSOs[i].myBlendStateDesc);
				key2.myDepthStateKey = SC_Hash(aShaderStateDesc.myAdditionalPSOs[i].myDepthStateDesc);
				key2.myRasterStateKey = SC_Hash(aShaderStateDesc.myAdditionalPSOs[i].myRasterizerStateDesc);
				key2.myRenderTargetsKey = SC_Hash(aShaderStateDesc.myAdditionalPSOs[i].myRenderTargetFormats);
				key2.myTopologyKey = SC_Hash(aShaderStateDesc.myAdditionalPSOs[i].myTopology);

				shaderState->myPipelineStates[key2] = additionalPSO;
			}
		}

		shaderState->SetState(SC_LoadState::Loaded);
		return shaderState;
	}

	bool SR_GraphicsDevice_DX12::ReflectShaderVertexLayout(ID3D12ShaderReflection* aReflection, SR_VertexLayout* aOutLayout)
	{
		D3D12_SHADER_DESC shaderDesc;
		aReflection->GetDesc(&shaderDesc);

		for (unsigned int i = 0; i < shaderDesc.InputParameters; i++)
		{
			D3D12_SIGNATURE_PARAMETER_DESC paramDesc;
			aReflection->GetInputParameterDesc(i, &paramDesc);

			SR_VertexLayoutElement& elementDesc = aOutLayout->myElements.Add();
			elementDesc.mySemanticName = paramDesc.SemanticName;
			elementDesc.mySemanticIndex = paramDesc.SemanticIndex;
			if (i == 0)
				elementDesc.myAlignedByteOffset = 0;
			else
				elementDesc.myAlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

			std::string semantic(elementDesc.mySemanticName);
			std::string instancePrefix("INSTANCE_");
			if (semantic.substr(0, instancePrefix.size()) == instancePrefix)
			{
				elementDesc.myInputSlotClass = SR_VertexLayoutElement::EInputClassification::INPUT_PER_INSTANCE_DATA;
				elementDesc.myInstanceDataStepRate = 1;
				elementDesc.myInputSlot = 1;
			}
			else
			{
				elementDesc.myInputSlotClass = SR_VertexLayoutElement::EInputClassification::INPUT_PER_VERTEX_DATA;
				elementDesc.myInstanceDataStepRate = 0;
				elementDesc.myInputSlot = 0;
			}

			if (paramDesc.Mask == 1)
			{
				if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.myFormat = SR_Format_R32_Uint;
				else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.myFormat = SR_Format_R32_Sint;
				else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.myFormat = SR_Format_R32_Float;
			}
			else if (paramDesc.Mask <= 3)
			{
				if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.myFormat = SR_Format_RG32_Uint;
				else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.myFormat = SR_Format_RG32_Sint;
				else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.myFormat = SR_Format_RG32_Float;
			}
			else if (paramDesc.Mask <= 7)
			{
				if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.myFormat = SR_Format_RGB32_Uint;
				else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.myFormat = SR_Format_RGB32_Sint;
				else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.myFormat = SR_Format_RGB32_Float;
			}
			else if (paramDesc.Mask <= 15)
			{
				if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.myFormat = SR_Format_RGBA32_Uint;
				else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.myFormat = SR_Format_RGBA32_Sint;
				else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.myFormat = SR_Format_RGBA32_Float;
			}
		}
		
		//aReflection->Release();
		return true;
	}

	bool SR_GraphicsDevice_DX12::ReflectShaderVertexLayout(void* aVertexShaderByteCode, uint aByteCodeSize, SR_VertexLayout* aOutLayout)
	{
		ID3D12ShaderReflection* shaderReflection = nullptr;
		HRESULT hr = D3DReflect(aVertexShaderByteCode, aByteCodeSize, SR_IID_PPV_ARGS(&shaderReflection));
		if (FAILED(hr))
		{
			SC_ERROR_LOG("Could not reflect vertex layout on shader");
			return false;
		}

		return ReflectShaderVertexLayout(shaderReflection, aOutLayout);
	}

	void SR_GraphicsDevice_DX12::GenerateMips(SR_TextureBuffer* /*aTextureBuffer*/)
	{
	}

	const SC_Vector2f& SR_GraphicsDevice_DX12::GetResolution() const
	{
		return myWindow->GetResolution();
	}

	SR_GraphicsDeviceStats SR_GraphicsDevice_DX12::GetStats() const
	{
		SR_GraphicsDeviceStats stats;

		stats.myNumDescriptorsUsedCPU = myResourceHeapCPU.GetUsedCount();
		stats.myNumDescriptorsUsedGPU = myDescriptorRingGPU.myRingBuffer.GetUsedCount();
		stats.myNumDescriptorsUsedStaged = myDescriptorRingCPU.myRingBuffer.GetUsedCount();
		stats.myNumTrianglesDrawn = myNumTrianglesDrawn;
		stats.myNumDrawCalls = myNumDrawCalls;
		stats.myNumDispatchCalls = myNumDispatchCalls;
		stats.myNumInstancesDrawn = myNumInstancesDrawn;

		return stats;
	}

	SR_Descriptor_DX12 SR_GraphicsDevice_DX12::AllocateDescriptorRange(uint aNumDescriptors, bool aIsStagingRange, SR_GraphicsContext* aCtx)
	{
		SR_Descriptor_DX12 tableStart;
		if (aIsStagingRange)
		{
			SC_MutexLock lock(myCPUStagingDescriptorsLock);
			bool result = myDescriptorRingCPU.Allocate(tableStart, aNumDescriptors, aCtx);
			SC_VERIFY(result, "descriptor ring buffer is too small (or something is wrong)");
		}
		else
		{
			SC_MutexLock lock(myGPUDescriptorsLock);
			bool result = myDescriptorRingGPU.Allocate(tableStart, aNumDescriptors, aCtx);
			SC_VERIFY(result, "descriptor ring buffer is too small (or something is wrong)");
		}

		myNumDescriptorsAllocatedThisFrame += aNumDescriptors;

		return tableStart;
	}

	void SR_GraphicsDevice_DX12::GetLatestGlobalResources(SR_GraphicsContext_DX12* aCtx)
	{
		SC_MutexLock lock(myGlobalResourcesMutex);
		for (uint i = 0; i < SR_ConstantBufferRef_NumGlobals; ++i)
		{
			SR_GlobalConstantsMask mask = (SR_GlobalConstantsMask(1) << i);
			if ((aCtx->myResources.myGlobalConstants & mask) == 0)
			{
				aCtx->myBoundResources.myGlobalConstants[i] = myGlobalResourcesCache.myGlobalConstants[i];
				aCtx->myResources.myGlobalConstants |= mask;
			}
		}
		for (uint i = 0; i < SR_TextureRef_NumGlobals; ++i)
		{
			SR_GlobalTexturesMask mask = (SR_GlobalTexturesMask(1) << i);
			if ((aCtx->myResources.myGlobalTextures & mask) == 0)
			{
				aCtx->myBoundResources.myGlobalTextures[i] = myGlobalResourcesCache.myGlobalTextures[i];
				aCtx->myResources.myGlobalTextures |= mask;
			}
		}
		for (uint i = 0; i < SR_BufferRef_NumGlobals; ++i)
		{
			SR_GlobalBuffersMask mask = (SR_GlobalBuffersMask(1) << i);
			if ((aCtx->myResources.myGlobalBuffers & mask) == 0)
			{
				aCtx->myBoundResources.myGlobalBuffers[i] = myGlobalResourcesCache.myGlobalBuffers[i];
				aCtx->myResources.myGlobalBuffers |= mask;
			}
		}
		for (uint i = 0; i < SR_TextureRWRef_NumGlobals; ++i)
		{
			SR_GlobalTexturesRWMask mask = (SR_GlobalTexturesRWMask(1) << i);
			if ((aCtx->myResources.myGlobalTextureRWs & mask) == 0)
			{
				aCtx->myBoundResources.myGlobalTextureRWs[i] = myGlobalResourcesCache.myGlobalTextureRWs[i];
				aCtx->myResources.myGlobalTextureRWs |= mask;
			}
		}
		for (uint i = 0; i < SR_BufferRWRef_NumGlobals; ++i)
		{
			SR_GlobalBuffersRWMask mask = (SR_GlobalBuffersRWMask(1) << i);
			if ((aCtx->myResources.myGlobalBufferRWs & mask) == 0)
			{
				aCtx->myBoundResources.myGlobalBufferRWs[i] = myGlobalResourcesCache.myGlobalBufferRWs[i];
				aCtx->myResources.myGlobalBufferRWs |= mask;
			}
		}
	}

	SR_DescriptorHeap_DX12& SR_GraphicsDevice_DX12::GetGPUVisibleDescriptorHeap()
	{
		return myResourceHeapGPU;
	}

	const bool SR_GraphicsDevice_DX12::PostInit()
	{
		if (!SR_GraphicsDevice::PostInit())
			return false;

		mySwapChain = new SR_SwapChain_DX12();

		SR_SwapChainDesc swapChainDesc;
		swapChainDesc.myBackbufferFormat = SR_Format_RGBA8_Unorm;
		swapChainDesc.myWindow = myWindow;
		if (!mySwapChain->Init(swapChainDesc))
			return false;

		InitNullDescriptors();

		return true;
	}

	bool SR_GraphicsDevice_DX12::CreateResourceHeaps()
	{
		myResourceDescriptorIncrementSize = myDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		mySamplerDescriptorIncrementSize = myDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);

		static constexpr uint CPUResourceDescriptorHeapSize = 4 * 1024;
		static constexpr uint GPUResourceDescriptorHeapSize = 512 * 1024;
		static constexpr uint GPUResourceDescriptorRingSize = 450 * 1024;
		static constexpr uint CPUStagingResourceDescriptorHeapSize = 4 * 1024;
		static constexpr uint CPUStagingResourceDescriptorRingSize = 2 * 1024;
		static constexpr uint RTVDescriptorHeapSize = 128;
		static constexpr uint DSVDescriptorHeapSize = 128;

		if (!myResourceHeapCPU.Init(SR_DescriptorHeapType_CBV_SRV_UAV, CPUResourceDescriptorHeapSize, false, "Main CPU-Visible DescriptorHeap"))
			return false;

		if (!myResourceHeapStagedCPU.Init(SR_DescriptorHeapType_CBV_SRV_UAV, CPUStagingResourceDescriptorHeapSize, false, "Staged CPU-Visible DescriptorHeap"))
			return false;

		myDescriptorRingCPU.Init(myResourceHeapStagedCPU, CPUStagingResourceDescriptorRingSize);

		if (!myResourceHeapGPU.Init(SR_DescriptorHeapType_CBV_SRV_UAV, GPUResourceDescriptorHeapSize, true, "Main GPU-Visible DescriptorHeap"))
			return false;

		myDescriptorRingGPU.Init(myResourceHeapGPU, GPUResourceDescriptorRingSize);

		if (!myRTVHeap.Init(SR_DescriptorHeapType_RTV, RTVDescriptorHeapSize, false, "Main RTV DescriptorHeap"))
			return false;

		if (!myDSVHeap.Init(SR_DescriptorHeapType_DSV, DSVDescriptorHeapSize, false, "Main DSV DescriptorHeap"))
			return false;

		return true;
	}

	void SR_GraphicsDevice_DX12::SetFeatureSupport()
	{
		D3D12_FEATURE_DATA_D3D12_OPTIONS dx12Options = {};
		HRESULT hr = myDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, &dx12Options, sizeof(dx12Options));
		if (FAILED(hr))
		{
			SC_ERROR_LOG("Could not check options from device");
			return;
		}

		// Conservative Rasterization
		myFeatureSupport.myConservativeRasterTier = dx12Options.ConservativeRasterizationTier;
		myFeatureSupport.myEnableConservativeRaster = myFeatureSupport.myConservativeRasterTier > D3D12_CONSERVATIVE_RASTERIZATION_TIER_NOT_SUPPORTED;
		SC_LOG("Conservative Rasterization Tier: %i", myFeatureSupport.myConservativeRasterTier);

		myFeatureSupport.myTiledResourcesTier = dx12Options.TiledResourcesTier;
		myFeatureSupport.myEnableTiledResources = myFeatureSupport.myTiledResourcesTier > D3D12_TILED_RESOURCES_TIER_NOT_SUPPORTED;
		SC_LOG("Tiled Resources Tier: %i", myFeatureSupport.myTiledResourcesTier);

		myFeatureSupport.myResourceBindingTier = dx12Options.ResourceBindingTier;
		SC_LOG("Resource Binding Tier: %i", myFeatureSupport.myResourceBindingTier);

		myFeatureSupport.myResourceHeapTier = dx12Options.ResourceHeapTier;
		SC_LOG("Resource Heap Tier: %i", myFeatureSupport.myResourceHeapTier);

		myFeatureSupport.myEnableTypedLoadAdditionalFormats = dx12Options.TypedUAVLoadAdditionalFormats;
		SC_LOG("Additional Typed Load Formats: %s", myFeatureSupport.myEnableTypedLoadAdditionalFormats ? "true" : "false");

		D3D12_FEATURE_DATA_D3D12_OPTIONS5 dx12Options5 = {};
		hr = myDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &dx12Options5, sizeof(dx12Options5));
		if (FAILED(hr))
		{
			SC_ERROR_LOG("Could not check Options5 from device");
			return;
		}

		myFeatureSupport.myEnableRaytracing = dx12Options5.RaytracingTier > D3D12_RAYTRACING_TIER_NOT_SUPPORTED;
		myFeatureSupport.myRaytracingTier = dx12Options5.RaytracingTier;
		SC_LOG("Hardware Raytracing Tier: %.1f", (myFeatureSupport.myRaytracingTier / 10.f));

		myFeatureSupport.myRenderPassTier = dx12Options5.RenderPassesTier;
		myFeatureSupport.myEnableRenderPass = myFeatureSupport.myRenderPassTier > D3D12_RENDER_PASS_TIER_0; // We will not enable software-based render-passes.
		SC_LOG("Render Pass Tier: %i", myFeatureSupport.myRenderPassTier);

#if ENABLE_DX12_20H1
		D3D12_FEATURE_DATA_D3D12_OPTIONS6 dx12Options6 = {};
		hr = myDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS6, &dx12Options6, sizeof(dx12Options6));
		if (FAILED(hr))
		{
			SC_ERROR_LOG("Could not check Options6 from device");
			return;
		}
		myFeatureSupport.myVariableRateShadingTier = dx12Options6.VariableShadingRateTier;
		myFeatureSupport.myEnableVariableRateShading = dx12Options6.VariableShadingRateTier > D3D12_VARIABLE_SHADING_RATE_TIER_NOT_SUPPORTED;
		SC_LOG("Variable Rate Shading Tier: %i", myFeatureSupport.myEnableVariableRateShading);

		D3D12_FEATURE_DATA_D3D12_OPTIONS7 dx12Options7 = {};
		hr = myDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS7, &dx12Options7, sizeof(dx12Options7));
		if (FAILED(hr))
		{
			SC_ERROR_LOG("Could not check Options7 from device");
			return;
		}

		myFeatureSupport.myMeshShaderTier = dx12Options7.MeshShaderTier;
		myFeatureSupport.myEnableMeshShaders = dx12Options7.MeshShaderTier > D3D12_MESH_SHADER_TIER_NOT_SUPPORTED;
		float meshShadersTier = myFeatureSupport.myMeshShaderTier / 10.f;
		SC_LOG("Mesh Shaders Tier: %.1f", meshShadersTier);
#endif
	}

	void SR_GraphicsDevice_DX12::InactivateContextInternal(SR_GraphicsContext* aCtx)
	{
		SR_GraphicsContext_DX12* ctx = static_cast<SR_GraphicsContext_DX12*>(aCtx);
		if (!ctx)
			return;

		SR_ContextType type = ctx->GetType();

		{
			SC_MutexLock lock(myGlobalResourcesMutex);
			myGlobalResourcesCache.UpdateResources(ctx->myResources, ctx->myBoundResources);
		}

		{
			SC_MutexLock lock(myContextsMutex);
			myInactiveContexts[type].Add(ctx);
		}
	}

	void SR_GraphicsDevice_DX12::GetStaticSamplerDesc(D3D12_STATIC_SAMPLER_DESC(&aSamplersOut)[SAMPLERSLOT_COUNT]) const
	{
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

	void SR_GraphicsDevice_DX12::CreateDefaultRootSignatures()
	{
		SR_RootSignatureDesc_DX12 rootSignatureDesc;

		SR_RootSignatureDesc_DX12::Table& locConstantsTable = rootSignatureDesc.myTableDescs[SR_RootSignature_DX12::Tables::LocalConstants];
		locConstantsTable.myNumDescriptors = SR_ConstantBufferRef_NumLocals;
		locConstantsTable.myType = SR_RootTableType_CBV;

		SR_RootSignatureDesc_DX12::Table& locBuffersTable = rootSignatureDesc.myTableDescs[SR_RootSignature_DX12::Tables::LocalBuffers];
		locBuffersTable.myNumDescriptors = SR_BufferRef_NumLocals;
		locBuffersTable.myType = SR_RootTableType_SRV;

		SR_RootSignatureDesc_DX12::Table& locTexturesTable = rootSignatureDesc.myTableDescs[SR_RootSignature_DX12::Tables::LocalTextures];
		locTexturesTable.myNumDescriptors = SR_TextureRef_NumLocals;
		locTexturesTable.myType = SR_RootTableType_SRV;

		SR_RootSignatureDesc_DX12::Table& locTextureRWTable = rootSignatureDesc.myTableDescs[SR_RootSignature_DX12::Tables::LocalRWTextures];
		locTextureRWTable.myNumDescriptors = SR_TextureRWRef_NumLocals;
		locTextureRWTable.myType = SR_RootTableType_UAV;

		SR_RootSignatureDesc_DX12::Table& locBufferRWTable = rootSignatureDesc.myTableDescs[SR_RootSignature_DX12::Tables::LocalRWBuffers];
		locBufferRWTable.myNumDescriptors = SR_BufferRWRef_NumLocals;
		locBufferRWTable.myType = SR_RootTableType_UAV;

		SR_RootSignatureDesc_DX12::Table& globalConstantsTable = rootSignatureDesc.myTableDescs[SR_RootSignature_DX12::Tables::GlobalConstants];
		globalConstantsTable.myNumDescriptors = SR_ConstantBufferRef_NumGlobals;
		globalConstantsTable.myType = SR_RootTableType_CBV;

		SR_RootSignatureDesc_DX12::Table& globalBuffersTable = rootSignatureDesc.myTableDescs[SR_RootSignature_DX12::Tables::GlobalBuffers];
		globalBuffersTable.myNumDescriptors = SR_BufferRef_NumGlobals;
		globalBuffersTable.myType = SR_RootTableType_SRV;

		SR_RootSignatureDesc_DX12::Table& globalTexturesTable = rootSignatureDesc.myTableDescs[SR_RootSignature_DX12::Tables::GlobalTextures];
		globalTexturesTable.myNumDescriptors = SR_TextureRef_NumGlobals;
		globalTexturesTable.myType = SR_RootTableType_SRV;

		SR_RootSignatureDesc_DX12::Table& globalBufferRWsTable = rootSignatureDesc.myTableDescs[SR_RootSignature_DX12::Tables::GlobalRWBuffers];
		globalBufferRWsTable.myNumDescriptors = SR_BufferRWRef_NumGlobals;
		globalBufferRWsTable.myType = SR_RootTableType_UAV;

		SR_RootSignatureDesc_DX12::Table& globalTextureRWsTable = rootSignatureDesc.myTableDescs[SR_RootSignature_DX12::Tables::GlobalRWTextures];
		globalTextureRWsTable.myNumDescriptors = SR_TextureRWRef_NumGlobals;
		globalTextureRWsTable.myType = SR_RootTableType_UAV;

		rootSignatureDesc.myRootCBV.myIsUsed = false;
		rootSignatureDesc.myRootSRV.myIsUsed = false;

		for (uint i = 0; i < SR_RootSignatureType_DX12_COUNT; ++i)
		{
			myRootSignatures[i] = new SR_RootSignature_DX12();
			myRootSignatures[i]->Init(rootSignatureDesc, SR_RootSignatureType_DX12(i));
		}

	}

	void SR_GraphicsDevice_DX12::InitNullDescriptors()
	{
		{
			D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};

			myNullCBV = myResourceHeapCPU.AllocateCPU();
			myDevice->CreateConstantBufferView(&cbvDesc, myNullCBV);
		}

		{
			D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
			uavDesc.Format = DXGI_FORMAT_R32_FLOAT;
			uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;

			myNullBufferUAV = myResourceHeapCPU.AllocateCPU();
			myDevice->CreateUnorderedAccessView(nullptr, nullptr, &uavDesc, myNullBufferUAV);
		}

		{
			D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
			uavDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
			uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;

			myNullTexture2DUAV = myResourceHeapCPU.AllocateCPU();
			myDevice->CreateUnorderedAccessView(nullptr, nullptr, &uavDesc, myNullTexture2DUAV);
		}

		{
			D3D12_SHADER_RESOURCE_VIEW_DESC srvdesc = {};
			srvdesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			srvdesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
			srvdesc.Format = DXGI_FORMAT_R32_TYPELESS;
			srvdesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW;

			myNullByteBufferSRV = myResourceHeapCPU.AllocateCPU();
			myDevice->CreateShaderResourceView(nullptr, &srvdesc, myNullByteBufferSRV);
		}

		{
			D3D12_SHADER_RESOURCE_VIEW_DESC srvdesc = {};
			srvdesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			srvdesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			srvdesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			srvdesc.Texture2D.MipLevels = 1;

			myNullTexture2DSRV = myResourceHeapCPU.AllocateCPU();
			myDevice->CreateShaderResourceView(nullptr, &srvdesc, myNullTexture2DSRV);
		}
	}

	const bool SR_GraphicsDevice_DX12::Init(SC_Window* aWindow)
	{
		SC_Window_Win64* window = static_cast<SC_Window_Win64*>(aWindow);
		if (!window)
			return false;

		myWindow = window;

		SC_LOG("Initializing Direct3D 12");
		HRESULT hr = S_OK;

		IDXGIFactory4* dxgiFactory;
		hr = CreateDXGIFactory1(SR_IID_PPV_ARGS(&dxgiFactory));
		if (FAILED(hr))
		{
			return false;
		}

		D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_12_1, D3D_FEATURE_LEVEL_12_0, D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0 };
		constexpr uint numFeatureLevels = (sizeof(featureLevels) / sizeof(D3D_FEATURE_LEVEL));

		IDXGIAdapter1* deviceAdapter = nullptr;
		DXGI_ADAPTER_DESC1 deviceDesc = {};

		int adapterIndex = 0;
		uint64 maxVRAM = 0;
		IDXGIAdapter1* adapter;
		while (dxgiFactory->EnumAdapters1(adapterIndex, &adapter) != DXGI_ERROR_NOT_FOUND)
		{
			DXGI_ADAPTER_DESC1 desc;
			adapter->GetDesc1(&desc);
			if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
			{
				adapterIndex++;
				continue;
			}
			
			D3D_FEATURE_LEVEL currFeatureLevel = D3D_FEATURE_LEVEL_11_0;
			HRESULT deviceResult = E_FAIL;
			for (uint i = 0; i < numFeatureLevels; ++i)
			{
				deviceResult = D3D12CreateDevice(adapter, featureLevels[i], _uuidof(ID3D12Device), nullptr);
				if (SUCCEEDED(deviceResult))
				{
					currFeatureLevel = featureLevels[i];
					break;
				}
			}

			if (desc.DedicatedVideoMemory > maxVRAM && currFeatureLevel >= myFeatureLevel)
			{
				maxVRAM = static_cast<uint64>(desc.DedicatedVideoMemory);
				myFeatureLevel = currFeatureLevel;
				deviceAdapter = adapter;
				deviceDesc = desc;
			}
			else
				adapter->Release();

			adapterIndex++;
		}

		if (myFeatureLevel < D3D_FEATURE_LEVEL_12_0)
		{
			SC_ERROR_LOG("Your graphics-card doesn't support D3D_Feature_Level 12_0");
			return false;
		}

		{
			if (deviceDesc.VendorId == 0x10DE)
			{
				SC_LOG("Graphics Vendor: Nvidia");
			}
			else if (deviceDesc.VendorId == 0x1002)
			{
				SC_LOG("Graphics Vendor: AMD");
			}
			else if (deviceDesc.VendorId == 0x163C || deviceDesc.VendorId == 0x8086)
			{
				SC_LOG("Graphics Vendor: Intel");
			}

			std::string deviceName = ToString(deviceDesc.Description);

			SC_LOG("Graphics card: %s (id: %x rev: %x)", deviceName.c_str(), deviceDesc.DeviceId, deviceDesc.Revision);
			SC_LOG("Video Memory: %lluMB", uint64(maxVRAM >> 20));
		}

		//////////////////////
		// Create Device

		if (myEnableDebugging)
		{
			SC_LOG("Creating D3D12Device_DEBUGGABLE");
			ID3D12Debug3* debugLayer3 = nullptr;
			D3D12GetDebugInterface(SR_IID_PPV_ARGS(&debugLayer3));
			debugLayer3->EnableDebugLayer();

			if (SC_CommandLineManager::HasCommand("gbv"))
				debugLayer3->SetEnableGPUBasedValidation(true);

			debugLayer3->Release();
		}
		else
			SC_LOG("Creating D3D12Device");

		if (SC_CommandLineManager::HasCommand("dred"))
		{
			SC_LOG("Initializing DRED");
			ID3D12DeviceRemovedExtendedDataSettings1* dredSettings = nullptr;
			D3D12GetDebugInterface(IID_PPV_ARGS(&dredSettings));

			// Turn on AutoBreadcrumbs and Page Fault reporting
			dredSettings->SetAutoBreadcrumbsEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);
			dredSettings->SetPageFaultEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);
			dredSettings->Release();
		}

		SC_LOG("Using D3D_FeatureLevel (%s)", GetFeatureLevelString(myFeatureLevel));

		hr = D3D12CreateDevice(deviceAdapter, myFeatureLevel, SR_IID_PPV_ARGS(&myDevice));
		if (FAILED(hr))
		{
			SC_ERROR_LOG("Could not create device.");
			return false;
		}

		// Query Interfaces
		{
			hr = myDevice->QueryInterface(SR_IID_PPV_ARGS(&myDevice5));
			if (FAILED(hr))
			{
			}
			if (myDevice5)
				SC_LOG("Using ID3D12Device5");
		}

		switch (myFeatureLevel)
		{
		default:
		case D3D_FEATURE_LEVEL_12_1:
		case D3D_FEATURE_LEVEL_12_0:
		case D3D_FEATURE_LEVEL_11_1:
		case D3D_FEATURE_LEVEL_11_0:
			globalShaderModel = "_5_1";
			break;
		}

		SetFeatureSupport();

		//////////////////////
		// Create debug-message filters
		if (myEnableDebugging)
		{
			if (SUCCEEDED(myDevice->QueryInterface(SR_IID_PPV_ARGS(&myInfoQueue))))
			{
				D3D12_MESSAGE_SEVERITY severities[] =
				{
					D3D12_MESSAGE_SEVERITY_INFO
				};
				D3D12_MESSAGE_ID denyIds[] =
				{
					D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,
					D3D12_MESSAGE_ID_CLEARDEPTHSTENCILVIEW_MISMATCHINGCLEARVALUE
				};

				D3D12_INFO_QUEUE_FILTER newFilter = {};
				newFilter.DenyList.NumSeverities = _countof(severities);
				newFilter.DenyList.pSeverityList = severities;
				newFilter.DenyList.NumIDs = _countof(denyIds);
				newFilter.DenyList.pIDList = denyIds;
				myInfoQueue->PushStorageFilter(&newFilter);
				if (myDebugBreakOnError)
				{
					myInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
					myInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
					if (myDebugBreakOnWarning)
						myInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);
				}
			}
		}

		CreateDefaultRootSignatures();
		CreateResourceHeaps();

		//////////////////////
		// Query heaps

		D3D12_QUERY_HEAP_DESC queryDesc;
		queryDesc.Type = D3D12_QUERY_HEAP_TYPE_TIMESTAMP;
		queryDesc.Count = 2 * ourNumQueriesPerContext; // Double up for start + end timestamps, multiply by num contexts.
		queryDesc.NodeMask = 0;

		hr = myDevice->CreateQueryHeap(&queryDesc, SR_IID_PPV_ARGS(&myQueryHeap));
		assert(hr == S_OK && "Couldn't create query heap.");

		SR_GraphicsContext::SetCurrentContext(GetContext(SR_ContextType_Render));

		SC_SUCCESS_LOG("Successfully initialized Direct3D 12.");

		myShaderCompiler = new SR_ShaderCompiler_DX12();

		if (!PostInit())
			return false;

		return true;
	}
}

#endif