#include "SRender_Precompiled.h"
#include "SR_ImGUI.h"

#if IS_WINDOWS
#	include "imgui_impl_win32.h"
#	include "SC_Window_Win64.h"
#endif

#if ENABLE_DX12
#	include "imgui_impl_dx12.h"
#	include "SR_GraphicsDevice_DX12.h"
#	include "SR_GraphicsContext_DX12.h"
#	include "SR_SwapChain_DX12.h"
#endif
#if ENABLE_VULKAN
#	include "imgui_impl_vulkan.h"
#	include "SR_GraphicsDevice_Vk.h"
#	include "SR_GraphicsContext_Vk.h"
#endif

#include "SR_SwapChain.h"
#include "SR_RenderInterface.h"

namespace Shift
{
	bool SR_ImGUI::myIsActive = false;
	bool SR_ImGUI::Init(const SC_Window* aWindow, SR_GraphicsDevice* aDevice)
	{
		IMGUI_CHECKVERSION();
		ImGui::SetCurrentContext(ImGui::CreateContext());
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

		switch (aDevice->APIType())
		{
#if ENABLE_DX12
		case SR_GraphicsAPI::DirectX12:
		{
			const SC_Window_Win64* window = static_cast<const SC_Window_Win64*>(aWindow);

			if (!ImGui_ImplWin32_Init(window->GetHandle()))
				return false;

			SR_GraphicsDevice_DX12* device = static_cast<SR_GraphicsDevice_DX12*>(aDevice);
			if (!ImGui_ImplDX12_Init(device->GetNativeDevice(), SR_SwapChain::GetBackbufferCount(), DXGI_FORMAT_R8G8B8A8_UNORM))
				return false;

			ImGui_ImplDX12_NewFrame();
			break;
		}
#endif
#if ENABLE_VULKAN
		case SR_GraphicsAPI::Vulkan:
		{

#if IS_WINDOWS
			const SC_Window_Win64* window = static_cast<const SC_Window_Win64*>(aWindow);

			if (!ImGui_ImplWin32_Init(window->GetHandle()))
				return false;

#elif IS_LINUX
			const SC_Window_Xlib* window = static_cast<const SC_Window_Xlib*>(aWindow);

			if (!ImGui_ImplXlib_Init(window->GetHandle()))
				return false;
#endif
			SR_GraphicsDevice_Vk* device = static_cast<SR_GraphicsDevice_Vk*>(aDevice);
			ImGui_ImplVulkan_InitInfo initInfo = {};
			initInfo.Device = device->GetNativeDevice();
			initInfo.PhysicalDevice = device->GetNativePhysicalDevice();
			initInfo.Instance = device->GetVkInstance();
			VkRenderPass renderPass = nullptr;

			if (!ImGui_ImplVulkan_Init(&initInfo, renderPass))
				return false;

			ImGui_ImplVulkan_NewFrame();
			break;
		}
#endif
		default:
		{
			assert(!"not implemented");
			break;
		}
		}

		SetStyleGUI();
		myIsActive = true;
		return true;
	}

	void SR_ImGUI::NewFrame()
	{
#if IS_WINDOWS
		ImGui_ImplWin32_NewFrame();
#endif
		ImGui::NewFrame();
	}

	void SR_ImGUI::Render()
	{
		SR_GraphicsContext* ctx = SR_GraphicsContext::GetCurrent();
		SR_GraphicsDevice* device = SR_GraphicsDevice::GetDevice();

		SR_PROFILER_FUNCTION_TAGGED("ImGui Render");

		SR_RenderTarget* backbuffer = device->GetSwapChain()->GetCurrentRenderTarget();
		ctx->Transition(SR_ResourceState_RenderTarget, backbuffer->GetTextureBuffer());
		ctx->SetRenderTargets(&backbuffer, 1, nullptr, 0);

		ImGui::Render();

		switch (SR_GraphicsDevice::GetDevice()->APIType())
		{
#if ENABLE_DX12
		case SR_GraphicsAPI::DirectX12:
		{
			SR_GraphicsContext_DX12* ctxDX12 = static_cast<SR_GraphicsContext_DX12*>(ctx);
			ctxDX12->SetViewport(SR_Viewport());

			ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), ctxDX12->GetNativeCommandList());
			break;
		}
#endif

#if ENABLE_VULKAN
		case SR_GraphicsAPI::Vulkan:
		{
			SR_GraphicsContext_Vk* ctxVk = static_cast<SR_GraphicsContext_Vk*>(ctx);
			ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), ctxVk->GetNativeCommandBuffer());
			break;
		}
#endif
		}
	}

	void SR_ImGUI::Shutdown()
	{
		if (!myIsActive)
			return;

		switch (SR_GraphicsDevice::GetDevice()->APIType())
		{
#if ENABLE_DX12
		case SR_GraphicsAPI::DirectX12:
			ImGui_ImplDX12_Shutdown();
			ImGui_ImplWin32_Shutdown();
			break;
#endif
#if ENABLE_VULKAN
		case SR_GraphicsAPI::Vulkan:
#if IS_WINDOWS
			ImGui_ImplWin32_Shutdown();
#elif IS_LINUX
			ImGui_ImplXlib_Shutdown();
#endif

			ImGui_ImplVulkan_Shutdown();
			break;
#endif
		default:
			assert(!"not implemented");
			break;
		}
		ImGui::DestroyContext();
		myIsActive = false;
	}

	SC_Vector2f SR_ImGUI::GetCurrentWindowClientSize()
	{
		SC_Vector2f clientAreaSize = ImGui::GetWindowSize().GetAsEngineVector();
		clientAreaSize -= ImGui::GetStyle().FramePadding.GetAsEngineVector();
		clientAreaSize.y -= ImGui::GetFrameHeight();
		float borderReduction = ImGui::GetStyle().WindowBorderSize * 4;
		clientAreaSize.x -= borderReduction;
		clientAreaSize.y -= borderReduction;
		return clientAreaSize;
	}

	SC_Vector2f SR_ImGUI::GetCurrentWindowClientPos()
	{
		SC_Vector2f windowPos = ImGui::GetWindowPos().GetAsEngineVector();
		windowPos.x += ImGui::GetStyle().WindowBorderSize;
		windowPos.y += ImGui::GetFrameHeight() + ImGui::GetStyle().WindowBorderSize;
		return windowPos;
	}

	SC_Vector2f SR_ImGUI::GetCurrentRelativeCursorPos()
	{
		SC_Vector2f windowSize = GetCurrentWindowClientSize();
		SC_Vector2f windowPos = GetCurrentWindowClientPos();
		SC_Vector2f cursorPos = ImGui::GetMousePos().GetAsEngineVector();
		cursorPos = cursorPos - windowPos;
		cursorPos /= windowSize;
		cursorPos.x = SC_Clamp(cursorPos.x, 0.0f, 1.0f);
		cursorPos.y = SC_Clamp(cursorPos.y, 0.0f, 1.0f);
		return cursorPos;
	}

	void SR_ImGUI::SetStyleGUI()
	{
		ImGuiStyle& style = ImGui::GetStyle();
		ImVec4* colors = style.Colors;

		/// 0 = FLAT APPEARENCE
		/// 1 = MORE "3D" LOOK
		int is3D = 0;

		colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
		colors[ImGuiCol_TextDisabled] = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
		colors[ImGuiCol_ChildBg] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
		colors[ImGuiCol_WindowBg] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
		colors[ImGuiCol_PopupBg] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
		colors[ImGuiCol_Border] = ImVec4(0.12f, 0.12f, 0.12f, 0.71f);
		colors[ImGuiCol_BorderShadow] = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
		colors[ImGuiCol_FrameBg] = ImVec4(0.42f, 0.42f, 0.42f, 0.54f);
		colors[ImGuiCol_FrameBgHovered] = ImVec4(0.42f, 0.42f, 0.42f, 0.40f);
		colors[ImGuiCol_FrameBgActive] = ImVec4(0.56f, 0.56f, 0.56f, 0.67f);
		colors[ImGuiCol_TitleBg] = ImVec4(0.19f, 0.19f, 0.19f, 1.00f);
		colors[ImGuiCol_TitleBgActive] = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
		colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.17f, 0.17f, 0.17f, 0.90f);
		colors[ImGuiCol_MenuBarBg] = ImVec4(0.335f, 0.335f, 0.335f, 1.000f);
		colors[ImGuiCol_ScrollbarBg] = ImVec4(0.24f, 0.24f, 0.24f, 0.53f);
		colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.52f, 0.52f, 0.52f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.76f, 0.76f, 0.76f, 1.00f);
		colors[ImGuiCol_CheckMark] = ImVec4(0.65f, 0.65f, 0.65f, 1.00f);
		colors[ImGuiCol_SliderGrab] = ImVec4(0.52f, 0.52f, 0.52f, 1.00f);
		colors[ImGuiCol_SliderGrabActive] = ImVec4(0.64f, 0.64f, 0.64f, 1.00f);
		colors[ImGuiCol_Button] = ImVec4(0.54f, 0.54f, 0.54f, 0.35f);
		colors[ImGuiCol_ButtonHovered] = ImVec4(0.52f, 0.52f, 0.52f, 0.59f);
		colors[ImGuiCol_ButtonActive] = ImVec4(0.76f, 0.76f, 0.76f, 1.00f);
		colors[ImGuiCol_Header] = ImVec4(0.38f, 0.38f, 0.38f, 1.00f);
		colors[ImGuiCol_HeaderHovered] = ImVec4(0.47f, 0.47f, 0.47f, 1.00f);
		colors[ImGuiCol_HeaderActive] = ImVec4(0.76f, 0.76f, 0.76f, 0.77f);
		colors[ImGuiCol_Separator] = ImVec4(0.000f, 0.000f, 0.000f, 0.137f);
		colors[ImGuiCol_SeparatorHovered] = ImVec4(0.700f, 0.671f, 0.600f, 0.290f);
		colors[ImGuiCol_SeparatorActive] = ImVec4(0.702f, 0.671f, 0.600f, 0.674f);
		colors[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.59f, 0.98f, 0.25f);
		colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
		colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
		colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
		colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
		colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
		colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
		colors[ImGuiCol_TextSelectedBg] = ImVec4(0.73f, 0.73f, 0.73f, 0.35f);
		colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
		colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
		colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
		colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);

		style.PopupRounding = 3;

		style.WindowPadding = ImVec2(4, 4);
		style.FramePadding = ImVec2(6, 4);
		style.ItemSpacing = ImVec2(6, 2);

		style.ScrollbarSize = 18;

		style.WindowBorderSize = 1;
		style.ChildBorderSize = 1;
		style.PopupBorderSize = 1;
		style.FrameBorderSize = float(is3D);

		style.WindowRounding = 3;
		style.ChildRounding = 3;
		style.FrameRounding = 3;
		style.ScrollbarRounding = 2;
		style.GrabRounding = 3;

#ifdef IMGUI_HAS_DOCK 
		style.TabBorderSize = (float)is3D;
		style.TabRounding = 3;

		colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.38f, 0.38f, 0.38f, 1.00f);
		colors[ImGuiCol_Tab] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
		colors[ImGuiCol_TabHovered] = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
		colors[ImGuiCol_TabActive] = ImVec4(0.33f, 0.33f, 0.33f, 1.00f);
		colors[ImGuiCol_TabUnfocused] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
		colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.33f, 0.33f, 0.33f, 1.00f);
		colors[ImGuiCol_DockingPreview] = ImVec4(0.85f, 0.85f, 0.85f, 0.28f);

		if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}
#endif
	}
}
