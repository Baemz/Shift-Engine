#include "SRender_Precompiled.h"
#if ENABLE_DX12

#include "SR_GraphicsContext_DX12.h"
#include "SR_GraphicsDevice_DX12.h"
#include "SR_QueueManager_DX12.h"
#include "SR_Buffer_DX12.h"
#include "SR_Texture_DX12.h"
#include "SR_TextureBuffer_DX12.h"
#include "SR_BufferView_DX12.h"
#include "SR_RenderTarget_DX12.h"
#include "SR_CommandList_DX12.h"
#include "SR_ShaderState_DX12.h"
#include "d3dx12.h"

namespace Shift
{

	static const float globalClearColor[] = { 0.0f, 1.f, 0.0f, 1.0f };
	static constexpr uint globalHeapOffsetCBV = 0;
	static constexpr uint globalHeapOffsetSRV = GPU_RESOURCE_HEAP_CBV_COUNT;
	static constexpr uint globalHeapOffsetUAV = GPU_RESOURCE_HEAP_CBV_COUNT + GPU_RESOURCE_HEAP_SRV_COUNT;

	SR_GraphicsContext_DX12::SR_GraphicsContext_DX12(const SR_ContextType& aType, SR_GraphicsDevice_DX12* aDevice)
		: SR_GraphicsContext(aType)
		, myDevice(aDevice)
	{
		D3D12_COMMAND_LIST_TYPE type;
		std::wstring name;
		switch (aType)
		{
		default:
		case SR_ContextType_Render:
			name = (L"Graphics Context");
			type = D3D12_COMMAND_LIST_TYPE_DIRECT;
			break;
		case SR_ContextType_Compute:
			name = (L"Compute Context");
			type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
			break;
		case SR_ContextType_CopyInit:
			name = (L"CopyInit Context");
			type = D3D12_COMMAND_LIST_TYPE_COPY;
			break;
		case SR_ContextType_CopyStream:
			name = (L"CopyStream Context");
			type = D3D12_COMMAND_LIST_TYPE_COPY;
			break;
		case SR_ContextType_CopyFast:
			name = (L"CopyFast Context");
			type = D3D12_COMMAND_LIST_TYPE_COPY;
			break;

		}
		ID3D12Device* device = myDevice->GetNativeDevice();
		myResourceDescriptorIncrementSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		myTempHeap = new SR_TempMemoryPool_DX12(true, device, 16*MB);
	}

	SR_GraphicsContext_DX12::~SR_GraphicsContext_DX12()
	{
		delete myTempHeap;
	}

	void SR_GraphicsContext_DX12::Begin()
	{
		myCommandListKeepAlive = SR_QueueManager::GetCommandList(myContextType);
		if (!myCommandListKeepAlive)
			assert(false);

		myCommandList = static_cast<SR_CommandList_DX12*>(myCommandListKeepAlive.Get());
		myNativeCommandList = myCommandList->GetCommandList();
#if SR_ENABLE_RAYTRACING
		myNativeCommandList4 = myCommandList->GetCommandList4();
#endif
		myNumCommandsSinceReset = 0;
	}

	void SR_GraphicsContext_DX12::End()
	{
		myCommandListKeepAlive.Reset();
		myNativeCommandList = nullptr;
#if ENABLE_DX12_20H1
		myNativeCommandList4 = nullptr;
#endif
		myInsertedWaitables.RemoveAll();
		myCurrentTopology = SR_Topology_Unknown;
		myCurrentBlendFactor = SC_Vector4f(0);
		myCurrentBlendState = nullptr;
		myCurrentDepthState = nullptr;
		myCurrentRasterizerState = nullptr;
		myCurrentRenderTargetFormats = nullptr;
		myCurrentStencilRef = 0;
		myCurrentResourceHeap = nullptr;
		myCurrentRootSignature = nullptr;
		myCurrentShaderState = nullptr;
		myDevice->InactivateContext(this);
		SetCurrentContext(nullptr);
		myResources.Clear();
	}

	void SR_GraphicsContext_DX12::BeginRecording()
	{
		myRootCache[Graphics].myIsFirstUpdateSinceReset = true;
		myRootCache[Compute].myIsFirstUpdateSinceReset = true;
	}

	void SR_GraphicsContext_DX12::EndRecording()
	{
		myCommandList->myNumCommands += myNumCommandsSinceReset;
		if (myIntermediateTransitions.Count() > 0)
			myCommandList->myPendingTransitions = SC_Move(myIntermediateTransitions);

		myCommandList->FinishRecording();
		InsertFence(SR_QueueManager_DX12::GetInstance()->GetFenceManager().InsertFence(this));
	}

	void SR_GraphicsContext_DX12::BeginEvent(const char* aID)
	{
#if ENABLE_PIX
		if (myUseDebugMarkers)
			PIXBeginEvent(myNativeCommandList, PIX_COLOR_DEFAULT, aID);
#else
		SC_UNUSED(aID);
#endif
	}

	void SR_GraphicsContext_DX12::EndEvent()
	{
#if ENABLE_PIX
		if (myUseDebugMarkers)
			PIXEndEvent(myNativeCommandList);
#endif
	}

	void SR_GraphicsContext_DX12::InsertWait(SR_Waitable* aEvent)
	{
		if (!aEvent)
			return;

		aEvent->myEventCPU.Wait();
		if (aEvent->myFenceContext != myContextType)
			myInsertedWaitables.Add(aEvent);
	}

	void SR_GraphicsContext_DX12::Draw(const unsigned int aVertexCount, const unsigned int aStartVertexLocation)
	{
		PreDraw();
		myNativeCommandList->DrawInstanced(aVertexCount, 1, aStartVertexLocation, 0);

		SetNeedsFlush();
		++myNumCommandsSinceReset;
	}
	void SR_GraphicsContext_DX12::DrawIndexed(const unsigned int aIndexCount, const unsigned int aStartIndexLocation, const unsigned int aBaseVertexLocation)
	{
		PreDraw();
		myNativeCommandList->DrawIndexedInstanced(aIndexCount, 1, aStartIndexLocation, aBaseVertexLocation, 0); 

		IncrementTrianglesDrawn(aIndexCount / 3);
		SetNeedsFlush();
		++myNumCommandsSinceReset;
	}
	void SR_GraphicsContext_DX12::DrawInstanced(const unsigned int aVertexCount, const unsigned int aInstanceCount, const unsigned int aStartVertexLocation, const unsigned int aStartInstanceLocation)
	{
		PreDraw();
		myNativeCommandList->DrawInstanced(aVertexCount, aInstanceCount, aStartVertexLocation, aStartInstanceLocation);

		IncrementInstancesDrawn(aInstanceCount);
		SetNeedsFlush();
		++myNumCommandsSinceReset;
	}

	void SR_GraphicsContext_DX12::DrawIndexedInstanced(const unsigned int aIndexCount, const unsigned int aInstanceCount, const unsigned int aStartIndexLocation, const unsigned int aBaseVertexLocation, const unsigned int aStartInstanceLocation)
	{
		PreDraw();
		myNativeCommandList->DrawIndexedInstanced(aIndexCount, aInstanceCount, aStartIndexLocation, aBaseVertexLocation, aStartInstanceLocation);

		IncrementInstancesDrawn(aInstanceCount);
		IncrementTrianglesDrawn(aIndexCount / 3);
		SetNeedsFlush();
		++myNumCommandsSinceReset;
	}

	void SR_GraphicsContext_DX12::Dispatch(const unsigned int aThreadGroupCountX, const unsigned int aThreadGroupCountY, const unsigned int aThreadGroupCountZ)
	{
		PreDispatch();

#if SR_ENABLE_RAYTRACING
		if (myCurrentShaderState->IsRaytracing())
		{
			SR_ShaderState_DX12* state = static_cast<SR_ShaderState_DX12*>(myCurrentShaderState);
			D3D12_DISPATCH_RAYS_DESC desc = state->myDispatchRaysDesc;
			desc.Width = aThreadGroupCountX;
			desc.Height = aThreadGroupCountY;
			desc.Depth = aThreadGroupCountZ;

			myNativeCommandList4->DispatchRays(&desc);
		}
		else
#endif
		{
			myNativeCommandList->Dispatch(aThreadGroupCountX, aThreadGroupCountY, aThreadGroupCountZ);
		}

		SetNeedsFlush();
		++myNumCommandsSinceReset;
	}

	void SR_GraphicsContext_DX12::ClearRenderTarget(SR_RenderTarget* aTarget, const SC_Vector4f& aClearColor)
	{
		const SR_RenderTarget_DX12* target = static_cast<const SR_RenderTarget_DX12*>(aTarget);

		myNativeCommandList->ClearRenderTargetView(target->myRenderTargetView, &aClearColor.x, 0, nullptr);
		SetNeedsFlush();
		++myNumCommandsSinceReset;
	}

	void SR_GraphicsContext_DX12::ClearDepthTarget(SR_RenderTarget* aTarget, float aDepthValue, uint8 aStencilValue, uint aClearFlags)
	{
		SC_UNUSED(aClearFlags);
		D3D12_CLEAR_FLAGS clearFlag = D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL;

		const SR_RenderTarget_DX12* target = static_cast<const SR_RenderTarget_DX12*>(aTarget);

		myNativeCommandList->ClearDepthStencilView(target->myDepthStencilView, clearFlag, aDepthValue, aStencilValue, 0, nullptr);
		SetNeedsFlush();
		++myNumCommandsSinceReset;
	}

	void SR_GraphicsContext_DX12::BindConstantBuffer(SR_Buffer* aBuffer, const uint aBindingIndex)
	{
		assert(aBindingIndex >= SR_ConstantBufferRef_LocalStart && aBindingIndex <= SR_ConstantBufferRef_LocalEnd);
		if (!aBuffer)
			return;

		ID3D12Device* device = myDevice->GetNativeDevice();

		SR_Buffer_DX12* buffer = static_cast<SR_Buffer_DX12*>(aBuffer);

		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
		cbvDesc.BufferLocation = buffer->myDX12Resource->GetGPUVirtualAddress();
		cbvDesc.SizeInBytes = buffer->GetProperties().mySize;

		uint index = aBindingIndex - SR_ConstantBufferRef_LocalStart;
		SR_Descriptor_DX12 handle = myDevice->AllocateDescriptorRange(1, true, this);
		device->CreateConstantBufferView(&cbvDesc, handle.myCPUHandle);

		myResources.myLocalConstants |= SR_GlobalConstantsMask(1) << index;
		myBoundResources.myLocalConstants[index] = handle.myCPUHandle;
	}

	void SR_GraphicsContext_DX12::BindConstantBuffer(void* aData, uint aSize, const uint aBindingIndex)
	{
		assert(aBindingIndex >= SR_ConstantBufferRef_LocalStart && aBindingIndex <= SR_ConstantBufferRef_LocalEnd);

		if (!aData)
			return;

		ID3D12Device* device = myDevice->GetNativeDevice();

		SR_TempAllocation tempBuffer = myTempHeap->Allocate(aSize);

		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
		cbvDesc.BufferLocation = tempBuffer.myGPUAddress;
		cbvDesc.SizeInBytes = (uint)tempBuffer.mySize;

		uint index = aBindingIndex - SR_ConstantBufferRef_LocalStart;
		SR_Descriptor_DX12 handle = myDevice->AllocateDescriptorRange(1, true, this);
		device->CreateConstantBufferView(&cbvDesc, handle.myCPUHandle);

		myResources.myLocalConstants |= SR_GlobalConstantsMask(1) << index;
		myBoundResources.myLocalConstants[index] = handle.myCPUHandle;

		SC_Memcpy(tempBuffer.myCPUAddress, aData, aSize);
	}

	void SR_GraphicsContext_DX12::BindConstantBufferRef(SR_Buffer* aBuffer, const SR_ConstantBufferRef::Enumerated& aRef)
	{
		assert(uint(aRef) >= SR_ConstantBufferRef_GlobalStart && uint(aRef) <= SR_ConstantBufferRef_GlobalEnd);

		if (!aBuffer)
			return;

		ID3D12Device* device = myDevice->GetNativeDevice();

		SR_Buffer_DX12* buffer = static_cast<SR_Buffer_DX12*>(aBuffer);

		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
		cbvDesc.BufferLocation = buffer->myDX12Resource->GetGPUVirtualAddress();
		cbvDesc.SizeInBytes = buffer->GetProperties().mySize;

		uint index = aRef - SR_ConstantBufferRef_GlobalStart;

		SR_Descriptor_DX12 handle = myDevice->AllocateDescriptorRange(1, true, this);
		device->CreateConstantBufferView(&cbvDesc, handle.myCPUHandle);

		myResources.myGlobalConstants |= SR_GlobalConstantsMask(1) << index;
		myBoundResources.myGlobalConstants[index] = handle.myCPUHandle;
	}

	void SR_GraphicsContext_DX12::BindConstantBufferRef(void* aData, uint aSize, const SR_ConstantBufferRef::Enumerated& aRef)
	{
		assert(uint(aRef) >= SR_ConstantBufferRef_GlobalStart && uint(aRef) <= SR_ConstantBufferRef_GlobalEnd);

		if (!aData)
			return;

		ID3D12Device* device = myDevice->GetNativeDevice();

		SR_TempAllocation tempBuffer = myTempHeap->Allocate(aSize);

		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
		cbvDesc.BufferLocation = tempBuffer.myGPUAddress;
		cbvDesc.SizeInBytes = (uint)tempBuffer.mySize;

		uint index = aRef - SR_ConstantBufferRef_GlobalStart;
		SR_Descriptor_DX12 handle = myDevice->AllocateDescriptorRange(1, true, this);
		device->CreateConstantBufferView(&cbvDesc, handle.myCPUHandle);

		myResources.myGlobalConstants |= SR_GlobalConstantsMask(1) << index;
		myBoundResources.myGlobalConstants[index] = handle.myCPUHandle;

		SC_Memcpy(tempBuffer.myCPUAddress, aData, aSize);
	}

	void SR_GraphicsContext_DX12::BindBuffer(SR_BufferView* aBufferView, const uint aBindingIndex)
	{
		assert(aBindingIndex >= SR_BufferRef_LocalStart && aBindingIndex <= SR_BufferRef_LocalEnd);

		if (!aBufferView)
			return;

		uint index = aBindingIndex - SR_BufferRef_LocalStart;
		SR_BufferView_DX12* buf = static_cast<SR_BufferView_DX12*>(aBufferView);

		myResources.myLocalBuffers |= SR_LocalBuffersMask(1) << index;
		myBoundResources.myLocalBuffers[index] = buf->myDescriptorCPU;
	}

	void SR_GraphicsContext_DX12::BindBufferRef(SR_BufferView* aBufferView, const SR_BufferRef::Enumerated& aRef)
	{
		assert(uint(aRef) >= SR_BufferRef_GlobalStart && uint(aRef) <= SR_BufferRef_GlobalEnd);

		if (!aBufferView)
			return;

		uint index = aRef - SR_BufferRef_GlobalStart;
		SR_BufferView_DX12* buf = static_cast<SR_BufferView_DX12*>(aBufferView);

		myResources.myGlobalBuffers |= SR_GlobalBuffersMask(1) << index;
		myBoundResources.myGlobalBuffers[index] = buf->myDescriptorCPU;
	}

	void SR_GraphicsContext_DX12::BindBufferRW(SR_BufferView* aBufferView, const uint aBindingIndex)
	{
		assert(aBindingIndex >= SR_BufferRWRef_LocalStart && aBindingIndex <= SR_BufferRWRef_LocalEnd);

		if (!aBufferView)
			return;

		uint index = aBindingIndex - SR_BufferRWRef_LocalStart;
		SR_BufferView_DX12* buf = static_cast<SR_BufferView_DX12*>(aBufferView);

		myResources.myLocalBufferRWs |= SR_LocalBuffersRWMask(1) << index;
		myBoundResources.myLocalBufferRWs[index] = buf->myDescriptorCPU;
	}

	void SR_GraphicsContext_DX12::BindBufferRWRef(SR_BufferView* aBufferView, const SR_BufferRWRef::Enumerated& aRef)
	{
		assert(uint(aRef) >= SR_BufferRWRef_GlobalStart && uint(aRef) <= SR_BufferRWRef_GlobalEnd);

		if (!aBufferView)
			return;

		uint index = aRef - SR_BufferRWRef_GlobalStart;
		SR_BufferView_DX12* buf = static_cast<SR_BufferView_DX12*>(aBufferView);

		myResources.myGlobalBufferRWs |= SR_GlobalBuffersRWMask(1) << index;
		myBoundResources.myGlobalBufferRWs[index] = buf->myDescriptorCPU;
	}
	
	void SR_GraphicsContext_DX12::BindTexture(SR_Texture* aTexture, const uint aBindingIndex)
	{
		assert(aBindingIndex >= SR_TextureRef_LocalStart && aBindingIndex <= SR_TextureRef_LocalEnd);

		if (!aTexture)
			return;
		uint index = aBindingIndex - SR_TextureRef_LocalStart;
		SR_Texture_DX12* tex = static_cast<SR_Texture_DX12*>(aTexture);

		myResources.myLocalTextures |= SR_LocalTexturesMask(1) << index;
		myBoundResources.myLocalTextures[index] = tex->myDescriptorCPU;
	}

	void SR_GraphicsContext_DX12::BindTextureRef(SR_Texture* aTexture, const SR_TextureRef::Enumerated& aRef)
	{
		assert(uint(aRef) >= SR_TextureRef_GlobalStart && uint(aRef) <= SR_TextureRef_GlobalEnd);

		if (!aTexture)
			return;

		uint index = aRef - SR_TextureRef_GlobalStart;
		SR_Texture_DX12* tex = static_cast<SR_Texture_DX12*>(aTexture);

		myResources.myGlobalTextures |= SR_GlobalTexturesMask(1) << index;
		myBoundResources.myGlobalTextures[index] = tex->myDescriptorCPU;
	}
	
	void SR_GraphicsContext_DX12::BindTextureRW(SR_Texture* aTexture, const uint aBindingIndex)
	{
		if (!aTexture)
			return;

		uint index = aBindingIndex - SR_TextureRWRef_LocalStart;
		SR_Texture_DX12* tex = static_cast<SR_Texture_DX12*>(aTexture);

		myResources.myLocalTextureRWs |= SR_LocalTexturesRWMask(1) << index;
		myBoundResources.myLocalTextureRWs[index] = tex->myDescriptorCPU;
	}

	void SR_GraphicsContext_DX12::BindTextureRWRef(SR_Texture* aTexture, const SR_TextureRWRef::Enumerated& aRef)
	{
		if (!aTexture)
			return;

		uint index = aRef - SR_TextureRWRef_GlobalStart;
		SR_Texture_DX12* tex = static_cast<SR_Texture_DX12*>(aTexture);

		myResources.myGlobalTextureRWs |= SR_GlobalTexturesRWMask(1) << index;
		myBoundResources.myGlobalTextureRWs[index] = tex->myDescriptorCPU;
	}

#if SR_ENABLE_RAYTRACING
	SC_Ref<SR_Buffer> SR_GraphicsContext_DX12::CreateAccelerationStructure(SR_Buffer_DX12* aBuffer, const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS& aInputs)
	{
		ID3D12Device5* device5 = myDevice->GetNativeDevice5();

		D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO info = {};
		device5->GetRaytracingAccelerationStructurePrebuildInfo(&aInputs, &info);

		SR_BufferDesc bufferDesc;
		bufferDesc.mySize = uint(info.ResultDataMaxSizeInBytes);
		bufferDesc.myBindFlag = SR_BindFlag_RaytracingBuffer;
		bufferDesc.myGPUAccess = SR_AccessGPU_Write;
		bufferDesc.myIsWritable = true;

		SC_Ref<SR_Buffer_DX12> buffer = aBuffer;
		if (!buffer || buffer->GetProperties().mySize < bufferDesc.mySize)
		{
			if (buffer)
				bufferDesc.mySize = SC_FindLowestPossiblePowerOfTwo(bufferDesc.mySize);

			buffer = static_cast<SR_Buffer_DX12*>(myDevice->CreateBuffer(bufferDesc, nullptr, "<temp name rt buffer>").Get());
		}

		bufferDesc.myBindFlag = SR_BindFlag_Buffer;
		bufferDesc.mySize = SC_FindLowestPossiblePowerOfTwo(SC_Max(uint(64 * 1024), uint(info.ScratchDataSizeInBytes)));
		if (!myScratchBufferTEMP || myScratchBufferTEMP->GetProperties().mySize < bufferDesc.mySize)
		{
			myScratchBufferTEMP = static_cast<SR_Buffer_DX12*>(myDevice->CreateBuffer(bufferDesc, nullptr, "RT scratch").Get());
		}

		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC desc = {};
		desc.DestAccelerationStructureData = buffer->myDX12Resource->GetGPUVirtualAddress();
		desc.ScratchAccelerationStructureData = myScratchBufferTEMP->myDX12Resource->GetGPUVirtualAddress();
		desc.Inputs = aInputs;

		if (aInputs.Flags & D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PERFORM_UPDATE)
		{
			assert(aBuffer);
			desc.SourceAccelerationStructureData = aBuffer->myDX12Resource->GetGPUVirtualAddress();
		}

		myNativeCommandList4->BuildRaytracingAccelerationStructure(&desc, 0, nullptr);
		BarrierUAV(buffer);

		++myNumCommandsSinceReset;

		return buffer;
	}

	SC_Ref<SR_Buffer> SR_GraphicsContext_DX12::CreateRTMesh(SR_Buffer* aBuffer, bool aFastUpdate, SR_Buffer* aVertexBuffer, uint aVertexStride, uint aNumVertices, SR_Buffer* aIndexBuffer, uint aNumIndices, const SC_Matrix44* aTransform)
	{
		SR_Buffer_DX12* vertexBuffer = static_cast<SR_Buffer_DX12*>(aVertexBuffer);
		SR_Buffer_DX12* indexBuffer = static_cast<SR_Buffer_DX12*>(aIndexBuffer);

		D3D12_RAYTRACING_GEOMETRY_DESC geometryDesc = {};
		geometryDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
		geometryDesc.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
		geometryDesc.Triangles.VertexBuffer.StartAddress = vertexBuffer->myDX12Resource->GetGPUVirtualAddress();
		geometryDesc.Triangles.VertexBuffer.StrideInBytes = aVertexStride;
		geometryDesc.Triangles.VertexCount = aNumVertices;
		geometryDesc.Triangles.IndexFormat = DXGI_FORMAT_R32_UINT;
		geometryDesc.Triangles.IndexBuffer = indexBuffer->myDX12Resource->GetGPUVirtualAddress();
		geometryDesc.Triangles.IndexCount = aNumIndices;

		if (aTransform)
		{
			SC_Vector4f transformPart[3] =
			{
				{aTransform->myAxisX.x, aTransform->myAxisY.x, aTransform->myAxisZ.x, aTransform->myPosition.x},
				{aTransform->myAxisX.y, aTransform->myAxisY.y, aTransform->myAxisZ.y, aTransform->myPosition.y},
				{aTransform->myAxisX.z, aTransform->myAxisY.z, aTransform->myAxisZ.z, aTransform->myPosition.z},
			};
			//uint offset;
			//SR_Buffer_DX12* buffer = static_cast<SR_Buffer_DX12*>(GetTempBuffer(offset, SR_Shader_Buffer, sizeof(t), t)); // GetTempBuffer here
			//geometryDesc.Triangles.Transform3x4 = buffer.GetGPUAddress() + offset;
		}

		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs = {};
		inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
		inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
		inputs.pGeometryDescs = &geometryDesc;
		inputs.NumDescs = 1;

		if (aFastUpdate)
		{
			uint flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE | D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_BUILD;
			if (aBuffer)
				flags |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PERFORM_UPDATE;

			inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS(flags);
		}
		else
			inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;

		return CreateAccelerationStructure(static_cast<SR_Buffer_DX12*>(aBuffer), inputs);
	}

	SC_Ref<SR_BufferView> SR_GraphicsContext_DX12::BuildRTScene(SR_BufferView* aBufferView, const SR_RaytracingInstanceData* aInstances, uint aNumInstances)
	{
		static_assert(sizeof(SR_RaytracingInstanceData) == sizeof(D3D12_RAYTRACING_INSTANCE_DESC));
		const D3D12_RAYTRACING_INSTANCE_DESC* instances = reinterpret_cast<const D3D12_RAYTRACING_INSTANCE_DESC*>(aInstances);

		uint size = aNumInstances * sizeof(D3D12_RAYTRACING_INSTANCE_DESC);
		SR_TempAllocation tempAlloc = myTempHeap->Allocate(size);
		SC_Memcpy(tempAlloc.myCPUAddress, (void*)instances, size);

		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs = {};
		inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
		inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
		inputs.InstanceDescs = tempAlloc.myGPUAddress;
		inputs.NumDescs = aNumInstances;

		SC_Ref<SR_Buffer> buffer = CreateAccelerationStructure(aBufferView ? static_cast<SR_Buffer_DX12*>(aBufferView->GetBuffer()) : nullptr, inputs);

		SR_BufferViewDesc viewDesc;
		viewDesc.myType = SR_BufferViewType_RaytracingScene;
		viewDesc.myNumElements = buffer->GetProperties().mySize;

		return myDevice->CreateBufferView(viewDesc, buffer);
	}
#endif

	void SR_GraphicsContext_DX12::Transition(const SR_ResourceState& aTransition, SR_TrackedResource* aResource)
	{
		SC_Pair pair(aTransition, aResource);
		Transition(&pair, 1);
	}

	void SR_GraphicsContext_DX12::Transition(const SC_Pair<SR_ResourceState, SR_TrackedResource*>* aPairs, uint aCount)
	{
		SC_HybridArray<D3D12_RESOURCE_BARRIER, 128> barriers;
		barriers.PreAllocate(aCount);
		for (uint i = 0; i < aCount; ++i)
		{
			const SR_ResourceState& transitionState = aPairs[i].myFirst;
			SR_TrackedResource* trackedResource = aPairs[i].mySecond;
			SC_Pair<uint, uint> pair;
			if (myIntermediateTransitions.Find(trackedResource, pair))
			{
				if (pair.mySecond == transitionState)
					continue;

				D3D12_RESOURCE_BARRIER& barrier = barriers.Add();
				barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
				barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	
				barrier.Transition.pResource = trackedResource->myDX12Resource;
				barrier.Transition.StateBefore = static_cast<D3D12_RESOURCE_STATES>(pair.mySecond);
				barrier.Transition.StateAfter = static_cast<D3D12_RESOURCE_STATES>(transitionState);
				barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

				myIntermediateTransitions[trackedResource].mySecond = transitionState;
			}
			else
			{
				pair.myFirst = transitionState;
				pair.mySecond = transitionState;
				myIntermediateTransitions.Insert(trackedResource, pair);
			}
		}

		if (barriers.Count())
		{
			myNativeCommandList->ResourceBarrier(barriers.Count(), barriers.GetBuffer());
			++myNumCommandsSinceReset;
		}
	}

	void SR_GraphicsContext_DX12::BarrierUAV(SR_TrackedResource* aResource)
	{
		BarrierUAV(&aResource, 1);
	}

	void SR_GraphicsContext_DX12::BarrierUAV(SR_TrackedResource** aResources, uint aCount)
	{
		SC_HybridArray<D3D12_RESOURCE_BARRIER, 16> barriers;
		barriers.PreAllocate(aCount);

		for (uint i = 0; i < aCount; ++i)
		{
			SR_TrackedResource* trackedResource = aResources[i];
			D3D12_RESOURCE_BARRIER& barrier = barriers.Add();
			barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
			barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			barrier.UAV.pResource = trackedResource->myDX12Resource;
		}

		if (barriers.Count())
		{
			myNativeCommandList->ResourceBarrier(barriers.Count(), barriers.GetBuffer());
			++myNumCommandsSinceReset;
		}
	}

	void SR_GraphicsContext_DX12::BarrierAlias(SR_TrackedResource* aBefore, SR_TrackedResource* aAfter)
	{
		SC_Pair pair(aBefore, aAfter);
		BarrierAlias(&pair, 1);
	}

	void SR_GraphicsContext_DX12::BarrierAlias(const SC_Pair<SR_TrackedResource*, SR_TrackedResource*>* aPairs, uint aCount)
	{
		SC_HybridArray<D3D12_RESOURCE_BARRIER, 16> barriers;
		barriers.PreAllocate(aCount);

		for (uint i = 0; i < aCount; ++i)
		{
			D3D12_RESOURCE_BARRIER& barrier = barriers.Add();
			barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_ALIASING;
			barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			barrier.Aliasing.pResourceBefore = aPairs[i].myFirst->myDX12Resource;
			barrier.Aliasing.pResourceAfter = aPairs[i].mySecond->myDX12Resource;
		}

		if (barriers.Count())
		{
			myNativeCommandList->ResourceBarrier(barriers.Count(), barriers.GetBuffer());
			++myNumCommandsSinceReset;
		}
	}

	void SR_GraphicsContext_DX12::SetBlendState(SR_BlendState* aBlendState, const SC_Vector4f& aBlendFactor)
	{
		if (!myCurrentBlendState || (myCurrentBlendState->myKey != aBlendState->myKey))
		{
			myCurrentBlendState = aBlendState;
			myCurrentPSOKey.myBlendStateKey = myCurrentBlendState->myKey;
		}

		if (myCurrentBlendFactor != aBlendFactor)
		{
			float blendFactor[] = {
				aBlendFactor.x,
				aBlendFactor.y,
				aBlendFactor.z,
				aBlendFactor.w
			};

			myNativeCommandList->OMSetBlendFactor(blendFactor);
			myCurrentBlendFactor = aBlendFactor;
		}
	}

	void SR_GraphicsContext_DX12::SetDepthState(SR_DepthState* aDepthState, uint aStencilRef)
	{
		if (!myCurrentDepthState || (myCurrentDepthState->myKey != aDepthState->myKey))
		{
			myCurrentDepthState = aDepthState;
			myCurrentPSOKey.myDepthStateKey = myCurrentDepthState->myKey;
		}

		if (myCurrentStencilRef != aStencilRef)
		{
			myNativeCommandList->OMSetStencilRef(aStencilRef);
			myCurrentStencilRef = aStencilRef;
		}
	}

	void SR_GraphicsContext_DX12::SetRasterizerState(SR_RasterizerState* aRasterizerState)
	{
		if (!myCurrentRasterizerState || (myCurrentRasterizerState->myKey != aRasterizerState->myKey))
		{
			myCurrentRasterizerState = aRasterizerState;
			myCurrentPSOKey.myRasterStateKey = myCurrentRasterizerState->myKey;
		}
	}

	void SR_GraphicsContext_DX12::SetViewport(const SR_Viewport& aViewport)
	{
		const SR_Viewport nullViewport;
		if (aViewport == nullViewport)
		{
			myNativeCommandList->RSSetViewports(0, nullptr);
			return;
		}
		D3D12_VIEWPORT viewport;
		viewport.TopLeftX = aViewport.topLeftX;
		viewport.TopLeftY = aViewport.topLeftY;
		viewport.Width = aViewport.width;
		viewport.Height = aViewport.height;
		viewport.MinDepth = aViewport.minDepth;
		viewport.MaxDepth = aViewport.maxDepth;
		myNativeCommandList->RSSetViewports(1, &viewport);
	}

	void SR_GraphicsContext_DX12::SetScissorRect(const SR_ScissorRect& aScissorRect)
	{
		D3D12_RECT scissor;
		scissor.top = aScissorRect.top;
		scissor.left = aScissorRect.left;
		scissor.right = aScissorRect.right;
		scissor.bottom = aScissorRect.bottom;
		myNativeCommandList->RSSetScissorRects(1, &scissor);
	}

	void SR_GraphicsContext_DX12::SetTopology(const SR_Topology& aTopology)
	{
		if (myCurrentTopology == aTopology)
			return;

		myCurrentPSOKey.myTopologyKey = SC_Hash(aTopology);

		D3D_PRIMITIVE_TOPOLOGY topology;
		switch (aTopology)
		{
		default:
		case SR_Topology_TriangleList:
			topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
			break;
		case SR_Topology_LineList:
			topology = D3D_PRIMITIVE_TOPOLOGY_LINELIST;
			break;
		case SR_Topology_PointList:
			topology = D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
			break;
		case SR_Topology_Patch:
			topology = D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST;
			break;
		}

		myNativeCommandList->IASetPrimitiveTopology(topology);
		myCurrentTopology = aTopology;
	}

	void SR_GraphicsContext_DX12::SetVertexBuffer(uint aStartVertex, SR_Buffer* aBuffer)
	{
		SetVertexBuffer(aStartVertex, &aBuffer, 1);
	}

	void SR_GraphicsContext_DX12::SetVertexBuffer(uint aStartVertex, SR_Buffer** aBuffer, uint aCount)
	{
		SC_HybridArray<D3D12_VERTEX_BUFFER_VIEW, 8> views;
		views.PreAllocate(aCount);

		SC_HybridArray<SC_Pair<SR_ResourceState, SR_TrackedResource*>, 8> transitions;
		transitions.PreAllocate(aCount);

		for (uint i = 0; i < aCount; ++i)
		{
			SR_Buffer_DX12* buffer = static_cast<SR_Buffer_DX12*>(aBuffer[i]);
			const SR_BufferDesc& bufferDesc = buffer->GetProperties();

			if ((bufferDesc.myBindFlag & SBindFlag_VertexBuffer) == 0)
			{
				SC_ERROR_LOG("Tried to bind non-vertex buffer as vertex buffer (buffer: %p)", buffer);
				continue;
			}

			transitions.Add(SC_Pair(SR_ResourceState_VertexConstantBuffer, buffer));

			D3D12_VERTEX_BUFFER_VIEW& vbView = views.Add();
			vbView.BufferLocation = buffer->myDX12Resource->GetGPUVirtualAddress();
			vbView.SizeInBytes = buffer->GetProperties().mySize;
			vbView.StrideInBytes = buffer->GetProperties().myStructSize;
		}
		Transition(transitions.GetBuffer(), transitions.Count());
		myNativeCommandList->IASetVertexBuffers(aStartVertex, views.Count(), views.GetBuffer());
	}

	void SR_GraphicsContext_DX12::SetVertexBuffer(uint aStartVertex, SR_Buffer* aVertexBuffer, uint aVertexBufferOffset, SR_Buffer* aInstanceBuffer, uint aInstanceBufferOffset)
	{
		D3D12_VERTEX_BUFFER_VIEW views[2] = {};

		SC_HybridArray<SC_Pair<SR_ResourceState, SR_TrackedResource*>, 8> transitions;
		transitions.PreAllocate(2);

		for (uint i = 0; i < 2; ++i)
		{
			SR_Buffer_DX12* buffer = static_cast<SR_Buffer_DX12*>((i == 0) ? aVertexBuffer : aInstanceBuffer);
			const SR_BufferDesc& bufferDesc = buffer->GetProperties();

			if ((bufferDesc.myBindFlag & SBindFlag_VertexBuffer) == 0)
			{
				SC_ERROR_LOG("Tried to bind non-vertex buffer as vertex buffer (buffer: %p)", buffer);
				continue;
			}

			transitions.Add(SC_Pair(SR_ResourceState_VertexConstantBuffer, buffer));

			uint offset = (i == 0) ? aVertexBufferOffset : aInstanceBufferOffset;
			D3D12_VERTEX_BUFFER_VIEW& vbView = views[i];
			vbView.BufferLocation = buffer->myDX12Resource->GetGPUVirtualAddress() + offset;
			vbView.SizeInBytes = buffer->GetProperties().mySize - offset;
			vbView.StrideInBytes = buffer->GetProperties().myStructSize;
		}
		Transition(transitions.GetBuffer(), transitions.Count());
		myNativeCommandList->IASetVertexBuffers(aStartVertex, 2, &views[0]);
	}

	void SR_GraphicsContext_DX12::SetIndexBuffer(SR_Buffer* aIndexBuffer)
	{
		SR_Buffer_DX12* buffer = static_cast<SR_Buffer_DX12*>(aIndexBuffer); 
		const SR_BufferDesc& bufferDesc = buffer->GetProperties();

		if ((bufferDesc.myBindFlag & SBindFlag_IndexBuffer) == 0)
		{
			SC_ERROR_LOG("Tried to bind non-index buffer as index buffer (buffer: %p)", buffer);
			assert(false);
		}

		Transition(SR_ResourceState_IndexBuffer, buffer);
		D3D12_INDEX_BUFFER_VIEW view;
		view.BufferLocation = buffer->myDX12Resource->GetGPUVirtualAddress();
		view.Format = DXGI_FORMAT_R32_UINT;
		view.SizeInBytes = bufferDesc.mySize;
		myNativeCommandList->IASetIndexBuffer(&view);
	}

	void SR_GraphicsContext_DX12::SetShaderState(SR_ShaderState* aShaderState)
	{
		SR_ShaderState_DX12* shaderState = static_cast<SR_ShaderState_DX12*>(aShaderState);

		if (myCurrentShaderState == aShaderState)
			return;

		SetDescriptorHeaps();
		BindRootSignature(shaderState->myRootSignature);

#if SR_ENABLE_RAYTRACING
		if (shaderState->myIsRaytracing)
		{
			ID3D12StateObject* so = shaderState->GetStateObject();
			assert(so);
			myNativeCommandList4->SetPipelineState1(so);
		}
		else
#endif
		{
			ID3D12PipelineState* pso = shaderState->GetPSO(myCurrentPSOKey);

			if (!pso)
			{
				// TODO: on the fly creation of PSO here.
				assert(false);
			}

			myNativeCommandList->SetPipelineState(pso);
		}

		myCurrentShaderState = aShaderState;
	}

	void SR_GraphicsContext_DX12::SetRenderTargets(SR_RenderTarget** aTargets, uint aNumTargets, SR_RenderTarget* aDepthStencil, uint /*aDepthAccess*/)
	{
		SC_ASSERT(aNumTargets <= SR_MAX_RENDER_TARGETS, "Cannot bind more than 6 render-targets.");

		SR_RenderTargetFormats rtf;
		rtf.myNumColorFormats = (uint8)aNumTargets;
		SR_DescriptorCPU_DX12 rtvHandles[SR_MAX_RENDER_TARGETS];
		for (uint i = 0; i < aNumTargets; ++i)
		{
			if (aTargets[i])
			{
				SR_RenderTarget_DX12* target = static_cast<SR_RenderTarget_DX12*>(aTargets[i]);

				assert(target->myRenderTargetView && "Trying to bind invalid RenderTargetView");

				rtvHandles[i] = target->myRenderTargetView;
				rtf.myColorFormats[i] = target->GetTextureBuffer()->GetProperties().myFormat;
			}
		}

		SR_DescriptorCPU_DX12* dsvHandle = nullptr;
		if (aDepthStencil)
		{
			SR_RenderTarget_DX12* target = static_cast<SR_RenderTarget_DX12*>(aDepthStencil);

			assert(target->myDepthStencilView && "Trying to bind invalid DepthStencilView");

			dsvHandle = &target->myDepthStencilView;
		}

		myCurrentPSOKey.myRenderTargetsKey = SC_Hash(rtf);
		myNativeCommandList->OMSetRenderTargets(aNumTargets, rtvHandles, false, dsvHandle);
	}
	void SR_GraphicsContext_DX12::CopyBuffer(SR_Buffer* aDestination, SR_Buffer* aSource)
	{
		SR_Buffer_DX12* sourceBuffer = static_cast<SR_Buffer_DX12*>(aSource);
		SR_Buffer_DX12* destBuffer = static_cast<SR_Buffer_DX12*>(aDestination);
		ID3D12Resource* src = sourceBuffer->myDX12Resource;
		ID3D12Resource* dst = destBuffer->myDX12Resource;

		SC_Pair<SR_ResourceState, SR_TrackedResource*> transitions[2];

		transitions[0] = SC_Pair(SR_ResourceState_CopyDest, destBuffer);
		transitions[1] = SC_Pair(SR_ResourceState_CopySrc, sourceBuffer);

		Transition(transitions, 2);
		myNativeCommandList->CopyResource(dst, src);
		SetNeedsFlush();
		++myNumCommandsSinceReset;
	}
	void SR_GraphicsContext_DX12::CopyTexture(SR_TextureBuffer* aDestination, SR_TextureBuffer* aSource)
	{
		SR_TextureBuffer_DX12* sourceBuffer = static_cast<SR_TextureBuffer_DX12*>(aSource);
		SR_TextureBuffer_DX12* destBuffer = static_cast<SR_TextureBuffer_DX12*>(aDestination);
		ID3D12Resource* src = sourceBuffer->myDX12Resource;
		ID3D12Resource* dst = destBuffer->myDX12Resource;

		SC_Pair<SR_ResourceState, SR_TrackedResource*> transitions[2];

		transitions[0] = SC_Pair(SR_ResourceState_CopyDest, destBuffer);
		transitions[1] = SC_Pair(SR_ResourceState_CopySrc, sourceBuffer);

		Transition(transitions, 2);
		myNativeCommandList->CopyResource(dst, src);
		SetNeedsFlush();
		++myNumCommandsSinceReset;
	}

	void SR_GraphicsContext_DX12::UpdateBufferData(uint aByteSize, void* aData, SR_Buffer* aOutBuffer)
	{
		bool needsInitialTransition = true;
		const uint actualNewByteSize = aByteSize;
		const uint currentByteSize = aOutBuffer->GetProperties().mySize;
		SR_Buffer_DX12* bufferDX12 = static_cast<SR_Buffer_DX12*>(aOutBuffer);
		ID3D12Resource* buffer = bufferDX12->myDX12Resource;
		if (actualNewByteSize > currentByteSize)
		{
			buffer->Release();
			ID3D12Resource* newBuffer;
			HRESULT hr = S_OK;
			{
				D3D12_HEAP_PROPERTIES heapProps;
				heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
				heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
				heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
				heapProps.CreationNodeMask = 1;
				heapProps.VisibleNodeMask = 1;

				D3D12_RESOURCE_DESC resourceDesc;
				resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
				resourceDesc.Alignment = 0;
				resourceDesc.Width = actualNewByteSize;
				resourceDesc.Height = 1;
				resourceDesc.DepthOrArraySize = 1;
				resourceDesc.MipLevels = 1;
				resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
				resourceDesc.SampleDesc.Count = 1;
				resourceDesc.SampleDesc.Quality = 0;
				resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
				resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

				hr = myDevice->GetNativeDevice()->CreateCommittedResource(
					&heapProps,
					D3D12_HEAP_FLAG_NONE,
					&resourceDesc,
					D3D12_RESOURCE_STATE_COPY_DEST,
					nullptr,
					SR_IID_PPV_ARGS(&newBuffer));

				if (FAILED(hr))
					return;

				newBuffer->SetName(L"Buffer Resource Heap");
				buffer = newBuffer;
				bufferDX12->myDX12Resource = buffer;
				bufferDX12->myDX12Tracking.myState = SR_ResourceState_CopyDest;
				needsInitialTransition = false;
			}
		}

		SR_TempAllocation tempAlloc = myTempHeap->Allocate(actualNewByteSize);
		ID3D12Resource* uploadHeap = (ID3D12Resource*)tempAlloc.myBuffer;
		SC_Memcpy(tempAlloc.myCPUAddress, aData, actualNewByteSize);

		Transition(SR_ResourceState_CopyDest, bufferDX12);

		myNativeCommandList->CopyBufferRegion(buffer, 0, uploadHeap, tempAlloc.myOffset, actualNewByteSize);

		if (aOutBuffer->myDescription.myBindFlag & SR_BindFlag_ConstantBuffer ||
			aOutBuffer->myDescription.myBindFlag & SR_BindFlag_VertexBuffer)
			Transition(SR_ResourceState_VertexConstantBuffer, bufferDX12);
		else if (aOutBuffer->myDescription.myBindFlag & SR_BindFlag_Buffer)
			Transition(SR_ResourceState_Common, bufferDX12);
		else if (aOutBuffer->myDescription.myBindFlag & SR_BindFlag_IndexBuffer)
			Transition(SR_ResourceState_IndexBuffer, bufferDX12);

		aOutBuffer->myDescription.mySize = aByteSize;

		SetNeedsFlush();
		++myNumCommandsSinceReset;
	}

	void SR_GraphicsContext_DX12::GenerateMips(SR_Texture& aTexture)
	{
		aTexture;
	}

	void SR_GraphicsContext_DX12::ResolveQuery(const EQueryType& aType, uint aStartIndex, uint aNumQueries, SR_Buffer* aOutBuffer)
	{
		SR_Buffer_DX12* buffer = static_cast<SR_Buffer_DX12*>(aOutBuffer);
		ID3D12Resource* resource = buffer->myDX12Resource;

		D3D12_QUERY_TYPE type = static_cast<D3D12_QUERY_TYPE>(aType);

		myNativeCommandList->ResolveQueryData(myDevice->GetQueryHeap(), type, aStartIndex, aNumQueries, resource, 0);
		SetNeedsFlush();
		++myNumCommandsSinceReset;
	}

	void SR_GraphicsContext_DX12::EndQuery(const EQueryType& aType, uint aIndex)
	{
		D3D12_QUERY_TYPE type = static_cast<D3D12_QUERY_TYPE>(aType);
		myNativeCommandList->EndQuery(myDevice->GetQueryHeap(), type, aIndex);
	}

	SR_CommandList* SR_GraphicsContext_DX12::GetCommandList()
	{
		return myCommandList;
	}

	ID3D12GraphicsCommandList* SR_GraphicsContext_DX12::GetNativeCommandList() const
	{
		return myNativeCommandList;
	}

	void SR_GraphicsContext_DX12::InsertFence(const SR_Fence& aFence)
	{
		myCommandList->SetLastFence(aFence);

		if (GetType() < SR_ContextType_CopyInit && (myLastFence != aFence))
			myTempHeap->FinishFrame(aFence.myFenceValue, SR_QueueManager_DX12::GetInstance()->GetFenceManager().GetLastInsertedFence(myContextType).myFenceValue);

		myLastFence = aFence;
	}

	void SR_GraphicsContext_DX12::PreDraw()
	{
		IncrementDrawCalls();
		SetResources();
	}

	void SR_GraphicsContext_DX12::PreDispatch()
	{
		IncrementDispatchCalls();
		SetResources();
	}

	void SR_GraphicsContext_DX12::SetDescriptorHeaps()
	{
		const SR_DescriptorHeap_DX12& heap = myDevice->GetGPUVisibleDescriptorHeap();
		ID3D12DescriptorHeap* descriptorHeap = heap.GetDescriptorHeap();
		if (descriptorHeap != myCurrentResourceHeap)
		{
			myNativeCommandList->SetDescriptorHeaps(1, &descriptorHeap);
			myCurrentResourceHeap = descriptorHeap;
		}
	}

	void SR_GraphicsContext_DX12::SetDescriptorTable(bool aIsCompute, uint aTable, const SR_Descriptor_DX12& aDescriptor)
	{
		const SR_DescriptorGPU_DX12 gpuHandle = aDescriptor.myGPUHandle;
		SR_DescriptorGPU_DX12& cachedHandle = myRootCache[aIsCompute].myTables[aTable].myGPUHandle;
		if (gpuHandle != cachedHandle)
		{
			cachedHandle = gpuHandle;
			if (aIsCompute)
				myNativeCommandList->SetComputeRootDescriptorTable(aTable, gpuHandle);
			else
				myNativeCommandList->SetGraphicsRootDescriptorTable(aTable, gpuHandle);
		}
	}

	void SR_GraphicsContext_DX12::SetResources()
	{
		assert(myCurrentRootSignature);

		SR_RootCache_DX12& rootCache = myRootCache[myCurrentRootSignature->IsCompute() || myCurrentRootSignature->IsRaytracing()];

		if (rootCache.myIsFirstUpdateSinceReset)
		{
			myDevice->GetLatestGlobalResources(this);
			rootCache.myIsFirstUpdateSinceReset = false;
		}

		// Constants
		if (myResources.myLocalConstants)
			SetResourcesInTable(myResources.myLocalConstants, rootCache.myBoundResourcesInTable.myLocalConstants, rootCache.myBoundResourcesSinceLastTableUpdate.myLocalConstants,
				SR_ConstantBufferRef_LocalStart, SR_RootSignature_DX12::LocalConstants, myCurrentShaderState,
				myBoundResources.myLocalConstants, SR_ConstantBufferRef_NumLocals, &myDevice->myNullCBV);

		if (myResources.myGlobalConstants)
			SetResourcesInTable(myResources.myGlobalConstants, rootCache.myBoundResourcesInTable.myGlobalConstants, rootCache.myBoundResourcesSinceLastTableUpdate.myGlobalConstants,
				SR_ConstantBufferRef_GlobalStart, SR_RootSignature_DX12::GlobalConstants, myCurrentShaderState,
				myBoundResources.myGlobalConstants, SR_ConstantBufferRef_NumGlobals, &myDevice->myNullCBV);
		
		// Textures
		if (myResources.myLocalTextures)
			SetResourcesInTable(myResources.myLocalTextures, rootCache.myBoundResourcesInTable.myLocalTextures, rootCache.myBoundResourcesSinceLastTableUpdate.myLocalTextures,
				SR_TextureRef_LocalStart, SR_RootSignature_DX12::LocalTextures, myCurrentShaderState,
				myBoundResources.myLocalTextures, SR_TextureRef_NumLocals, &myDevice->myNullTexture2DSRV);

		if (myResources.myGlobalTextures)
			SetResourcesInTable(myResources.myGlobalTextures, rootCache.myBoundResourcesInTable.myGlobalTextures, rootCache.myBoundResourcesSinceLastTableUpdate.myGlobalTextures,
				SR_TextureRef_GlobalStart, SR_RootSignature_DX12::GlobalTextures, myCurrentShaderState,
				myBoundResources.myGlobalTextures, SR_TextureRef_NumGlobals, &myDevice->myNullTexture2DSRV);
		
		// Buffers
		if (myResources.myLocalBuffers)
			SetResourcesInTable(myResources.myLocalBuffers, rootCache.myBoundResourcesInTable.myLocalBuffers, rootCache.myBoundResourcesSinceLastTableUpdate.myLocalBuffers,
				SR_BufferRef_LocalStart, SR_RootSignature_DX12::LocalBuffers, myCurrentShaderState,
				myBoundResources.myLocalBuffers, SR_BufferRef_NumLocals, &myDevice->myNullByteBufferSRV);

		if (myResources.myGlobalBuffers)
			SetResourcesInTable(myResources.myGlobalBuffers, rootCache.myBoundResourcesInTable.myGlobalBuffers, rootCache.myBoundResourcesSinceLastTableUpdate.myGlobalBuffers,
				SR_BufferRef_GlobalStart, SR_RootSignature_DX12::GlobalBuffers, myCurrentShaderState,
				myBoundResources.myGlobalBuffers, SR_BufferRef_NumGlobals, &myDevice->myNullByteBufferSRV);

		// TextureRWs
		if (myResources.myLocalTextureRWs)
			SetResourcesInTable(myResources.myLocalTextureRWs, rootCache.myBoundResourcesInTable.myLocalTextureRWs, rootCache.myBoundResourcesSinceLastTableUpdate.myLocalTextureRWs,
				SR_TextureRWRef_LocalStart, SR_RootSignature_DX12::LocalRWTextures, myCurrentShaderState,
				myBoundResources.myLocalTextureRWs, SR_TextureRWRef_NumLocals, &myDevice->myNullTexture2DUAV);

		if (myResources.myGlobalTextureRWs)
			SetResourcesInTable(myResources.myGlobalTextureRWs, rootCache.myBoundResourcesInTable.myGlobalTextureRWs, rootCache.myBoundResourcesSinceLastTableUpdate.myGlobalTextureRWs,
				SR_TextureRWRef_GlobalStart, SR_RootSignature_DX12::GlobalRWTextures, myCurrentShaderState,
				myBoundResources.myGlobalTextureRWs, SR_TextureRWRef_NumGlobals, &myDevice->myNullTexture2DUAV);

		// BufferRWs
		if (myResources.myLocalBufferRWs)
			SetResourcesInTable(myResources.myLocalBufferRWs, rootCache.myBoundResourcesInTable.myLocalBufferRWs, rootCache.myBoundResourcesSinceLastTableUpdate.myLocalBufferRWs,
				SR_BufferRWRef_LocalStart, SR_RootSignature_DX12::LocalRWBuffers, myCurrentShaderState,
				myBoundResources.myLocalBufferRWs, SR_BufferRWRef_NumLocals, &myDevice->myNullBufferUAV);

		if (myResources.myGlobalBufferRWs)
			SetResourcesInTable(myResources.myGlobalBufferRWs, rootCache.myBoundResourcesInTable.myGlobalBufferRWs, rootCache.myBoundResourcesSinceLastTableUpdate.myGlobalBufferRWs,
				SR_BufferRWRef_GlobalStart, SR_RootSignature_DX12::GlobalRWBuffers, myCurrentShaderState,
				myBoundResources.myGlobalBufferRWs, SR_BufferRWRef_NumGlobals, &myDevice->myNullBufferUAV);
	}

	template<class ResourceMask>
	void SR_GraphicsContext_DX12::UpdateResourcesInTable(ResourceMask& aResourceMask, ResourceMask& aBoundResourcesInTableMask, ResourceMask& aBoundResourcesSinceLastTableUpdateMask)
	{
		if (aResourceMask & ~aBoundResourcesInTableMask)
		{
			aResourceMask &= ~aBoundResourcesInTableMask;
			aBoundResourcesSinceLastTableUpdateMask &= ~aResourceMask;
			aBoundResourcesInTableMask |= aResourceMask;
		}
		else
			aResourceMask = 0;
	}

	template<typename ResourceMask>
	void SR_GraphicsContext_DX12::SetResourcesInTable(ResourceMask& aResourceMask, ResourceMask& aBoundResourcesInTableMask, ResourceMask& aBoundResourcesSinceLastTableUpdateMask,
		uint /*aResourceTypeStartIndex*/, uint aRootTable, SR_ShaderState* aShaderState, SR_DescriptorCPU_DX12* aBoundDescriptors, uint aNumBoundDescriptors, const SR_DescriptorCPU_DX12* aNullDescriptor)
	{
		SR_ShaderState_DX12* shaderState = static_cast<SR_ShaderState_DX12*>(aShaderState);

		bool isCompute = aShaderState->IsCompute() || aShaderState->IsRaytracing();
		uint tableSize = shaderState->myRootSignature->GetNumDescriptors(aRootTable);
		SR_Descriptor_DX12 descriptor = myRootCache[isCompute].myTables[aRootTable];

		ResourceMask updateResourceMask = aResourceMask;
		if ((aResourceMask & aBoundResourcesInTableMask & aBoundResourcesSinceLastTableUpdateMask) || !descriptor.myCPUHandle)
		{
			descriptor = myDevice->AllocateDescriptorRange(tableSize, false, this);

			aBoundResourcesSinceLastTableUpdateMask = 0;
			aBoundResourcesInTableMask = aResourceMask;
		}
		else
			UpdateResourcesInTable(updateResourceMask, aBoundResourcesInTableMask, aBoundResourcesSinceLastTableUpdateMask);

		ID3D12Device* device = myDevice->GetNativeDevice();
		ResourceMask remainingBits = updateResourceMask;
		while (remainingBits) 
		{
			uint indexInTable = SC_FindFirstBit(remainingBits);
			assert(indexInTable < aNumBoundDescriptors);
			//uint actualIndex = indexInTable + aResourceTypeStartIndex;
			ResourceMask bit = ResourceMask(1) << indexInTable;
			remainingBits ^= bit;

			SR_DescriptorCPU_DX12 dstDescriptor = descriptor.myCPUHandle;
			dstDescriptor.ptr += indexInTable * myResourceDescriptorIncrementSize;
			const SR_DescriptorCPU_DX12& srcDescriptor = aBoundDescriptors[indexInTable];

			if (srcDescriptor && (updateResourceMask & bit))
				device->CopyDescriptorsSimple(1, dstDescriptor, srcDescriptor, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
			else if (aNullDescriptor && (updateResourceMask & bit))
				device->CopyDescriptorsSimple(1, dstDescriptor, *aNullDescriptor, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		}
		SetDescriptorTable(isCompute, aRootTable, descriptor);
	}

	void SR_GraphicsContext_DX12::BindRootSignature(SR_RootSignature_DX12* aRootSignature)
	{
		if (myCurrentRootSignature != aRootSignature)
		{
			if (aRootSignature->IsCompute() || aRootSignature->IsRaytracing())
				myNativeCommandList->SetComputeRootSignature(aRootSignature->myRootSignature);
			else
				myNativeCommandList->SetGraphicsRootSignature(aRootSignature->myRootSignature);

			myCurrentRootSignature = aRootSignature;
		}
	}

}
#endif