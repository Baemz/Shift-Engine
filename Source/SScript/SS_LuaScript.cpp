#include "SScript_Precompiled.h"
#include "SS_LuaScript.h"

namespace Shift
{
	SS_LuaScript::SS_LuaScript()
		: myLuaState(nullptr)
	{
	}

	SS_LuaScript::~SS_LuaScript()
	{
		lua_close(myLuaState);
	}

	void SS_LuaScript::InitScript()
	{
		if (!CallFunction("Start"))
			SC_ERROR_LOG("Scripts must contain a 'Start' function.");
	}

	bool SS_LuaScript::CallFunction(const char* aFunctionName)
	{
#if SS_DEBUG_TIME_FUNCTION_CALLS
		SC_Stopwatch __timer;
#endif
		SC_MutexLock lock(myMutex);
		lua_getglobal(myLuaState, aFunctionName);
		bool exists = (!lua_isnil(myLuaState, -1) && lua_isfunction(myLuaState, -1));
		if (!exists)
		{
			SC_ERROR_LOG("Function (%s) does not exist in LUA script.", aFunctionName);
			return false;
		}

		lua_call(myLuaState, 0, 0);

#if SS_DEBUG_TIME_FUNCTION_CALLS
		SC_LOG("LUA function took %.3f ms to execute.", __timer.Stop());
#endif

		return true;
	}
}