#pragma once
#include "SC_CommonFuncs.h"
#include <new>

namespace Shift
{
	template<typename T>
	struct SC_AllowMemcpyRelocation
	{
		static const bool value = __has_trivial_destructor(T) || __is_trivial(T);
	};

	template <class T, bool AllowMemcpy> struct SC_RelocationUtil;

	template <class T>
	struct SC_RelocationUtil<T, false>
	{
		static inline void Relocate(T* aDstLocation, T* aSrcLocation)
		{
			new (const_cast<typename SC_RemoveConst<T>::Type*>(aDstLocation)) T(SC_Move(*aSrcLocation));
			(aSrcLocation)->~T();
		}

		static inline void RelocateN(T* aDstLocation, T* aSrcLocation, int aCount)
		{
			if (aDstLocation == aSrcLocation || !aCount)
				return;

			if ((uintptr_t)aDstLocation < (uintptr_t)aSrcLocation)
			{
				for (int i = 0; i < aCount; ++i)
					Relocate(aDstLocation + i, aSrcLocation + i);
			}
			else if ((uintptr_t)aSrcLocation < (uintptr_t)aDstLocation)
			{
				for (int i = aCount - 1; i >= 0; --i)
					Relocate(aDstLocation + i, aSrcLocation + i);
			}
		}
	};

	void SC_RelocateBytes(void* aDstLocation, void* aSrcLocation, size_t aCount);

	template <class T>
	struct SC_RelocationUtil<T, true>
	{
		static inline void Relocate(T* aDstLocation, T* aSrcLocation)
		{
			switch (sizeof(T))
			{
			case 1:
				*((int8*)aDstLocation) = *((int8*)aSrcLocation);
				break;
			case 2:
				*((int16*)aDstLocation) = *((int16*)aSrcLocation);
				break;
			case 4:
				*((int32*)aDstLocation) = *((int32*)aSrcLocation);
				break;
			case 8:
				*((int64*)aDstLocation) = *((int64*)aSrcLocation);
				break;
			default:
				memcpy((void*)aDstLocation, (const void*)aSrcLocation, sizeof(T));
				break;
			}
		}
		static inline void RelocateN(T* aDstLocation, T* aSrcLocation, int aCount)
		{
			SC_RelocateBytes((void*)aDstLocation, (void*)aSrcLocation, aCount * sizeof(T));
		}
	};

	template <class T>
	inline void SC_Relocate(T* aDstLocation, T* aSrcLocation)
	{
		if (aDstLocation == aSrcLocation)
			return;

		SC_RelocationUtil<T, SC_AllowMemcpyRelocation<T>::value>::Relocate(aDstLocation, aSrcLocation);
	}

	template <class T>
	inline void SC_RelocateN(T* aDstLocation, T* aSrcLocation, int aCount)
	{
		if (aDstLocation == aSrcLocation)
			return;

		SC_RelocationUtil<T, SC_AllowMemcpyRelocation<T>::value>::RelocateN(aDstLocation, aSrcLocation, aCount);
	}

#define SC_ALLOW_RELOCATE(Type) \
	template<> \
	struct SC_AllowMemcpyRelocation<Type> \
	{ \
		static const bool value = true; \
	};
}