#include "STools_Precompiled.h"
#include "STools_LogWindow.h"
#include <sstream>

namespace Shift
{
	STools_LogWindow::STools_LogWindow()
	{
		//std::string inFileName(GetFilename(CEnginePaths::GetExecutableName()) + ".log");
		//auto onChange = [&](const std::string& aFile, const EFileEvent& aEvent)
		//{
		//	OnFileChanged(aFile, aEvent);
		//};
		//CEngineInterface::WatchFile(inFileName, onChange);
	}

	STools_LogWindow::~STools_LogWindow()
	{
		if (myLogFile.is_open())
			myLogFile.close();
	}

	void STools_LogWindow::Update()
	{
		STools_EditorSubModule::Update();

		mySavedLogMessages.RemoveAll();
		TryOpenLogFile();

		// TODO: only update when file changed.
		if (myLogFile.is_open())
		{
			myLogFile.clear();
			myLogFile.seekg(0);

			std::stringstream accumulatedStr;
			std::string logMsg;

			SC_LogType lastType = SC_LogType::Standard;
			uint count = 0;
			while (std::getline(myLogFile, logMsg))
			{
				if (logMsg.empty())
					continue;

				SC_LogType newType;
				if (logMsg.find("SUCCESS") != std::string::npos)
					newType = SC_LogType::Success;
				else if (logMsg.find("ERROR") != std::string::npos)
					newType = SC_LogType::Error;
				else if (logMsg.find("WARNING") != std::string::npos)
					newType = SC_LogType::Warning;
				else
					newType = SC_LogType::Standard;

				if (newType == lastType && count < 20)
				{
					accumulatedStr << "\n" << logMsg;
					++count;
				}
				else
				{
					if (!accumulatedStr.str().empty())
						mySavedLogMessages.Add(SC_Pair(lastType, accumulatedStr.str()));

					accumulatedStr.str("");
					accumulatedStr << logMsg;
					lastType = newType;
					count = 1;
				}
			}

			if (!accumulatedStr.str().empty())
				mySavedLogMessages.Add(SC_Pair(lastType, accumulatedStr.str()));
		}
	}

	void STools_LogWindow::Render()
	{
		for (SC_Pair<SC_LogType, std::string>& logText : mySavedLogMessages)
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
		ImGui::SetScrollHereY(1.0);

	}
	void STools_LogWindow::TryOpenLogFile()
	{
		std::string inFileName(GetFilename(SC_EnginePaths::GetExecutablePath()) + ".log");
		myLogFile.open(inFileName);
		if (!myLogFile.is_open())
		{
			SC_ERROR_LOG("Couldn't open log file.");
			myLogFile.close();
		}
	}
	void STools_LogWindow::OnFileChanged(const std::string& /*aFile*/, const EFileEvent&)
	{
		
	}
}