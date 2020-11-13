#include "SCore_Precompiled.h"
#include "SC_CommandLineManager.h"
#include <unordered_map>
#include <string>


namespace Shift
{
	static std::map<std::wstring, std::vector<std::wstring>> myCommands;

	void SC_CommandLineManager::Init()
	{
		wchar_t** argv(__wargv);
		const int argc(__argc);

		std::wstring lastCommandEntered;

		for (int i = 0; i < argc; ++i)
		{
			std::wstring arg(argv[i]);
			if (!arg.empty())
			{
				if (arg.front() != '-')
				{
					myCommands[lastCommandEntered].push_back(arg);
				}
				else
				{
					if (myCommands.find(arg) == myCommands.end())
					{
						arg = std::wstring(arg.begin() + 1, arg.end());
						myCommands[arg];
						lastCommandEntered = arg;
					}
				}
			}
		}
	}

	bool SC_CommandLineManager::HasCommand(const wchar_t* aParam)
	{
		if (myCommands.find(aParam) != myCommands.end())
		{
			return true;
		}
		return false;
	}

	bool SC_CommandLineManager::HasArgument(const wchar_t* aParam, const wchar_t* aArg)
	{
		if (myCommands.find(aParam) != myCommands.end())
		{
			for (decltype(myCommands.begin()->second)::size_type index(0); index < myCommands[aParam].size(); ++index)
			{
				if (myCommands[aParam][index] == aArg)
				{
					return true;
				}
			}
		}
		return false;
	}

	bool SC_CommandLineManager::HasCommand(const char* aParam)
	{
		std::string cmd(aParam);
		std::wstring wCmd(cmd.begin(), cmd.end());
		return HasCommand(wCmd.c_str());
	}

	bool SC_CommandLineManager::HasArgument(const char* aParam, const char* /*aArg*/)
	{
		std::string cmd(aParam);
		std::wstring wCmd(cmd.begin(), cmd.end());

		std::string arg(aParam);
		std::wstring wArg(cmd.begin(), cmd.end());

		return HasArgument(wCmd.c_str(), wArg.c_str());
	}

	const std::vector<std::wstring>& SC_CommandLineManager::GetArguments(const wchar_t * aCommand)
	{
		return myCommands.at(aCommand);
	}
}