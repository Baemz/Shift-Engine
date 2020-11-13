#include "SRender_Precompiled.h"
#include "SR_TextureBuffer.h"

namespace Shift
{
	SR_TextureBuffer::SR_TextureBuffer()
	{
	}
	SR_TextureBuffer::~SR_TextureBuffer()
	{
	}
	const SR_TextureBufferDesc& SR_TextureBuffer::GetProperties() const
	{
		return myProperties;
	}
}