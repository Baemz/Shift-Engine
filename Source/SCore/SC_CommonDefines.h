#pragma once

#include "SC_PlatformDefines.h"
#include "SC_BaseDefines.h"

// DEBUG DEFINES
#ifndef _FINAL
#	define IS_FINAL			(0)
#	define IS_FINAL_BUILD	(0)
#	define USE_CONSOLE		(0)
#	define USE_LOGGING		(1)
#	define IS_EDITOR_BUILD	(1)
#	define USE_DEBUG_NAMES	(1)
#	define ENABLE_PROFILER	(1)
#	define ENABLE_ASSERTS	(1)
#else
#	define IS_FINAL			(1)
#	define IS_FINAL_BUILD	(1)
#	define USE_CONSOLE		(0)
#	define USE_LOGGING		(0)
#	define IS_EDITOR_BUILD	(0)
#	define USE_DEBUG_NAMES	(0)
#	define ENABLE_PROFILER	(0)
#	define ENABLE_ASSERTS	(0)
#endif
// Defines

#define _SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING 1

#if _DEBUG
#	define IS_DEBUG_BUILD	(1)
#else
#	define IS_DEBUG_BUILD	(0)
#endif

#define SC_STRINGIFY2(string) #string
#define SC_STRINGIFY(string) SC_STRINGIFY2(string)

#define SC_PRAGMA_DEOPTIMIZE __pragma(optimize("", off))
#define SC_PRAGMA_OPTIMIZE __pragma(optimize("", on))

#define SC_CAST(type, val) static_cast<type>(val)
#define SC_CAST_SHORT(num) static_cast<short>(num)
#define SC_CAST_USHORT(num) static_cast<unsigned short>(num)
#define SC_CAST_INT(num) static_cast<int>(num)
#define SC_CAST_UINT(num) static_cast<unsigned int>(num)

#define ENABLE_WORKER_POOL (1)

#define KB (1024)
#define MB (1024*1024)
#define GB (1024*1024*1024ull)

#define SC_UNUSED(aVariable) (void)aVariable

#define SC_SAFE_RELEASE(aComPointer) if(aComPointer != nullptr) { aComPointer->Release(); } aComPointer = nullptr
#define SC_SAFE_DELETE(aPointer) delete aPointer; aPointer = nullptr;
#define SC_SAFE_DELETE_ARRAY(aPointer) delete[] aPointer; aPointer = nullptr;

#if USE_CONSOLE

#define S_LOGV(text, ...) {SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), static_cast<unsigned short>(0x0F)); vprintf(text, __VA_ARGS__); printf("\n");}
#define S_INIT_LOGV(text, ...) {SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), static_cast<unsigned short>(0x0A)); vprintf(text, __VA_ARGS__); printf("\n");}
#define S_WARNING_LOGV(text, ...) {SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), static_cast<unsigned short>(0x0E)); vprintf(text, __VA_ARGS__); printf("\n");}
#define S_ERROR_LOGV(text, ...) {SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), static_cast<unsigned short>(0x0C)); vprintf("[ERROR] "); printf(text, __VA_ARGS__); printf("\n");}

#define vLog(text, ...) S_LOGV(text, __VA_ARGS__)
#define vInit_Log(text, ...) S_INIT_LOGV(text, __VA_ARGS__)
#define vWarning_Log(text, ...) S_WARNING_LOGV(text, __VA_ARGS__)
#define vError_Log(text, ...) S_ERROR_LOGV(text, __VA_ARGS__)
#else

#define S_LOGV(text, ...) {}
#define S_INIT_LOGV(text, ...)  {}
#define S_WARNING_LOGV(text, ...) {}
#define S_ERROR_LOGV(text, ...) {}

#endif