#pragma once

#if IS_VISUAL_STUDIO
	#define SC_FORCEINLINE __forceinline
	#define SC_NOINLINE __declspec(noinline)
	#define SC_RESTRICT __restrict
	#define SC_FASTCALL __fastcall
	#define SC_ALIGN(value) __declspec(align(value))
	#define SC_BRANCH_HINT(condition, value) (condition)
	#define SC_DEPRECATED __declspec(deprecated)
	#define SC_DEPRECATED_MSG(msg) __declspec(deprecated(msg))

	#ifndef _CRT_SECURE_NO_DEPRECATE
		#define _CRT_SECURE_NO_DEPRECATE
	#endif

	#pragma inline_depth(255)
	#pragma inline_recursion(on)
	#pragma auto_inline(on)

	#pragma warning(disable : 4201)		// nonstandard extension used: nameless struct/union
	#pragma warning(disable : 4345)		// behavior change: an object of POD type constructed with an initializer of the form () will be default-initialized
	#pragma warning(disable : 4351)		// new behavior: elements of array 'array' will be default-initialized
	#pragma warning(disable : 4482)		// nonstandard extension used: enum 'enum' used in qualified name
	#pragma warning(disable : 4324) 
	#pragma warning(disable : 26444) 
	#pragma warning(disable : 26495)	// C26495: Variable '_variable_' is uninitialized. Always initialize a member variable.
	#pragma warning(disable : 26812)	// C26812: prefer 'enum class' over 'enum'
#else
#error Compiler not implemented!
#endif