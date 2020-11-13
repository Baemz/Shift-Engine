#pragma once

#include "SC_CommonDefines.h"
#include "SC_CommonIncludes.h"

#include "lua.hpp"

#define SS_DEBUG_TIME_FUNCTION_CALLS (0)

#define SS_PRINT_LUA_ERROR(aState)  const char* errorMsg = lua_tostring(aState, -1);\
									SC_ERROR_LOG("Error: %s", errorMsg);\
									lua_pop(aState, 1)