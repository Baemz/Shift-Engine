#include "SCore_Precompiled.h"
#include "SC_Assert.h"
#include <sstream>

namespace Shift
{
	bool SC_Assert_Internal(const char* aFile, int aLine, const char* aMessage)
	{
		return SC_Assert_Internal(aFile, aLine, aMessage, nullptr);
	}

	bool SC_Assert_Internal(const char* aFile, int aLine, const char* aMessage, const char* aMessageFmtStr, ...)
	{
		SC_UniquePtr<char> formattedString = new char[2048];
		if (aMessageFmtStr)
		{
			va_list paramList;

			va_start(paramList, aMessageFmtStr);
			vsnprintf(formattedString.Get(), sizeof(char) * 2048, aMessageFmtStr, paramList);
			va_end(paramList);
		}

		std::stringstream outString;
		if (aMessageFmtStr)
			outString << "[" << aFile << "]: Line " << aLine << " - Assert Failed (" << aMessage << ") : " << formattedString << std::endl;
		else
			outString << "[" << aFile << "]: Line " << aLine << " - Assert Failed (" << aMessage << ")" << std::endl;
		SC_ERROR_LOG(outString.str().c_str());

#if USE_LOGGING
		SC_Logger::GetInstance()->Flush();
#endif

		return true;
	}
}
