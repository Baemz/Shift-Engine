#pragma once
#include "SC_RefCounted.h"

struct lua_State;
namespace Shift
{
	class SS_LuaScript : public SC_RefCounted
	{
		friend class SS_LuaBase;
	public:
		SS_LuaScript();
		~SS_LuaScript();

		void InitScript();

		bool CallFunction(const char* aFunctionName);

	private:
		SC_Mutex myMutex;
		lua_State* myLuaState;
	};
}
