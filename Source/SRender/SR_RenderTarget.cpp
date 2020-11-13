#include "SRender_Precompiled.h"
#include "SR_RenderTarget.h"

namespace Shift
{
	SR_RenderTarget::SR_RenderTarget()
	{
	}

	SR_RenderTarget::~SR_RenderTarget()
	{
	}

	SR_TextureBuffer* SR_RenderTarget::GetTextureBuffer()
	{
		return myTextureBuffer;
	}

	const SR_TextureBuffer* SR_RenderTarget::GetTextureBuffer() const
	{
		return myTextureBuffer;
	}
}