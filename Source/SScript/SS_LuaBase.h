#pragma once

namespace Shift
{
	class SS_LuaScript;
	class SS_LuaBase
	{
	public: 
		SS_LuaBase();
		~SS_LuaBase();

		bool Init();

		SC_Ref<SS_LuaScript> CreateScript(const char* aScriptFile);

	private:

	};
}


