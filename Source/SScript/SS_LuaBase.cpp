#include "SScript_Precompiled.h"
#include "SS_LuaBase.h"
#include "SS_LuaScript.h"

namespace Shift
{
	SS_LuaBase::SS_LuaBase()
	{
	}

	SS_LuaBase::~SS_LuaBase()
	{
	}

	SC_Ref<SS_LuaScript> SS_LuaBase::CreateScript(const char* aScriptFile)
	{
		SC_Ref<SS_LuaScript> script = new SS_LuaScript();

		script->myLuaState = luaL_newstate();
		luaL_openlibs(script->myLuaState);

		if (luaL_dofile(script->myLuaState, aScriptFile))
		{
			SC_ERROR_LOG("Could not load LUA script file (%s)", aScriptFile);
			SS_PRINT_LUA_ERROR(script->myLuaState);
			return nullptr;
		}

		SS_LuaScript* scriptPtr = script;
		auto watchFn = [&, scriptPtr](const std::string& aFile, const EFileEvent&)
		{
			lua_State* state = luaL_newstate();
			luaL_openlibs(state);

			if (luaL_dofile(state, aFile.c_str()))
			{
				SC_ERROR_LOG("Could not load LUA script file (%s)", aFile.c_str());
				SS_PRINT_LUA_ERROR(state);
				lua_close(state);
				return;
			}

			SC_MutexLock lock(scriptPtr->myMutex);
			lua_close(scriptPtr->myLuaState);
			scriptPtr->myLuaState = state;
		};
		SC_EngineInterface::WatchFile(aScriptFile, watchFn);

		return script;
	}
}