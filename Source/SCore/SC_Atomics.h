#pragma once
#ifndef SC_ATOMICS_H
#define SC_ATOMICS_H

namespace Shift
{
	template<typename T>
	class SC_Atomic
	{
		static T SC_CompareExchange_GetOld(T volatile& aDest, T aValue, T aComperand);
		static bool SC_CompareExchange(T volatile& aDest, T aValue, T aComperand);
		static T SC_Exchange_GetOld(T volatile& aDest, T aValue);
		static T SC_Increment(T volatile& aDestination);
		static T SC_Decrement(T volatile& aDestination);
		static T SC_Increment_GetNew(T volatile& aDestination);
		static T SC_Decrement_GetNew(T volatile& aDestination);
		static T SC_Add(T volatile& aDestination, T aValue);
		static T SC_Sub(T volatile& aDestination, T aValue);
	};

#if IS_WINDOWS

#include <intrin.h>

	template<>
	class SC_Atomic<uint8>
	{
	public:
#		pragma intrinsic (_InterlockedCompareExchange8)
#		pragma intrinsic (_InterlockedExchange8)
#		pragma intrinsic (_InterlockedExchangeAdd8)
#		pragma intrinsic (_InterlockedAnd8)
#		pragma intrinsic (_InterlockedOr8)

		static inline uint8 SC_CompareExchange_GetOld(uint8 volatile& aDest, uint8 aValue, uint8 aComperand) { return _InterlockedCompareExchange8((volatile char*)& aDest, aValue, aComperand); }
		static inline bool SC_CompareExchange(uint8 volatile& aDest, uint8 aValue, uint8 aComperand) { return (SC_CompareExchange_GetOld(aDest, aValue, aComperand) == aComperand); }
		static inline uint8 SC_Exchange_GetOld(uint8 volatile& aDest, uint8 aValue) { return _InterlockedExchange8((volatile char*)&aDest, aValue); }
		static uint8 SC_Increment(uint8 volatile& aDestination) { return SC_Add(aDestination, 1); }
		static uint8 SC_Decrement(uint8 volatile& aDestination) { return SC_Sub(aDestination, 1); }
		static uint8 SC_Increment_GetNew(uint8 volatile& aDestination) { return SC_Add(aDestination, 1) + 1; }
		static uint8 SC_Decrement_GetNew(uint8 volatile& aDestination) { return SC_Sub(aDestination, 1) - 1; }
		static uint8 SC_Add(uint8 volatile& aDestination, uint8 aValue) { return _InterlockedExchangeAdd8((volatile char*)& aDestination, (char)aValue); }
		static uint8 SC_Sub(uint8 volatile& aDestination, uint8 aValue) { return SC_Add(aDestination, -(char)aValue); }
		static uint8 SC_Or(uint8 volatile& aDestination, uint8 aValue) { return _InterlockedOr8((volatile char*)&aDestination, (char)aValue); }
		static uint8 SC_And(uint8 volatile& aDestination, uint8 aValue) { return _InterlockedAnd8((volatile char*)&aDestination, (char)aValue); }
	};

	template<>
	class SC_Atomic<uint16>
	{
	public:
#		pragma intrinsic (_InterlockedCompareExchange16)
#		pragma intrinsic (_InterlockedExchange16)
#		pragma intrinsic (_InterlockedIncrement16)
#		pragma intrinsic (_InterlockedDecrement16)
#		pragma intrinsic (_InterlockedExchangeAdd16)
#		pragma intrinsic (_InterlockedAnd16)
#		pragma intrinsic (_InterlockedOr16)

		static inline uint16 SC_CompareExchange_GetOld(uint16 volatile& aDest, uint16 aValue, uint16 aComperand) { return _InterlockedCompareExchange16((volatile short*)& aDest, aValue, aComperand); }
		static inline bool SC_CompareExchange(uint16 volatile& aDest, uint16 aValue, uint16 aComperand) { return (SC_CompareExchange_GetOld(aDest, aValue, aComperand) == aComperand); }
		static inline uint16 SC_Exchange_GetOld(uint16 volatile& aDest, uint16 aValue) { return _InterlockedExchange16((volatile short*)&aDest, aValue); }
		static uint16 SC_Increment(uint16 volatile& aDestination) { return _InterlockedIncrement16((volatile short*)& aDestination) - 1; }
		static uint16 SC_Decrement(uint16 volatile& aDestination) { return _InterlockedDecrement16((volatile short*)& aDestination) + 1; }
		static uint16 SC_Increment_GetNew(uint16 volatile& aDestination) { return _InterlockedIncrement16((volatile short*)&aDestination); }
		static uint16 SC_Decrement_GetNew(uint16 volatile& aDestination) { return _InterlockedDecrement16((volatile short*)&aDestination); }
		static uint16 SC_Add(uint16 volatile& aDestination, uint16 aValue) { return _InterlockedExchangeAdd16((volatile short*)& aDestination, (short)aValue); }
		static uint16 SC_Sub(uint16 volatile& aDestination, uint16 aValue) { return SC_Add(aDestination, -(short)aValue); }
		static uint16 SC_Or(uint16 volatile& aDestination, uint16 aValue) { return _InterlockedOr16((volatile short*)&aDestination, (short)aValue); }
		static uint16 SC_And(uint16 volatile& aDestination, uint16 aValue) { return _InterlockedAnd16((volatile short*)&aDestination, (short)aValue); }
	};

	template<>
	class SC_Atomic<uint32>
	{
	public:
#		pragma intrinsic (_InterlockedCompareExchange)
#		pragma intrinsic (_InterlockedExchange)
#		pragma intrinsic (_InterlockedIncrement)
#		pragma intrinsic (_InterlockedDecrement)
#		pragma intrinsic (_InterlockedExchangeAdd)
#		pragma intrinsic (_InterlockedAnd)
#		pragma intrinsic (_InterlockedOr)

		static inline uint32 SC_CompareExchange_GetOld(uint32 volatile& aDest, uint32 aValue, uint32 aComperand) { return _InterlockedCompareExchange((volatile long*)& aDest, aValue, aComperand); }
		static inline bool SC_CompareExchange(uint32 volatile& aDest, uint32 aValue, uint32 aComperand) { return (SC_CompareExchange_GetOld(aDest, aValue, aComperand) == aComperand); }
		static inline uint32 SC_Exchange_GetOld(uint32 volatile& aDest, uint32 aValue) { return _InterlockedExchange((volatile long*)&aDest, aValue); }
		static uint32 SC_Increment(uint32 volatile& aDestination) { return _InterlockedIncrement((volatile long*)& aDestination) - 1; }
		static uint32 SC_Decrement(uint32 volatile& aDestination) { return _InterlockedDecrement((volatile long*)& aDestination) + 1; }
		static uint32 SC_Increment_GetNew(uint32 volatile& aDestination) { return _InterlockedIncrement((volatile long*)&aDestination); }
		static uint32 SC_Decrement_GetNew(uint32 volatile& aDestination) { return _InterlockedDecrement((volatile long*)&aDestination); }
		static uint32 SC_Add(uint32 volatile& aDestination, uint32 aValue) { return _InterlockedExchangeAdd((volatile long*)&aDestination, (long)aValue); }
		static uint32 SC_Sub(uint32 volatile& aDestination, uint32 aValue) { return SC_Add(aDestination, -(long)aValue); }
		static uint32 SC_Or(uint32 volatile& aDestination, uint32 aValue) { return _InterlockedOr((volatile long*)&aDestination, (long)aValue); }
		static uint32 SC_And(uint32 volatile& aDestination, uint32 aValue) { return _InterlockedAnd((volatile long*)&aDestination, (long)aValue); }
	};

	template<>
	class SC_Atomic<uint64>
	{
	public:
#		pragma intrinsic (_InterlockedCompareExchange64)
#		pragma intrinsic (_InterlockedExchange64)
#		pragma intrinsic (_InterlockedIncrement64)
#		pragma intrinsic (_InterlockedDecrement64)
#		pragma intrinsic (_InterlockedExchangeAdd64)
#		pragma intrinsic (_InterlockedAnd64)
#		pragma intrinsic (_InterlockedOr64)

		static inline uint64 SC_CompareExchange_GetOld(uint64 volatile& aDest, uint64 aValue, uint64 aComperand) { return _InterlockedCompareExchange64((volatile __int64*)& aDest, aValue, aComperand); }
		static inline bool SC_CompareExchange(uint64 volatile& aDest, uint64 aValue, uint64 aComperand) { return (SC_CompareExchange_GetOld(aDest, aValue, aComperand) == aComperand); }
		static inline uint64 SC_Exchange_GetOld(uint64 volatile& aDest, uint64 aValue) { return _InterlockedExchange64((volatile __int64*)&aDest, aValue); }
		static uint64 SC_Increment(uint64 volatile& aDestination) { return _InterlockedIncrement64((volatile __int64*)& aDestination) - 1; }
		static uint64 SC_Decrement(uint64 volatile& aDestination) { return _InterlockedDecrement64((volatile __int64*)& aDestination) + 1; }
		static uint64 SC_Increment_GetNew(uint64 volatile& aDestination) { return _InterlockedIncrement64((volatile __int64*)&aDestination); }
		static uint64 SC_Decrement_GetNew(uint64 volatile& aDestination) { return _InterlockedDecrement64((volatile __int64*)&aDestination); }
		static uint64 SC_Add(uint64 volatile& aDestination, uint64 aValue) { return _InterlockedExchangeAdd64((volatile __int64*)& aDestination, (__int64)aValue); }
		static uint64 SC_Sub(uint64 volatile& aDestination, uint64 aValue) { return SC_Add(aDestination, -(__int64)aValue); }
		static uint64 SC_Or(uint64 volatile& aDestination, uint64 aValue) { return _InterlockedOr64((volatile __int64*)&aDestination, (__int64)aValue); }
		static uint64 SC_And(uint64 volatile& aDestination, uint64 aValue) { return _InterlockedAnd64((volatile __int64*)&aDestination, (__int64)aValue); }
	};

#endif

#define SE_ATOMIC_DEFINITION(Type1, Type2)\
	static_assert(sizeof(Type1) == sizeof(Type2));\
	inline Type1 SC_Atomic_CompareExchange_GetOld(Type1 volatile& aDest, Type1 aValue, Type1 aComperand) { return (Type1)SC_Atomic<Type2>::SC_CompareExchange_GetOld((Type2 volatile&)aDest, *(Type2*)&aValue, *(Type2*)&aComperand); }\
	inline bool SC_Atomic_CompareExchange(Type1 volatile& aDest, Type1 aValue, Type1 aComperand) { return SC_Atomic<Type2>::SC_CompareExchange((Type2 volatile&)aDest, *(Type2*)&aValue, *(Type2*)&aComperand); }\
	inline Type1 SC_Atomic_Exchange_GetOld(Type1 volatile& aDest, Type1 aValue) { return (Type1)SC_Atomic<Type2>::SC_Exchange_GetOld((Type2 volatile&)aDest, *(Type2*)&aValue);}\
	inline Type1 SC_Atomic_Increment(Type1 volatile& aDest) { return (Type1)SC_Atomic<Type2>::SC_Increment((Type2 volatile&)aDest); }\
	inline Type1 SC_Atomic_Decrement(Type1 volatile& aDest) { return (Type1)SC_Atomic<Type2>::SC_Decrement((Type2 volatile&)aDest); }\
	inline Type1 SC_Atomic_Increment_GetNew(Type1 volatile& aDest) { return (Type1)SC_Atomic<Type2>::SC_Increment_GetNew((Type2 volatile&)aDest); }\
	inline Type1 SC_Atomic_Decrement_GetNew(Type1 volatile& aDest) { return (Type1)SC_Atomic<Type2>::SC_Decrement_GetNew((Type2 volatile&)aDest); }\
	inline Type1 SC_Atomic_Add(Type1 volatile& aDest, Type1 aValue) { return (Type1)SC_Atomic<Type2>::SC_Add((Type2 volatile&)aDest, *(Type2*)&aValue); }\
	inline Type1 SC_Atomic_Sub(Type1 volatile& aDest, Type1 aValue) { return (Type1)SC_Atomic<Type2>::SC_Sub((Type2 volatile&)aDest, *(Type2*)&aValue); }\
	inline Type1 SC_Atomic_Or(Type1 volatile& aDest, Type1 aValue) { return (Type1)SC_Atomic<Type2>::SC_Or((Type2 volatile&)aDest, *(Type2*)&aValue); }\
	inline Type1 SC_Atomic_And(Type1 volatile& aDest, Type1 aValue) { return (Type1)SC_Atomic<Type2>::SC_And((Type2 volatile&)aDest, *(Type2*)&aValue); }


	SE_ATOMIC_DEFINITION(int8, uint8);
	SE_ATOMIC_DEFINITION(uint8, uint8);

	SE_ATOMIC_DEFINITION(int16, uint16);
	SE_ATOMIC_DEFINITION(uint16, uint16);

	SE_ATOMIC_DEFINITION(int32, uint32);
	SE_ATOMIC_DEFINITION(uint32, uint32);

	SE_ATOMIC_DEFINITION(int64, uint64);
	SE_ATOMIC_DEFINITION(uint64, uint64);

}
#endif // SC_ATOMICS_H