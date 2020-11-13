#include "SRender_Precompiled.h"
#include "SR_SwapChain.h"

namespace Shift
{
	SR_SwapChain::SR_SwapChain()
		: myCurrentBackbufferIndex(0)
	{
	}
	SR_SwapChain::~SR_SwapChain()
	{
	}
	const SR_Texture* SR_SwapChain::GetCurrentTarget() const
	{
		return myScreenTextures[myCurrentBackbufferIndex];
	}
	SR_Texture* SR_SwapChain::GetCurrentTarget()
	{
		return myScreenTextures[myCurrentBackbufferIndex];
	}
	const SR_RenderTarget* SR_SwapChain::GetCurrentRenderTarget() const
	{
		return myScreenTargets[myCurrentBackbufferIndex];
	}
	SR_RenderTarget* SR_SwapChain::GetCurrentRenderTarget()
	{
		return myScreenTargets[myCurrentBackbufferIndex];
	}
}