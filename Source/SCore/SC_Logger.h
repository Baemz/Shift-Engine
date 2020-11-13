#pragma once
#include "SC_ThreadTools.h"
#include "SC_StringHelpers.h"
#include <ctime> 

#if USE_LOGGING
#define SC_LOG(text, ...)			Shift::SC_Logger::GetInstance()->LogMessage(Shift::GetFormattedString(text, __VA_ARGS__), Shift::SC_LogType::Standard, __FUNCTION__)
#define SC_COMMAND_LOG(text, ...)	Shift::SC_Logger::GetInstance()->LogMessage(Shift::GetFormattedString(text, __VA_ARGS__), Shift::SC_LogType::Command, __FUNCTION__)
#define SC_SUCCESS_LOG(text, ...)	Shift::SC_Logger::GetInstance()->LogMessage(Shift::GetFormattedString(text, __VA_ARGS__), Shift::SC_LogType::Success, __FUNCTION__)
#define SC_ERROR_LOG(text, ...)		Shift::SC_Logger::GetInstance()->LogMessage(Shift::GetFormattedString(text, __VA_ARGS__), Shift::SC_LogType::Error, __FUNCTION__)
#define SC_WARNING_LOG(text, ...)	Shift::SC_Logger::GetInstance()->LogMessage(Shift::GetFormattedString(text, __VA_ARGS__), Shift::SC_LogType::Warning, __FUNCTION__)

#define Log(text, ...)				SC_LOG(text, __VA_ARGS__)
#define Init_Log(text, ...)			SC_SUCCESS_LOG(text, __VA_ARGS__)
#define Warning_Log(text, ...)		SC_WARNING_LOG(text, __VA_ARGS__)
//#define S_ERROR_LOG(text, ...)		S_ERROR_LOG(text, __VA_ARGS__)

namespace Shift
{
	enum class SC_LogType
	{
		Standard,
		Command,
		Success,
		Warning,
		Error
	};
	class SC_Logger
	{
		friend class SC_Engine;
	public:
		struct SC_LogMessage
		{
			std::string myMessage;
			std::string myFunction;
			std::time_t myTime;
			SC_LogType myType;
		};

		~SC_Logger();

		void Flush();

		static void Create();
		static void Destroy();
		static void Exit();

		static void LogMessage(const std::string& aMsg, const SC_LogType& aType, const char* aFunc = nullptr);

		static SC_Logger* GetInstance() { return ourInstance; }
	private:
		SC_Logger();

		void WaitPop();
		void TryOpenFile();

		SC_ThreadSafeQueue<SC_LogMessage> myQueue;
		static SC_Logger* ourInstance;

		std::ofstream myOutFile;
		volatile bool myIdle;
		volatile bool myCanQueueMsg;
	};

}

#else

#define SC_LOG(text, ...)			{}
#define SC_COMMAND_LOG(text, ...)	{}
#define SC_ERROR_LOG(text, ...)		{}
#define SC_WARNING_LOG(text, ...)	{}
#define SC_SUCCESS_LOG(text, ...)	{}

#endif