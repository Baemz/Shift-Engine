#pragma once
#include <vector>

namespace Shift
{
	class SC_CommandLineManager
	{
	public:
		static void Init();
		static bool HasCommand(const wchar_t* aParam);
		static bool HasArgument(const wchar_t* aParam, const wchar_t* aArg);

		static bool HasCommand(const char* aParam);
		static bool HasArgument(const char* aParam, const char* aArg);

		static const std::vector<std::wstring>& GetArguments(const wchar_t * aCommand);
	};

}