#pragma once
#include "SR_GraphicsResource.h"
#include "SR_TextureBuffer.h"
		  

namespace Shift
{
	struct SR_RenderTargetDesc
	{
		SR_Format myFormat;
		uint myArrayIndex;
		uint myArraySize;
		uint myMipLevel;

		SR_RenderTargetDesc() : myArraySize(1), myArrayIndex(0), myMipLevel(0) {}
	};

	class SR_RenderTarget : public SR_Resource
	{
		friend class SR_GraphicsDevice_DX12;
		friend class SR_GraphicsDevice_Vk;
	public:
		SR_RenderTarget();
		virtual ~SR_RenderTarget();

		SR_TextureBuffer* GetTextureBuffer();
		const SR_TextureBuffer* GetTextureBuffer() const;

	private:
		SC_Ref<SR_TextureBuffer> myTextureBuffer;
	};

}
