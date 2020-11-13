#include "SCore_Precompiled.h"
#include "SC_Window_Win64.h"
#include "SC_InputManager.h"
#if IS_WINDOWS
#include "imgui.h"
namespace Shift
{
	//////////
	//////////
	//////////
	//////////
	//////////
	//////////
	bool CaptureInput(MSG& aMessage)
	{
		ImGuiIO& io = ImGui::GetIO();
		switch (aMessage.message)
		{
		case WM_CLOSE:
		case WM_QUIT:
		case WM_DESTROY:
			return false;
		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_MBUTTONDOWN:
		{
			int button = 0;
			if (aMessage.message == WM_LBUTTONDOWN) button = 0;
			if (aMessage.message == WM_RBUTTONDOWN) button = 1;
			if (aMessage.message == WM_MBUTTONDOWN) button = 2;

			io.MouseDown[button] = true;
			break;
		}
		case WM_LBUTTONUP:
		case WM_RBUTTONUP:
		case WM_MBUTTONUP:
		{
			int button = 0;
			if (aMessage.message == WM_LBUTTONUP) button = 0;
			if (aMessage.message == WM_RBUTTONUP) button = 1;
			if (aMessage.message == WM_MBUTTONUP) button = 2;

			io.MouseDown[button] = false;
			break;
		}
		case WM_MOUSEWHEEL:
			io.MouseWheel += GET_WHEEL_DELTA_WPARAM(aMessage.wParam) > 0 ? +1.0f : -1.0f;
			break;
		case WM_MOUSEMOVE:
			io.MousePos.x = (signed short)(aMessage.lParam);
			io.MousePos.y = (signed short)(aMessage.lParam >> 16);
			break;
		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
			if (aMessage.wParam < 256)
				io.KeysDown[aMessage.wParam] = 1;
			break;
		case WM_KEYUP:
		case WM_SYSKEYUP:
			if (aMessage.wParam < 256)
				io.KeysDown[aMessage.wParam] = 0;
			break;
		case WM_CHAR:
			// You can also use ToAscii()+GetKeyboardState() to retrieve characters.
			if (aMessage.wParam > 0 && aMessage.wParam < 0x10000)
				io.AddInputCharacter((unsigned short)aMessage.wParam);
			break;
		}
		return true;
	}

	//////////
	//////////
	//////////
	//////////
	//////////

	LRESULT CALLBACK locWinProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		if (msg == WM_DESTROY || msg == WM_CLOSE)
		{
			PostQuitMessage(0);
			return 0;
		}
		return DefWindowProc(hWnd,
			msg,
			wParam,
			lParam);
	}

	SC_Window_Win64::SC_Window_Win64()
		: myWindowHandle(nullptr)
	{
	}

	bool SC_Window_Win64::Init()
	{
		if (myIsFullscreen)
		{
			HMONITOR hmon = MonitorFromWindow(myWindowHandle,
				MONITOR_DEFAULTTONEAREST);
			MONITORINFO info = { sizeof(info) };
			GetMonitorInfo(hmon, &info);

			myResolution.x = (float)info.rcMonitor.right - info.rcMonitor.left;
			myResolution.y = (float)info.rcMonitor.bottom - info.rcMonitor.top;
		}

		HINSTANCE hInstance = GetModuleHandle(NULL);

		std::wstring name(myAppName.begin(), myAppName.end());
		WNDCLASSEX wc;
		wc.cbSize = sizeof(WNDCLASSEX);
		wc.style = CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc = locWinProc;
		wc.cbClsExtra = NULL;
		wc.cbWndExtra = NULL;
		wc.hInstance = hInstance;
		wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 2);
		wc.lpszMenuName = NULL;
		wc.lpszClassName = name.c_str();
		wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

		if (!RegisterClassEx(&wc))
		{
			MessageBox(NULL, L"Error registering window-class", L"Error", MB_OK | MB_ICONERROR);
			return false;
		}

		myWindowHandle = CreateWindowEx(
			NULL,
			name.c_str(),
			name.c_str(),
			WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT, CW_USEDEFAULT,
			(int)myResolution.x, (int)myResolution.y,
			NULL,
			NULL,
			hInstance,
			NULL
		);

		if (!myWindowHandle)
		{
			MessageBox(NULL, L"Error creating window", L"Error", MB_OK | MB_ICONERROR);
			return false;
		}

		if (myIsFullscreen)
			SetWindowLong(myWindowHandle, GWL_STYLE, 0);

		ShowWindow(myWindowHandle, SW_SHOWMAXIMIZED);
		UpdateWindow(myWindowHandle);

		RECT windowRes;
		if (!GetClientRect(myWindowHandle, &windowRes))
		{
			SC_ERROR_LOG("Couldn't get client area from windw.");
		}

		myResolution.x = (float)windowRes.right;
		myResolution.y = (float)windowRes.bottom;

		return true;

	}

	bool SC_Window_Win64::HandleMessages(SC_InputManager* aInputManager)
	{
		// SR_0
		MSG msg;
		ZeroMemory(&msg, sizeof(MSG));
		while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
		{
			if (!CaptureInput(msg))
				return false;

			// SR_0 // this does not work right yet, will do next
			if (myWindowHandle == GetForegroundWindow())
			{
				aInputManager->HandleMessage(msg.message);
			}

			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}

		return true;
	}
	HWND SC_Window_Win64::GetHandle() const
	{
		return myWindowHandle;
	}
	SC_Window_Win64::~SC_Window_Win64()
	{
		DestroyWindow(myWindowHandle);
	}
}
#endif