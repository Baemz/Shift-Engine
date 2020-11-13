#pragma once
#include "SR_TextureBuffer.h"

#if ENABLE_DX12

struct ID3D12Resource;
namespace Shift
{
	class SR_TextureBuffer_DX12 final : public SR_TextureBuffer
	{
		friend class SR_GraphicsDevice_DX12;
	public:
		SR_TextureBuffer_DX12();
		~SR_TextureBuffer_DX12();

	private:
	};
}

#endif