#include "SGraphics_Precompiled.h"
#include "SG_GraphicsModule.h"

#include "SR_QueueManager.h"

#if ENABLE_DX12
#	include "SC_Window_Win64.h"
#	include "SR_GraphicsDevice_DX12.h"
#	include "SR_SwapChain_DX12.h"
#endif
#if ENABLE_VULKAN
#	include "SR_GraphicsDevice_Vk.h"
//#	include "SR_SwapChain_Vk.h"

#	if IS_WINDOWS
#		include "SC_Window_Win64.h"
#	elif IS_LINUX 
#		include "SC_Window_Linux.h"
#	endif
#endif
#if ENABLE_PS4
#	include "SR_GraphicsDevice_PS4.h"
#endif

#include "SG_ViewManager.h"
#include "SG_CommonRenderStates.h"
#include "SG_LodPresets.h"


namespace Shift 
{
	SG_GraphicsModule::SG_GraphicsModule()
	{
	}

	SG_GraphicsModule::~SG_GraphicsModule()
	{
		Destroy();
	}

	bool SG_GraphicsModule::Init()
	{
		SG_LodPresets::Create();
		SG_CommonRenderStates::Create();
		myViewManager = new SG_ViewManager();

		return true;
	}

	void SG_GraphicsModule::Destroy()
	{
		SG_CommonRenderStates::Destroy();
		SG_LodPresets::Destroy();
	}

	void SG_GraphicsModule::BeginFrame()
	{
		SR_GraphicsDevice::GetDevice()->BeginFrame();
		myRenderStats.Reset();
	}

	void SG_GraphicsModule::Update()
	{
		BeginFrame();
		myViewManager->Update();
		EndFrame();
	}

	void SG_GraphicsModule::EndFrame()
	{
	}

	SG_ViewManager* SG_GraphicsModule::GetViewManager()
	{
		return myViewManager;
	}

	const SG_RenderStats& SG_GraphicsModule::GetRenderStats() const
	{
		return myRenderStats;
	}
}