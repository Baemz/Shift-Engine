#pragma once

#define IMGUI_DEFINE_MATH_OPERATORS
#pragma warning( disable : 26451)
#pragma warning( disable : 26439)
#pragma warning( disable : 6011)
#pragma warning( disable : 6031)
#pragma warning( disable : 6385)

#include "imgui.h"
#include "imgui_internal.h"

namespace Shift
{
	class SC_Window;
	class SR_GraphicsDevice;
	class SR_SwapChain;
	class SR_Texture;
	class SR_ImGUI
	{
	public:
		static bool Init(const SC_Window* aWindow, SR_GraphicsDevice* aDevice);
		static void NewFrame();
		static void Render();
		static void Shutdown();
		static SC_Vector2f GetCurrentWindowClientSize();
		static SC_Vector2f GetCurrentWindowClientPos();
		static SC_Vector2f GetCurrentRelativeCursorPos(); // Gets the normalized cursor pos [0 .. 1] inside of the current window

	private:
		static void SetStyleGUI();
		static bool myIsActive;
	};
}
