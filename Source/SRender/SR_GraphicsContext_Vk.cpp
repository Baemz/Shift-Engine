#include "SRender_Precompiled.h"
#include "SR_GraphicsContext_Vk.h"

#if ENABLE_VULKAN

#include "SR_RenderTarget_Vk.h"

namespace Shift
{
	SR_GraphicsContext_Vk::SR_GraphicsContext_Vk(const SR_ContextType& aType, SR_GraphicsDevice_Vk* aDevice)
		: SR_GraphicsContext(aType)
		, myDevice(aDevice)
	{
	}

	SR_GraphicsContext_Vk::~SR_GraphicsContext_Vk()
	{
	}

	void SR_GraphicsContext_Vk::Begin()
	{
	}

	void SR_GraphicsContext_Vk::End()
	{
	}

	void SR_GraphicsContext_Vk::BeginRecording()
	{
	}

	void SR_GraphicsContext_Vk::EndRecording()
	{
	}

	void SR_GraphicsContext_Vk::BeginEvent(const char* /*aID*/)
	{
	}

	void SR_GraphicsContext_Vk::EndEvent()
	{
	}

	void SR_GraphicsContext_Vk::InsertWait(SR_Waitable* /*aEvent*/)
	{
	}

	void SR_GraphicsContext_Vk::Draw(const unsigned int aVertexCount, const unsigned int aStartVertexLocation)
	{
		vkCmdDraw(myCommandBuffer, aVertexCount, 1, aStartVertexLocation, 0);
	}

	void SR_GraphicsContext_Vk::DrawIndexed(const unsigned int aIndexCount, const unsigned int aStartIndexLocation, const unsigned int aBaseVertexLocation)
	{
		vkCmdDrawIndexed(myCommandBuffer, aIndexCount, 1, aStartIndexLocation, aBaseVertexLocation, 0);
	}

	void SR_GraphicsContext_Vk::DrawInstanced(const unsigned int aVertexCount, const unsigned int aInstanceCount, const unsigned int aStartVertexLocation, const unsigned int aStartInstanceLocation)
	{
		vkCmdDraw(myCommandBuffer, aVertexCount, aInstanceCount, aStartVertexLocation, aStartInstanceLocation);
	}

	void SR_GraphicsContext_Vk::DrawIndexedInstanced(const unsigned int aIndexCount, const unsigned int aInstanceCount, const unsigned int aStartIndexLocation, const unsigned int aBaseVertexLocation, const unsigned int aStartInstanceLocation)
	{
		vkCmdDrawIndexed(myCommandBuffer, aIndexCount, aInstanceCount, aStartIndexLocation, aBaseVertexLocation, aStartInstanceLocation);
	}

	void SR_GraphicsContext_Vk::Dispatch(const unsigned int aThreadGroupCountX, const unsigned int aThreadGroupCountY, const unsigned int aThreadGroupCountZ)
	{
		vkCmdDispatch(myCommandBuffer, aThreadGroupCountX, aThreadGroupCountY, aThreadGroupCountZ);
	}

	void SR_GraphicsContext_Vk::ClearRenderTarget(SR_RenderTarget* aTarget, const SC_Vector4f& aClearColor)
	{
		SR_RenderTarget_Vk* tex = static_cast<SR_RenderTarget_Vk*>(aTarget);
		tex->IsLoaded();

		VkImage image;
		VkClearColorValue clearColor;
		clearColor.float32[0] = aClearColor.r;
		clearColor.float32[1] = aClearColor.g;
		clearColor.float32[2] = aClearColor.b;
		clearColor.float32[3] = aClearColor.a;

		VkImageSubresourceRange subResourceRange = {};
		subResourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subResourceRange.baseMipLevel = 0;
		subResourceRange.levelCount = 1;
		subResourceRange.baseArrayLayer = 0;
		subResourceRange.layerCount = 1;

		vkCmdClearColorImage(myCommandBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clearColor, 1, &subResourceRange);
	}

	void SR_GraphicsContext_Vk::ClearDepthTarget(SR_RenderTarget* /*aTarget*/, float /*aDepthValue*/, uint8 /*aStencilValue*/, uint /*aClearFlags*/)
	{
	}

	void SR_GraphicsContext_Vk::BindConstantBuffer(SR_Buffer* /*aBuffer*/, const uint /*aBindingIndex*/)
	{
	}

	void SR_GraphicsContext_Vk::BindConstantBuffer(void* /*aData*/, uint /*aSize*/, const uint /*aBindingIndex*/)
	{
	}

	void SR_GraphicsContext_Vk::BindConstantBufferRef(SR_Buffer* /*aBuffer*/, const SR_ConstantBufferRef::Enumerated& /*aRef*/)
	{
	}

	void SR_GraphicsContext_Vk::BindConstantBufferRef(void* /*aData*/, uint /*aSize*/, const SR_ConstantBufferRef::Enumerated& /*aRef*/)
	{
	}

	void SR_GraphicsContext_Vk::BindBuffer(SR_BufferView* /*aBufferView*/, const uint /*aBindingIndex*/)
	{
	}

	void SR_GraphicsContext_Vk::BindBufferRef(SR_BufferView* /*aBufferView*/, const SR_BufferRef::Enumerated& /*aRef*/)
	{
	}

	void SR_GraphicsContext_Vk::BindBufferRW(SR_BufferView* /*aBufferView*/, const uint /*aBindingIndex*/)
	{
	}

	void SR_GraphicsContext_Vk::BindBufferRWRef(SR_BufferView* /*aBufferView*/, const SR_BufferRWRef::Enumerated& /*aRef*/)
	{
	}

	void SR_GraphicsContext_Vk::BindTexture(SR_Texture* /*aTexture*/, const uint /*aBindingIndex*/)
	{
	}

	void SR_GraphicsContext_Vk::BindTextureRef(SR_Texture* /*aTexture*/, const SR_TextureRef::Enumerated& /*aRef*/)
	{
	}

	void SR_GraphicsContext_Vk::BindTextureRW(SR_Texture* /*aTexture*/, const uint /*aBindingIndex*/)
	{
	}

	void SR_GraphicsContext_Vk::BindTextureRWRef(SR_Texture* /*aTexture*/, const SR_TextureRWRef::Enumerated& /*aRef*/)
	{
	}

#if SR_ENABLE_RAYTRACING
	SC_Ref<SR_Buffer> SR_GraphicsContext_Vk::CreateRTMesh(SR_Buffer* /*aBuffer*/, bool /*aFastUpdate*/, SR_Buffer* /*aVertexBuffer*/, uint /*aVertexStride*/, uint /*aNumVertices*/, SR_Buffer* /*aIndexBuffer*/, uint /*aNumIndices*/, const SC_Matrix44* /*aTransform*/)
	{
		return SC_Ref<SR_Buffer>();
	}

	SC_Ref<SR_BufferView> SR_GraphicsContext_Vk::BuildRTScene(SR_BufferView* /*aBufferView*/, const SR_RaytracingInstanceData* /*aInstances*/, uint /*aNumInstances*/)
	{
		return SC_Ref<SR_BufferView>();
	}
#endif

	void SR_GraphicsContext_Vk::Transition(const SR_ResourceState& /*aTransition*/, SR_TrackedResource* /*aResource*/)
	{
	}

	void SR_GraphicsContext_Vk::Transition(const SC_Pair<SR_ResourceState, SR_TrackedResource*>* /*aPairs*/, uint /*aCount*/)
	{
	}

	void SR_GraphicsContext_Vk::BarrierUAV(SR_TrackedResource* /*aResource*/)
	{
	}

	void SR_GraphicsContext_Vk::BarrierUAV(SR_TrackedResource** /*aResources*/, uint /*aCount*/)
	{
	}

	void SR_GraphicsContext_Vk::BarrierAlias(SR_TrackedResource* /*aBefore*/, SR_TrackedResource* /*aAfter*/)
	{
	}

	void SR_GraphicsContext_Vk::BarrierAlias(const SC_Pair<SR_TrackedResource*, SR_TrackedResource*>* /*aPairs*/, uint /*aCount*/)
	{
	}

	void SR_GraphicsContext_Vk::SetBlendState(SR_BlendState* /*aBlendState*/, const SC_Vector4f& /*aBlendFactor*/)
	{
	}

	void SR_GraphicsContext_Vk::SetDepthState(SR_DepthState* /*aDepthState*/, uint /*aStencilRef*/)
	{
	}

	void SR_GraphicsContext_Vk::SetRasterizerState(SR_RasterizerState* /*aRasterizerState*/)
	{
	}

	void SR_GraphicsContext_Vk::SetViewport(const SR_Viewport& /*aViewport*/)
	{
	}

	void SR_GraphicsContext_Vk::SetScissorRect(const SR_ScissorRect& /*aScissorRect*/)
	{
	}

	void SR_GraphicsContext_Vk::SetTopology(const SR_Topology& /*aTopology*/)
	{
	}

	void SR_GraphicsContext_Vk::SetVertexBuffer(uint /*aStartVertex*/, SR_Buffer* /*aBuffer*/)
	{
	}

	void SR_GraphicsContext_Vk::SetVertexBuffer(uint /*aStartVertex*/, SR_Buffer** /*aBuffer*/, uint /*aCount*/)
	{

	}

	void SR_GraphicsContext_Vk::SetVertexBuffer(uint /*aStartVertex*/, SR_Buffer* /*aVertexBuffer*/, uint /*aVertexBufferOffset*/, SR_Buffer* /*aInstanceBuffer*/, uint /*aInstanceBufferOffset*/)
	{

	}

	void SR_GraphicsContext_Vk::SetIndexBuffer(SR_Buffer* /*aIndexBuffer*/)
	{

	}

	void SR_GraphicsContext_Vk::SetShaderState(SR_ShaderState* /*aShaderState*/)
	{

	}

	void SR_GraphicsContext_Vk::SetRenderTargets(SR_RenderTarget** /*aTargets*/, uint /*aNumTargets*/, SR_RenderTarget* /*aDepthStencil*/, uint /*aDepthAccess*/)
	{

	}

	void SR_GraphicsContext_Vk::CopyBuffer(SR_Buffer* /*aDestination*/, SR_Buffer* /*aSource*/)
	{

	}

	void SR_GraphicsContext_Vk::CopyTexture(SR_TextureBuffer* /*aDestination*/, SR_TextureBuffer* /*aSource*/)
	{

	}

	void SR_GraphicsContext_Vk::UpdateBufferData(uint /*aByteSize*/, void* /*aData*/, SR_Buffer* /*aOutBuffer*/)
	{

	}

	void SR_GraphicsContext_Vk::GenerateMips(SR_Texture& /*aTexture*/)
	{

	}

	void SR_GraphicsContext_Vk::ResolveQuery(const EQueryType& /*aType*/, uint /*aStartIndex*/, uint /*aNumQueries*/, SR_Buffer* /*aOutBuffer*/)
	{

	}

	void SR_GraphicsContext_Vk::EndQuery(const EQueryType& /*aType*/, uint /*aIndex*/)
	{

	}

	SR_CommandList* SR_GraphicsContext_Vk::GetCommandList()
	{

	}

	void SR_GraphicsContext_Vk::InsertFence(const SR_Fence& /*aFence*/)
	{

	}

}

#endif