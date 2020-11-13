#include "SC_StringHelpers.h"

#pragma warning(push)
#pragma warning(disable : 4505) 
#pragma warning(disable : 4244) 

namespace Shift
{
	std::wstring ToWString(const char* aString)
	{
		std::string str(aString);
		std::wstring wstr(str.begin(), str.end());
		return wstr;
	}
	std::wstring ToWString(const std::string& aString)
	{
		std::wstring wstr(aString.begin(), aString.end());
		return wstr;
	}

	std::string ToString(const wchar_t* aWString)
	{
		std::wstring wstr(aWString);
		std::string str(wstr.begin(), wstr.end());
		return str;
	}
	std::string ToString(const std::wstring& aWString)
	{
		std::string str(aWString.begin(), aWString.end());
		return str;
	}

	std::string GetFileExtension(const std::string& aPath)
	{
		size_t idx = aPath.rfind('.');
		return std::string(aPath.begin() + (++idx), aPath.end());
	}

	std::string GetFileWithExtension(const std::string& aPath)
	{
		size_t idxNameStart = aPath.rfind('\\');
		if (idxNameStart == std::string::npos)
			idxNameStart = aPath.rfind('/');

		if (idxNameStart == std::string::npos)
			return std::string(aPath.begin(), aPath.end());
		else
			return std::string(aPath.begin() + (idxNameStart + 1), aPath.end());
	}

	std::string SC_GetFileDirectory(const char* aPath)
	{
		std::string p(aPath);

		size_t idxNameStart = p.rfind('\\');
		if (idxNameStart == std::string::npos)
			idxNameStart = p.rfind('/');

		if (idxNameStart != std::string::npos)
			p = std::string(p.begin(), p.begin() + idxNameStart);

		return p;
	}

	bool SC_Strcmp(const char* aValue0, const char* aValue1)
	{
		return strcmp(aValue0, aValue1) == 0;
	}

	std::string GetFilename(const std::string& aPath)
	{
		size_t idxExt = aPath.rfind('.');
		size_t idxNameStart = aPath.rfind('\\');
		if (idxNameStart == std::string::npos)
			idxNameStart = aPath.rfind('/');

		if (idxNameStart == std::string::npos)
			return aPath;

		return std::string(aPath.begin() + (++idxNameStart), aPath.begin() + (idxExt));
	}
}
#pragma warning(pop)