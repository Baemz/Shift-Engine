#pragma once
#include "SC_Module.h"

#include "SG_RenderStats.h"

namespace Shift
{
	class SC_Window;
	class SR_SwapChain;

	class SG_GraphicsModule : public SC_Module
	{
		SC_MODULE_DECL(SG_GraphicsModule);

	public:
		SG_GraphicsModule();
		~SG_GraphicsModule();

		bool Init();
		void Destroy();

		void Update() override;

		SG_ViewManager* GetViewManager();

		const SG_RenderStats& GetRenderStats() const;
	private:
		void BeginFrame();
		void EndFrame();

		SG_RenderStats myRenderStats;
		SC_UniquePtr<SG_ViewManager> myViewManager;
	};
}
