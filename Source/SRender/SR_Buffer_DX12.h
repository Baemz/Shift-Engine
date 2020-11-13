#pragma once
#include "SR_Buffer.h"

#if ENABLE_DX12

struct ID3D12Resource;
namespace Shift
{
	class SR_Buffer_DX12 final : public SR_Buffer
	{
		friend class SR_GraphicsContext_DX12;
		friend class SR_GraphicsDevice_DX12;
	public:
		SR_Buffer_DX12();
		~SR_Buffer_DX12();

		void Invalidate() override;
	};

	uint SR_GetBufferAlignment_DX12(const SR_BufferDesc& aBufferDesc);
}

#endif