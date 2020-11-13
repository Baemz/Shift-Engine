#include "SRender_Precompiled.h"
#include "SR_TempResourcePool_DX12.h"

#if ENABLE_DX12
namespace Shift
{
	SR_TempResourcePool_DX12::SR_TempResourcePool_DX12()
	{
	}
	SR_TempResourcePool_DX12::~SR_TempResourcePool_DX12()
	{
	}
	SC_Ref<SR_TextureBuffer> SR_TempResourcePool_DX12::CreateTextureBuffer(const SR_TextureBufferDesc& aDesc, const char* aID)
	{
		SC_UNUSED(aDesc);
		SC_UNUSED(aID);
		return SC_Ref<SR_TextureBuffer>();
	}
	SC_Ref<SR_Texture> SR_TempResourcePool_DX12::CreateTexture(const SR_TextureBuffer* aTextureBuffer, const SR_TextureDesc& aDesc)
	{
		SC_UNUSED(aTextureBuffer);
		SC_UNUSED(aDesc);
		return SC_Ref<SR_Texture>();
	}
}
#endif