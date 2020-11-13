#pragma once
#include "SR_GraphicsDefinitions.h"
#include "SR_Texture.h"
#include "SR_RenderTarget.h"

namespace Shift
{
	class SC_Window;

	struct SR_SwapChainDesc 
	{
		SC_Window* myWindow;
		SR_Format myBackbufferFormat;
	};

	class SR_SwapChain
	{
	public:
		SR_SwapChain();
		virtual ~SR_SwapChain();

		virtual bool Init(const SR_SwapChainDesc& aDesc) = 0;
		virtual void Present(bool aSync = false) = 0;

		const SR_Texture* GetCurrentTarget() const;
		SR_Texture* GetCurrentTarget();

		const SR_RenderTarget* GetCurrentRenderTarget() const;
		SR_RenderTarget* GetCurrentRenderTarget();

		static uint GetBackbufferCount() { return ourNumBackbuffers; }
		uint GetCurrentBackbufferIndex() { return myCurrentBackbufferIndex; }

	protected:
		static constexpr uint ourNumBackbuffers = 3;
		SC_Ref<SR_Texture> myScreenTextures[ourNumBackbuffers];
		SC_Ref<SR_RenderTarget> myScreenTargets[ourNumBackbuffers];
		volatile uint myCurrentBackbufferIndex;
	};
}
