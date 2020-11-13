#pragma once

namespace Shift
{
	template <typename T> struct SC_RemoveFnConstVolatileReference																{ typedef T Type; };
	template <typename Fn, class... Args> struct SC_RemoveFnConstVolatileReference<Fn(Args...) const>							{ typedef Fn Type(Args...); };
	template <typename Fn, class... Args> struct SC_RemoveFnConstVolatileReference<Fn(Args...) const &>							{ typedef Fn Type(Args...); };
	template <typename Fn, class... Args> struct SC_RemoveFnConstVolatileReference<Fn(Args...) const &&>						{ typedef Fn Type(Args...); };
	template <typename Fn, class... Args> struct SC_RemoveFnConstVolatileReference<Fn(Args...) volatile const>					{ typedef Fn Type(Args...); };
	template <typename Fn, class... Args> struct SC_RemoveFnConstVolatileReference<Fn(Args...) volatile const &>				{ typedef Fn Type(Args...); };
	template <typename Fn, class... Args> struct SC_RemoveFnConstVolatileReference<Fn(Args...) volatile const &&>				{ typedef Fn Type(Args...); };
	template <typename Fn, class... Args> struct SC_RemoveFnConstVolatileReference<Fn(Args...) volatile >						{ typedef Fn Type(Args...); };
	template <typename Fn, class... Args> struct SC_RemoveFnConstVolatileReference<Fn(Args...) volatile &>						{ typedef Fn Type(Args...); };
	template <typename Fn, class... Args> struct SC_RemoveFnConstVolatileReference<Fn(Args...) volatile &&>						{ typedef Fn Type(Args...); };

	template <typename T> struct SC_IsFunctionInternal																			{ static constexpr const bool ourValue = false; };
	template <class Ret, class... Args> struct SC_IsFunctionInternal<Ret(Args...)>												{ static constexpr const bool ourValue = true; };

	template <class T>
	struct SC_IsFunction : SC_IsFunctionInternal<typename SC_RemoveFnConstVolatileReference<T>::Type>							{};
	template <typename T> struct SC_IsMemberFunctionPointer																		{ static constexpr const bool ourValue = false; };
	template <typename Function, typename Class> struct SC_IsMemberFunctionPointer<Function Class::*> : SC_IsFunction<Function>	{ typedef typename SC_RemoveFnConstVolatileReference<Function>::Type Signature; };
	template<typename T> struct SC_IsLeftValueReference																			{ static const bool ourValue = false; };
	template<typename T> struct SC_IsLeftValueReference<T&>																		{ static const bool ourValue = false; };

	template<typename T> struct SC_RemoveReference																				{ typedef T Type; };
	template<typename T> struct SC_RemoveReference<T&>																			{ typedef T Type; };
	template<typename T> struct SC_RemoveReference<T&&>																			{ typedef T Type; };

	template <typename T> struct SC_RemoveConst																					{ typedef T Type; };
	template <typename T> struct SC_RemoveConst<const T>																		{ typedef T Type; };

	template<bool Condition, typename T = void>
	struct SC_EnableIf
	{
		// nothing when Condition is false
	};

	template<typename T>
	struct SC_EnableIf<true, T>
	{
		using Type = T;
	};

	template<bool Condition, class T = void>
	using SC_EnableIfT = typename SC_EnableIf<Condition, T>::Type;

}