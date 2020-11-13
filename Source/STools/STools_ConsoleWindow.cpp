#include "STools_Precompiled.h"
#include "STools_ConsoleWindow.h"
#include "STools_LogWindow.h"

#include "SC_Console.h"

#include "SR_GraphicsDevice.h"

namespace Shift
{
	STools_ConsoleWindow::STools_ConsoleWindow(STools_LogWindow* aLogWindow)
		: myLogWindow(aLogWindow)
		, myNeedsRefresh(true)
	{
	}

	STools_ConsoleWindow::~STools_ConsoleWindow()
	{
	}

	void STools_ConsoleWindow::PreRender()
	{
		SR_GraphicsDevice* device = SR_GraphicsDevice::GetDevice();
		SC_Vector2f resolution = device->GetResolution();
		ImGui::SetNextWindowSize(ImVec2(resolution.x, 700), ImGuiCond_Appearing);
		ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Appearing);
	}

	void STools_ConsoleWindow::Render()
	{
		SR_GraphicsDevice* device = SR_GraphicsDevice::GetDevice();
		SC_Vector2f resolution = device->GetResolution();
		ImGui::BeginChild("Scrolling", { resolution.x - 40, 630 }, true, ImGuiWindowFlags_ChildWindow);
		for (const SC_Pair<SC_LogType, std::string>& logText : myLogWindow->GetSavedMessages())
		{
			ImVec4 color(1.0f, 1.0f, 1.0f, 1.0f);
			if (logText.myFirst == SC_LogType::Error)
				color = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
			else if (logText.myFirst == SC_LogType::Warning)
				color = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
			else if (logText.myFirst == SC_LogType::Success)
				color = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);

			ImGui::TextColored(color, logText.mySecond.c_str());
		}
		if (myNeedsRefresh)
		{
			ImGui::SetScrollHereY(1.0);
			myNeedsRefresh = false;
		}

		ImGui::EndChild();

		std::string locCommandStr = "";
		ImGui::SetKeyboardFocusHere();
		if (ImGui::InputText("", myInput, ourMaxInputLength, ImGuiInputTextFlags_EnterReturnsTrue))
		{
			locCommandStr = std::string(myInput);
			SC_COMMAND_LOG("%s", locCommandStr.c_str());

			myConsole->ExecuteCommand(locCommandStr.c_str());

			locCommandStr.clear();
			SC_Fill(myInput, ourMaxInputLength, '\0');
			myNeedsRefresh = true;

		}
	}
}