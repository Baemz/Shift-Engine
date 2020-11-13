#pragma once
#include "SC_TypeTraits.h"

namespace Shift
{
	void SC_Memcpy(void* aDst, void* aSrc, unsigned long long aSize);
	void SC_Memmove(void* aDst, void* aSrc, unsigned long long aSize);
	void SC_Memset(void* aDst, int aValue, unsigned long long aSize);
	void SC_ZeroMemory(void* aDst, unsigned long long aSize);

	template<typename T>
	typename SC_RemoveReference<T>::Type&& SC_Move(T&& aValue)
	{
		using ReturnType = typename SC_RemoveReference<T>::Type &&;
		return static_cast<ReturnType>(aValue);
	}

	// Forward an lvalue as either an lvalue or an rvalue
	template<typename T>
	T&& SC_Forward(typename SC_RemoveReference<T>::Type& aValue)
	{
		return static_cast<T&&>(aValue);
	}

	// Forward an rvalue as an rvalue
	template<typename T>
	T&& SC_Forward(typename SC_RemoveReference<T>::Type&& aValue)
	{
		static_assert(!SC_IsLeftValueReference<T>::ourValue && "Invalid forwarding");
		return static_cast<T&&>(aValue);
	}

	//-------------------------------------------------------------//
	// Min / Max

	template <class Type1, class Type2>
	inline Type1 SC_Min(const Type1& aFirst, const Type2& aSecond)
	{
		return (aFirst < static_cast<Type1>(aSecond)) ? aFirst : static_cast<Type1>(aSecond);
	}

	template <class Type1, class Type2>
	inline Type1 SC_Max(const Type1& aFirst, const Type2& aSecond)
	{
		return (aFirst < static_cast<Type1>(aSecond)) ? static_cast<Type1>(aSecond) : aFirst;
	}

	template<typename Type>
	inline Type SC_Max(const Type& aFirst, const Type& aSecond, const Type& aThird)
	{
		Type temp = SC_Max(aFirst, aSecond);
		return SC_Max(temp, aThird);
	}

	//-------------------------------------------------------------//
	// Clamp

	template<typename Type>
	inline Type SC_Clamp(const Type& aValue, const Type& aMin, const Type& aMax)
	{
		if (aValue < aMin)
			return aMin;
		else if (aValue > aMax)
			return aMax;
		else
			return aValue;
	}

	//-------------------------------------------------------------//
	// Lerp

	template<typename Type, typename Interpolator>
	inline Type SC_Lerp(const Type& aFirst, const Type& aSecond, const Interpolator& aT)
	{
		return aFirst + (aSecond - aFirst) * aT;
	}

	//-------------------------------------------------------------//
	// Array Helpers

	template <typename Type, size_t Size>
	char(&__tempArraySizeVariable(Type(&array)[Size]))[Size];
	#define SC_ARRAY_SIZE(aArray) (sizeof(__tempArraySizeVariable(aArray)))

	template<typename Type>
	inline void SC_Fill(Type* aBuffer, unsigned int aCount, Type aValue)
	{
		for (unsigned int i = 0; i < aCount; ++i)
			aBuffer[i] = aValue;
	}

	template <class Type>
	inline void SC_CopyConstruct(Type* aDest, const Type* aSrc, int aCount)
	{
		for (int i = 0; i < aCount; ++i)
			new (static_cast<void*>(aDest + i)) Type(aSrc[i]);
	}

	template <class Type>
	inline void SC_CopyConstruct(const Type* aDest, const Type* aSrc, int aCount)
	{
		for (int i = 0; i < aCount; ++i)
			new (const_cast<typename SC_RemoveConst<Type>::Type*>(aDest + i)) Type(aSrc[i]);
	}

	template <class Type>
	inline void SC_CopyConstructN(Type* aDest, const Type& aSrc, int aCount)
	{
		for (int i = 0; i < aCount; ++i)
			new (static_cast<void*>(aDest + i)) Type(aSrc);
	}

	template <class Type>
	inline void SC_CopyConstructN(const Type* aDest, const Type& aSrc, int aCount)
	{
		for (int i = 0; i < aCount; ++i)
			new (const_cast<typename SC_RemoveConst<Type>::Type*>(aDest + i)) Type(aSrc);
	}
}