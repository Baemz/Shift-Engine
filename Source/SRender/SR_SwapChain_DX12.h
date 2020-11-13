#pragma once
#include "SR_SwapChain.h"

#if ENABLE_DX12
struct IDXGISwapChain;
struct IDXGISwapChain4;
namespace Shift
{
	class SR_SwapChain_DX12 final : public SR_SwapChain 
	{
	public:
		SR_SwapChain_DX12();
		virtual ~SR_SwapChain_DX12();

		bool Init(const SR_SwapChainDesc& aDesc) override;
		void Present(bool aSync = false) override;
	private:
		IDXGISwapChain* mySwapchain;
		IDXGISwapChain4* mySwapchain4;
		bool myAllowTearing;
	};
}
#endif