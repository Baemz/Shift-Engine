#include "SRender_Precompiled.h"
#include "SR_Buffer_DX12.h"

#if ENABLE_DX12

namespace Shift
{
	SR_Buffer_DX12::SR_Buffer_DX12()
		: SR_Buffer()
	{
	}

	SR_Buffer_DX12::~SR_Buffer_DX12()
	{
	}

	void SR_Buffer_DX12::Invalidate()
	{
	}

	uint SR_GetBufferAlignment_DX12(const SR_BufferDesc& aBufferDesc)
	{
		uint alignment = 4;

		if (aBufferDesc.myBindFlag & SR_BindFlag_ConstantBuffer)
			alignment = 256;

		return alignment;
	}
}

#endif