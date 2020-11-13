#include "SCore_Precompiled.h"
#include "SC_Window.h"

#if IS_WINDOWS
#	include "SC_Window_Win64.h"
#elif IS_LINUX
#	include "SC_Window_Linux.h"
#endif

namespace Shift
{
	SC_Window::SC_Window()
		: myIsFullscreen(false)
	{
	}
	SC_Window* SC_Window::Create(const char* aAppName, const SC_Vector2f& aResolution)
	{
		SC_Window* win = nullptr;

#if IS_WINDOWS
		win = new SC_Window_Win64();
#elif IS_LINUX
		win = new CWindow_Linux();
#endif
		win->myResolution = aResolution;
		win->myAppName = aAppName;
		if (!win->Init())
		{
			delete win;
			win = nullptr;
		}
		return win;
	}
	SC_Window::~SC_Window()
	{
	}
	const SC_Vector2f& SC_Window::GetResolution() const
	{
		return myResolution;
	}
	bool SC_Window::IsFullscreen() const
	{
		return myIsFullscreen;
	}
}