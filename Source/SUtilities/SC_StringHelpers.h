#pragma once
#include <memory>
#include <string>
#include <stdexcept>

namespace Shift
{
	std::wstring ToWString(const char* aString);
	std::wstring ToWString(const std::string& aString);
	std::string ToString(const wchar_t* aWString);
	std::string ToString(const std::wstring& aWString);
	std::string GetFileExtension(const std::string& aPath);
	std::string GetFilename(const std::string& aPath);
	std::string GetFileWithExtension(const std::string& aPath);

	std::string SC_GetFileDirectory(const char* aPath);

	bool SC_Strcmp(const char* aValue0, const char* aValue1);

	template<typename ... Args>
	std::string GetFormattedString(const std::string& aFormat, Args ... aArgs)
	{
		size_t size = static_cast<size_t>(snprintf(nullptr, (size_t)0, aFormat.c_str(), aArgs ...) + (size_t)1);
		if (size <= 0) 
		{ 
			throw std::runtime_error("Error during formatting.");
		}

		std::unique_ptr<char[]> buf(new char[size]);
		snprintf(buf.get(), size, aFormat.c_str(), aArgs ...);
		return std::string(buf.get(), buf.get() + size - 1);
	}

}