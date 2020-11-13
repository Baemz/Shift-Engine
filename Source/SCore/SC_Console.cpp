#include "SCore_Precompiled.h"
#include "SC_Console.h"

namespace Shift
{
	SC_Console::SC_Console()
	{
	}

	SC_Console::~SC_Console()
	{
	}

	void SC_Console::RegisterCommand(const char* aId, SC_ConsoleCommand* aCommand)
	{
		if (!myCommands.Find(aId))
			myCommands.Insert(aId, aCommand);
		else
			SC_ASSERT(false, "Command (%s) already registered to SC_Console.", aId);
	}
}