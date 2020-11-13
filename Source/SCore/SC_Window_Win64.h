#pragma once
#include "SC_Window.h"

#if IS_WINDOWS
namespace Shift
{
	class SC_Window_Win64 : public SC_Window
	{
		friend class SC_Window;
	public:
		virtual ~SC_Window_Win64();

		bool Init() override final;
		bool HandleMessages(class SC_InputManager* aInputManager) override final;

		HWND GetHandle() const;

	private:
		SC_Window_Win64();
		HWND myWindowHandle;
	};
}

#endif