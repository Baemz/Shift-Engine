#pragma once
#include <string>

namespace Shift
{
	class SC_EnginePaths
	{
	public:

		static const char* GetRelativeDataPath();
		static const char* GetRelativeDataPathDblSlash();
		static const char* GetShadersDirectory();
		static const char* GetDocumentsDirectory();
		static const wchar_t* GetRelativeDataPathW();
		static const wchar_t* GetDocumentsDirectoryW();
		static std::string GetExecutablePath();
		static std::string GetExecutableDirectory();
		static std::string GetWorkingDirectoryAbsolute();
		static std::wstring GetWorkingDirectoryAbsoluteW();

		static void SetWorkingDirectory(const char* aWorkingDir);
	};
}