#pragma once

namespace Shift
{
	struct SC_ConsoleCommand
	{
		SC_ConsoleCommand()					{}
		virtual ~SC_ConsoleCommand()		{}
		virtual void Execute()				{ assert(false && "missing implementation"); }
		virtual void Execute(int)			{ assert(false && "missing implementation"); }
		virtual void Execute(int, int)		{ assert(false && "missing implementation"); }
		virtual void Execute(float)			{ assert(false && "missing implementation"); }
		virtual void Execute(float, float)	{ assert(false && "missing implementation"); }
		virtual void Execute(const char*)	{ assert(false && "missing implementation"); }
	};

	class SC_Console
	{
	public: 
		SC_Console();
		~SC_Console();

		void RegisterCommand(const char* aId, SC_ConsoleCommand* aCommand);

		template<typename... Args>
		void ExecuteCommand(const char* aCommand, Args... aArgs)
		{
			if (myCommands.Find(aCommand))
			{
				/* Error	C2661 - no overloaded function takes 'x' arguments */
				/* Missing overload error here means that you're sending arguments that are not overloaded by SC_ConsoleCommand */
				myCommands[aCommand]->Execute(aArgs...);
			}
		}

	private:
		SC_HashMap<const char*, SC_ConsoleCommand*> myCommands;
	};
}
