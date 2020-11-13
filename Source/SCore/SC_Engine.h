#pragma once
#include <atomic>
#include <functional>
#include "SC_ModuleManager.h"
#include "SG_GraphicsModule.h"
//#include "PhysicsEngine.h"
#include "SR_GraphicsEngineEnums.h"
#include "SC_Filewatcher.h"

namespace Shift
{
	using AppInitCallback = std::function<bool()>;
	using AppUpdateCallback = std::function<void()>;
	using AppExitCallback = std::function<void()>;
	using ToolsRenderCallback = std::function<void()>;

	struct SC_EngineStartParams
	{
		SC_Vector2f myResolution = { 1280.f, 720.f };
		bool myIsFullscreen = false;
		const char* applicationName = "Shift Engine 2";
		AppInitCallback initCallback = nullptr;
		AppInitCallback postInitCallback = nullptr;
		AppUpdateCallback updateCallback = nullptr;
		AppExitCallback exitCallback = nullptr;
		ToolsRenderCallback tools_renderCallback = nullptr;
	};

	class CEntityManager;
	class SC_Window;

	// SR_0
	class SC_InputManager;

	class SC_Engine
	{
	public:
		SC_Engine();
		~SC_Engine();

		void Start(const SC_EngineStartParams& aStartParams);

		void RegisterModule(SC_Module* aModule);
		void UnregisterModule(SC_Module* aModule);

		SC_Window* GetWindow() const { return myWindow; }

		// SR_0
		SC_InputManager* GetInputManager() const { return myInputManager; }

	private:
		void RunMainLoop();
		void RunLogLoop();

	private:
		friend class SC_EngineInterface;

		SC_ModuleManager myModuleManager;
		SG_GraphicsModule myGraphicsModule;
		//CPhysicsEngine myPhysicsEngine;
		SC_Filewatcher myFilewatcher;
		SC_EngineStartParams myStartParams;
		SC_Window* myWindow; // Should probably not own the window since the engine isn't dependent on a window.
		AppInitCallback myInitCallback;
		AppUpdateCallback myUpdateCallback;
		AppExitCallback myExitCallback;
		ToolsRenderCallback myToolsRenderCallback;
		std::atomic_bool myIsRunning;
		volatile bool myIsLogging;

		// SR_0
		SC_InputManager* myInputManager;
	};

	void StartEngine(Shift::SC_Engine& aEngine, const Shift::SC_EngineStartParams& aStartParams);
}

