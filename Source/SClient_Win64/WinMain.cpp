#include "SClient_Win64_Precompiled.h"

#define SC_HEAVY_MEMORY_DEBUG (0)

#if SC_HEAVY_MEMORY_DEBUG && IS_DEBUG_BUILD
#include <vld.h>
#endif

#include <string>

#define SC_OVERLOAD_MEMORY_OPERATORS 1
#include "SC_MemoryOperators.h"

#include "SC_Engine.h"
#include "SC_Config.h"
#include "SClient_Base.h"
#include "STools_Editor.h"

#include "resource.h"

#if IS_WINDOWS
#include "SC_Window_Win64.h"

#if !USE_CONSOLE
#	pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:wmainCRTStartup")
#endif

const bool CreateStartParams(Shift::SC_EngineStartParams& aStartParamsToFill)
{
	aStartParamsToFill.applicationName = "Shift Engine Sandbox";

	if (Shift::SC_CommandLineManager::HasCommand(L"width"))
	{
		auto& args = Shift::SC_CommandLineManager::GetArguments(L"width");
		if (args.empty() == false)
		{
			aStartParamsToFill.myResolution.x = static_cast<float>(std::stoi(args[0]));
		}
		else
		{
			aStartParamsToFill.myResolution.x = static_cast<float>(GetSystemMetrics(SM_CXSCREEN)); // Windows Specific
		}
	}
	else
	{
		aStartParamsToFill.myResolution.x = Shift::SC_Config::GetWindowWidth();
	}

	if (Shift::SC_CommandLineManager::HasCommand(L"height"))
	{
		auto& args = Shift::SC_CommandLineManager::GetArguments(L"height");
		if (args.empty() == false)
		{
			aStartParamsToFill.myResolution.y = static_cast<float>(std::stoi(args[0]));
		}
		else
		{
			aStartParamsToFill.myResolution.y = static_cast<float>(GetSystemMetrics(SM_CXSCREEN)); // Windows Specific
		}
	}
	else
	{
		aStartParamsToFill.myResolution.y = Shift::SC_Config::GetWindowHeight();
	}

	if (Shift::SC_CommandLineManager::HasCommand(L"windowed"))
	{
		aStartParamsToFill.myIsFullscreen = false;
	}
	else
	{
		aStartParamsToFill.myIsFullscreen = Shift::SC_Config::IsFullscreen();
	}

	if (Shift::SC_CommandLineManager::HasCommand(L"appname"))
	{
		//auto& args = Shift::CCommandLineManager::GetArguments(L"appname");
		//aStartParamsToFill.applicationName = std::string(args[0].begin(), args[0].end()).c_str();
	}
	else
	{
		aStartParamsToFill.applicationName = Shift::SC_Config::GetAppName().c_str();
	}

	return true;
}



void InitApplication()
{
	std::string workingDir = Shift::SC_EnginePaths::GetExecutableDirectory() + std::string("/../Data");
	Shift::SC_EnginePaths::SetWorkingDirectory(workingDir.c_str());

	Shift::SC_CommandLineManager::Init();
	Shift::SC_Config::Load();

	Shift::SC_EngineStartParams startParams;
	Shift::SC_Engine engine;

	SClient_Base client; // Client is the actual application for client-users

	auto postInit = [&]()
	{
		HINSTANCE hInstance = GetModuleHandle(NULL);
		HICON hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));

		Shift::SC_Window_Win64* window = static_cast<Shift::SC_Window_Win64*>(engine.GetWindow());

		SendMessage(window->GetHandle(), WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
		SendMessage(window->GetHandle(), WM_SETICON, ICON_BIG, (LPARAM)hIcon);

		return true;
	};

	startParams.postInitCallback = postInit;

#if IS_EDITOR_BUILD
	Shift::STools_Editor editor; // Editor is our engine tool
	editor.AddClientHook(&client);
	startParams.initCallback = std::bind(&Shift::STools_Editor::Init, &editor);

	engine.RegisterModule(&editor);
#else
	startParams.initCallback = std::bind(&SClient_Base::Init, &client);

	engine.RegisterModule(&client);
#endif

	CreateStartParams(startParams);
	try
	{
		Shift::StartEngine(engine, startParams);
	}
	catch (...)
	{
		// Catch exceptions
	}
}

int APIENTRY wmain(HINSTANCE /*hInstance*/, HINSTANCE /*hPrevInstance*/, PWSTR /*lpCmdLine*/, int /*nCmdShow*/)
{
#if USE_CONSOLE
		AllocConsole();
		FILE* fp;
		freopen_s(&fp, "CONOUT$", "w+", stdout);
#endif

		InitApplication();

#if USE_CONSOLE
		FreeConsole();
#endif

	return 0;
}

#endif // IS_WINDOWS





