#include "STools_Precompiled.h"
#include "STools_Editor.h"

#include "imgui_node_editor.h"

#include "STools_EditorSubModule.h"
#include "STools_RegisterWindows.h"

#include "STools_AssetRepository.h"
#include "STools_LogWindow.h"
#include "STools_DynamicSky.h"
#include "STools_GraphicsWorld.h"
#include "STools_AssetImporter.h"
#include "STools_ScreenToWorld.h"
#include "STools_LevelEditor.h"
#include "STools_MaterialEditor.h"
#include "STools_GraphicsSettings.h"
#include "STools_Statistics.h"
#include "STools_ConsoleWindow.h"

#include "SClient_Base.h"

#include "SG_View.h"
#include "SG_Camera.h"

#include "SR_GraphicsDevice.h"

#include "SG_World.h"
#include "SG_Raytracer.h"

#include "SC_Profiler.h"
#include "SC_InputManager.h"

namespace ed = ax::NodeEditor;

static ed::EditorContext* g_Context = nullptr;

namespace Shift
{
	STools_Editor* STools_Editor::ourInstance = nullptr;

	STools_Editor::STools_Editor()
		: myTargetClientHook(nullptr)
		, myIsConsoleOpen(false)
	{
		if (ourInstance)
			assert(false && "Only one Editor instance is allowed.");

		ourInstance = this;
		STools_RegisterWindows();
	}

	STools_Editor::~STools_Editor()
	{
		ourInstance = nullptr;
	}

	bool STools_Editor::Init()
	{
		bool result = true;
		if (myTargetClientHook)
			result = myTargetClientHook->Init();

		//myCamera = myTargetClientHook->GetCamera();

		//g_Context = ed::CreateEditor();
		mySubModules.Add(new STools_AssetRepository());
		mySubModules.Add(new STools_Statistics());
		mySubModules.Add(new STools_DynamicSky(myTargetClientHook->GetGraphicsWorld()->mySky));
		mySubModules.Add(new STools_GraphicsWorld(myTargetClientHook->GetGraphicsWorld()));

		myLogWindow = new STools_LogWindow;
		myConsole = new STools_ConsoleWindow(myLogWindow);

		mySubModules.Add(myLogWindow);

		return result;
	}
	void STools_Editor::Update()
	{
		if (SC_InputManager::IsKeyDown(SC_Key_Insert))
		{
			if (!myIsConsoleOpen)
			{
				mySubModules.Add(myConsole);
				myIsConsoleOpen = true;
			}
			else if (myIsConsoleOpen)
			{
				mySubModules.RemoveCyclic(myConsole);
				myIsConsoleOpen = false;
			}
		}

		for (STools_EditorSubModule* tool : myRemovedTools)
			mySubModules.RemoveCyclic(tool);

		myRemovedTools.RemoveAll();

		for (auto& tool : mySubModules)
		{
			tool->UpdateModule();
		}

		//myGizmo.Interact();

		SC_PROFILER_FUNCTION();
		if (myTargetClientHook)
			myTargetClientHook->Update();
	}
	void STools_Editor::GameExitHook()
	{
	}

	void STools_Editor::Render()
	{
		SG_View* clientView = myTargetClientHook->GetView();
		
		SC_Vector2f resolution = SR_GraphicsDevice::GetDevice()->GetResolution();

		SC_Vector2f cursorPos;

		bool fullscreen = true;
		bool open = true;
		
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
		if (fullscreen)
		{
			ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(viewport->Pos);
			ImGui::SetNextWindowSize(viewport->Size);
			ImGui::SetNextWindowViewport(viewport->ID);
			window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
			window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
		}
		
		ImGui::Begin("DockSpace", &open, window_flags);
		ImGuiID dockspace_id = ImGui::GetID("DockSpaceBackground");
		ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), 0);
		
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Exit", "Alt+F4"))
					SC_EngineInterface::Exit();
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Window"))
			{
				for (auto& subModule : mySubModuleActivateFuncs)
				{
					if (ImGui::MenuItem(subModule.myName.c_str()))
						subModule.myActivateFunc();
				}
				ImGui::EndMenu();
			}

			// This should go into specific editors. Eg. level editor
			ImGui::Begin("Viewport");

			SR_Texture* viewportTexture = clientView->GetLastFinishedFrame();

			ImVec2 viewportSize = SR_ImGUI::GetCurrentWindowClientSize();
			ImGui::Image(viewportTexture, viewportSize);
			//myRelativeCursorPos = SR_ImGUI::GetCurrentRelativeCursorPos();

			ImGui::End(); // Viewport
			///////////////////////////////////////

			ImGui::EndMenuBar();
		}
		
		for (auto& tool : mySubModules)
		{
			tool->RenderModule();
		}
		
		ImGui::End(); // DockSpace
		
		ImGui::Begin("Profiling");
		ImGui::Text("FPS: %i (Delta: %.6f ms)", SC_Timer::GetFPS(), SC_Timer::GetDeltaTime() * 1000.f);
		uint64 memoryUsed = SC_Memory::GetMemoryUsage();
		ImGui::Text("Memory: %llu kB (%llu MB)", memoryUsed / KB, memoryUsed / MB);
		ImGui::Text("VRAM: %u MB", SR_GraphicsDevice::GetUsedVRAM());
		ImGui::Text("Viewport Cursor: x:%.3f y:%.3f", cursorPos.x, cursorPos.y);

		//SC_Vector3f camPos = clientView->GetCamera().GetPosition();
		//ImGui::Text("Camera Pos: x:%.3f y:%.3f z:%.3f", camPos.x, camPos.y, camPos.z);

		//SC_Vector3f camForward = clientView->GetCamera().GetForward();
		//ImGui::Text("Camera Forward: x:%.3f y:%.3f z:%.3f", camForward.x, camForward.y, camForward.z);
		
#if ENABLE_PROFILER
		auto& profilerData = SC_Profiler::Get().GetData();
		for (auto& section : profilerData)
		{
			ImGui::Text("%s", section.first.c_str());
		
			std::sort(section.second.begin(), section.second.end());
			for (auto& event : section.second)
			{
				ImGui::Text("\t%06.3f ms \t: %s", event.myTimeElapsedMS, event.myTag.c_str());
			}
			section.second.RemoveAll();
		}
#endif	
		
		ImGui::End(); // Profiling
		myLastViewportCursorPos = cursorPos;

		//myGizmo.Render();
	}

	void STools_Editor::AddClientHook(SClient_Base* aTargetClient)
	{
		myTargetClientHook = aTargetClient;
	}
	void STools_Editor::UnregisterTool(STools_EditorSubModule* aTool)
	{
		myRemovedTools.Add(aTool);
	}
}