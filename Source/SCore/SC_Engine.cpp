#include "SCore_Precompiled.h"
#include "SC_Engine.h"

#include "SC_Window.h"
#include "SC_ThreadPool.h"
#include "SC_Config.h"

#include "SR_RenderInterface.h"
#include "SR_ImGUI.h"
#include "SR_GraphicsContext.h"
#include "SR_QueueManager.h"


#include "SP_PhysicsDevice.h"

//
#include "SC_InputManager.h"

namespace Shift 
{
	SC_Engine::SC_Engine()
		: myIsRunning(false)
		, myIsLogging(false)
	{
		if (SC_EngineInterface::ourEngine)
		{
			assert("Engine already created!");
		}
		SC_EngineInterface::ourEngine = this;
		SC_Memory::Setup();
		SC_ThreadTools::RegisterMainThread();
	}


	SC_Engine::~SC_Engine()
	{
		myGraphicsModule.Destroy();
		delete myWindow;
		SC_Memory::Cleanup();

	}

	void SC_Engine::Start(const SC_EngineStartParams& aStartParams)
	{
#if USE_LOGGING
		SC_Logger::Create();
		myIsLogging = true;
		SC_ThreadProperties loggerThreadProps;
		loggerThreadProps.myName = "Logger";
		loggerThreadProps.myAffinity = SC_ThreadTools::ourDefaultThreadAffinity;
		loggerThreadProps.myPriority = SC_ThreadPriority_Low;
		SC_Future<void> loggerThread = SC_CreateThread(loggerThreadProps, this, &SC_Engine::RunLogLoop);
#endif
		myStartParams = aStartParams;
		// init all engine sub-systems
		// Load modules here!
		//CMemoryHandler::Create();

		SC_ThreadPool::SetNumTaskThreads();

		myFilewatcher.Start();
		SP_PhysicsDevice::Create();

		SR_RenderInitParams initParams;
#if ENABLE_VULKAN
		if (SC_CommandLineManager::HasCommand(L"vulkan"))
		{
			initParams.myAPI = SR_GraphicsAPI::Vulkan;
		}
		else 
#endif
#if ENABLE_DX12
		if (SC_CommandLineManager::HasCommand(L"dx12"))
		{
			initParams.myAPI = SR_GraphicsAPI::DirectX12;
		}
		else
#endif
		{
			initParams.myAPI = SC_Config::GetGraphicsAPI();
		}


		//
		myWindow = SC_Window::Create(aStartParams.applicationName, aStartParams.myResolution);
		if (!myWindow)
			return;
		
		// SR_0
		myInputManager = new SC_InputManager;

		initParams.myWindow = myWindow;

		if (!SR_RenderInterface::Init(initParams))
		{
			SC_ERROR_LOG("Failed to initialize SRender");
			return;
		}

		if (!myGraphicsModule.Init())
			return;

		RegisterModule(&myGraphicsModule);

#if ENABLE_DEBUG_PRIMITIVES
		SG_DebugPrimitives::Create();
#endif

		myInitCallback = aStartParams.initCallback;
		myUpdateCallback = aStartParams.updateCallback;
		myExitCallback = aStartParams.exitCallback;

		if (myInitCallback)
			myIsRunning = myInitCallback();

		if (aStartParams.postInitCallback)
			aStartParams.postInitCallback();

		RunMainLoop();

		myFilewatcher.Stop();

		SC_ThreadPool::StopTaskThreads();
#if USE_LOGGING
		myIsLogging = false;
		SC_Logger::Exit();
		loggerThread.GetResult();
		SC_Logger::Destroy();
#endif
	}

	void SC_Engine::RunMainLoop()
	{
		//SP_PhysicsDevice* physics = SP_PhysicsDevice::GetInstance();
		while (myIsRunning)
		{
			SC_Timer::Tick(true);
			if (!myWindow->HandleMessages(myInputManager))
			{
				myIsRunning = false;
				break;
			}

			SR_RenderInterface::ourPresentEvent->myEventCPU.Wait();
			SR_RenderInterface::BeginFrame();
			SC_PROFILER_BEGIN_SECTION(SC_PROFILER_TAG_MODULE_UPDATE);
			// Update Modules
			myModuleManager.Update();

			//physics->Update(SC_Timer::GetDeltaTime());
			SC_PROFILER_END_SECTION();

			SR_ImGUI::NewFrame();

			{
				SC_PROFILER_BEGIN_SECTION(SC_PROFILER_TAG_MODULE_RENDER);
				SR_PROFILER_FUNCTION_TAGGED("Frame Render");
				// Render Modules
				myModuleManager.Render();
				SC_PROFILER_END_SECTION();

				SC_PROFILER_BEGIN_SECTION(SC_PROFILER_TAG_IMGUI_RENDER);
				SR_ImGUI::Render();
				SC_PROFILER_END_SECTION();
			}
			SR_RenderInterface::EndFrame();

			SR_RenderInterface::PostPresentTask();

#if ENABLE_DEBUG_PRIMITIVES
			SG_DebugPrimitives::Get()->Clear();
#endif
		}

#if ENABLE_DEBUG_PRIMITIVES
		SG_DebugPrimitives::Destroy();
#endif

		SR_RenderInterface::Destroy();
		SP_PhysicsDevice::Destroy();
	}

	void SC_Engine::RegisterModule(SC_Module* aModule)
	{
		myModuleManager.RegisterModule(aModule);
	}
	void SC_Engine::UnregisterModule(SC_Module* aModule)
	{
		myModuleManager.UnregisterModule(aModule);
	}

	void SC_Engine::RunLogLoop()
	{
		// TODO: Move the entire logging thread into SC_Logger itself.
#if USE_LOGGING
		while (myIsLogging)
		{
			SC_Logger::GetInstance()->WaitPop();
		}
#endif
	}

	void StartEngine(Shift::SC_Engine& aEngine, const Shift::SC_EngineStartParams& aStartParams)
	{
		aEngine.Start(aStartParams);
	}
}
