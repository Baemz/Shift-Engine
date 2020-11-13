#include "SCore_Precompiled.h"
#include "SC_EnginePaths.h"

namespace Shift
{
	static constexpr const char* locRelativeDataPath = "../Data/";
	static constexpr const char* locRelativeDataPathDblSlash = "..\\Data\\";
	static constexpr const char* locShadersDirectory = "../Data/Shaders/";
	static constexpr const char* locDocumentsDirectory = "NOT IMPLEMENTED YET!";

	static constexpr const wchar_t* locRelativeDataPathW = L"../Data/";
	static constexpr const wchar_t* locDocumentsDirectoryW = L"NOT IMPLEMENTED YET!";

	const char* SC_EnginePaths::GetRelativeDataPath()
	{
		return locRelativeDataPath;
	}

	const char* SC_EnginePaths::GetRelativeDataPathDblSlash()
	{
		return locRelativeDataPathDblSlash;
	}

	const char* SC_EnginePaths::GetShadersDirectory()
	{
		return locShadersDirectory;
	}

	const char* SC_EnginePaths::GetDocumentsDirectory()
	{
		return locDocumentsDirectory;
	}
	const wchar_t* SC_EnginePaths::GetRelativeDataPathW()
	{
		return locRelativeDataPathW;
	}

	const wchar_t* SC_EnginePaths::GetDocumentsDirectoryW()
	{
		return locDocumentsDirectoryW;
	}
	std::string SC_EnginePaths::GetExecutablePath()
	{
		std::string outName;

#if IS_WINDOWS
		CHAR exeFileName[MAX_PATH];
		GetModuleFileNameA(NULL, exeFileName, MAX_PATH);
		outName = exeFileName;
#else
#error NOT IMPLEMENTED YET!
#endif

		return outName;
	}

	std::string SC_EnginePaths::GetExecutableDirectory()
	{
		std::string outName = GetExecutablePath();

		outName = outName.substr(0, outName.rfind(GetFileWithExtension(outName)));

		return outName;
	}

	std::string SC_EnginePaths::GetWorkingDirectoryAbsolute()
	{
		std::string outName;

#if IS_WINDOWS
		CHAR currDir[MAX_PATH];
		::GetCurrentDirectoryA(MAX_PATH, currDir);
		outName = currDir;
#else
#error NOT IMPLEMENTED YET!
#endif
		std::replace(outName.begin(), outName.end(), '\\', '/');
		return outName;
	}

	std::wstring SC_EnginePaths::GetWorkingDirectoryAbsoluteW()
	{
		std::wstring outName;

#if IS_WINDOWS
		WCHAR currDir[MAX_PATH];
		::GetCurrentDirectoryW(MAX_PATH, currDir);
		outName = currDir;
#else
#error NOT IMPLEMENTED YET!
#endif
		std::replace(outName.begin(), outName.end(), '\\', '/');
		return outName;
	}

	void SC_EnginePaths::SetWorkingDirectory(const char* aWorkingDir)
	{
		std::string s(aWorkingDir);
		std::replace(s.begin(), s.end(), '\\', '/');

#if IS_WINDOWS
		SetCurrentDirectoryA(s.c_str());
#elif IS_LINUX
		chdir(s.c_str());
#else
#error NOT IMPLEMENTED YET!
#endif
	}
}