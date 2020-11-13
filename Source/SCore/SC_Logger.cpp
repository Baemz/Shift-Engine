#include "SCore_Precompiled.h"
#include "SC_Logger.h"
#include <chrono>
#include <sstream>
#include <fstream>

#if USE_LOGGING
namespace Shift
{
	SC_Logger* SC_Logger::ourInstance = nullptr;

	SC_Logger::~SC_Logger()
	{
		if (myOutFile.good())
		{
			myOutFile.close();
		}
	}

	void SC_Logger::Flush()
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1));

		myCanQueueMsg = false;
		while (!myQueue.empty() && !myIdle) {};
		myCanQueueMsg = true;
	}

	void SC_Logger::Create()
	{
		if (!ourInstance)
			ourInstance = new SC_Logger();
	}

	void SC_Logger::Destroy()
	{
		if (ourInstance)
			delete ourInstance;
	}

	void SC_Logger::Exit()
	{
		ourInstance->myQueue.invalidate();
	}

	void SC_Logger::LogMessage(const std::string& aMsg, const SC_LogType& aType, const char* aFunc)
	{
		while(!ourInstance->myCanQueueMsg) {}

		SC_LogMessage msg;
		msg.myMessage = aMsg;
		msg.myFunction = aFunc;
		msg.myType = aType;
		msg.myTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
		ourInstance->myQueue.push(msg);
	}

	SC_Logger::SC_Logger()
		: myIdle(true)
		, myCanQueueMsg(true)
	{
	}

	void SC_Logger::WaitPop()
	{
		SC_GrowingArray<SC_LogMessage> msgs;
		if (myQueue.WaitPopAll(msgs))
		{
			myIdle = false;
			TryOpenFile();
			for (auto& msg : msgs)
			{
				std::string prefix;
				switch (msg.myType)
				{
				case SC_LogType::Warning:
					prefix = "WARNING";
					break;
				case SC_LogType::Error:
					prefix = "ERROR";
					break;
				case SC_LogType::Success:
					prefix = "SUCCESS";
					break;
				case SC_LogType::Command:
					prefix = "COMMAND";
					break;
				default:
					prefix = "LOG";
					break;
				}
				std::stringstream timeStr;
				timeStr << std::put_time(std::localtime(&msg.myTime), "%Y-%m-%d %X");
				std::stringstream sstr;
				sstr << "[" << timeStr.str() << "][" << prefix << "] - " << msg.myMessage << std::endl;

#if USE_CONSOLE

				WORD color = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
				switch (msg.myType)
				{
				case SC_LogType::Warning:
					color = FOREGROUND_GREEN | FOREGROUND_RED;
					break;
				case SC_LogType::Error:
					color = FOREGROUND_RED;
					break;
				case SC_LogType::Success:
					color = FOREGROUND_GREEN;
					break;
				default:
				case SC_LogType::Standard:
					color = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
					break;
				}

				SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
				std::cout << sstr.str();
#endif
#if IS_VISUAL_STUDIO
				OutputDebugStringA(sstr.str().c_str());
#endif
				if (myOutFile.is_open())
				{
					myOutFile << sstr.str();
					myOutFile.flush();
				}
			}
			myIdle = true;
		}
	}

	void SC_Logger::TryOpenFile()
	{
		if (myOutFile.is_open())
			return;

		std::string outFileName(GetFilename(SC_EnginePaths::GetExecutablePath()) + ".log");
		myOutFile.open(outFileName);

		if (!myOutFile.is_open())
		{
			SC_ERROR_LOG("Couldn't open log file.");
			myOutFile.close();
		}
	}
}
#endif