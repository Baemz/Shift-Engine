#pragma once
#include "SR_TempResourcePool.h"

#if ENABLE_DX12

namespace Shift
{
	class SR_Texture;
	class SR_TextureBuffer;
	class SR_TempResourcePool_DX12 : public SR_TempResourcePool
	{
	public:
		SR_TempResourcePool_DX12();
		~SR_TempResourcePool_DX12();

		SC_Ref<SR_TextureBuffer> CreateTextureBuffer(const SR_TextureBufferDesc& aDesc, const char* aID = nullptr) override;
		SC_Ref<SR_Texture> CreateTexture(const SR_TextureBuffer* aTextureBuffer, const SR_TextureDesc& aDesc) override;

	private:
	};
}

#endif