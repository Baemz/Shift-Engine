#pragma once
#include "SC_TypeTraits.h"
#include "SC_Vector.h"
#include "SC_String.h"
#include <string>

using uint = unsigned int;
namespace Shift
{
	template<typename Type>
	struct SC_HasAllowRawHashing
	{
		template<typename U>	static char SFINAE(typename U::SC_AllowRawHashing_INTERNAL*);
		template<typename>		static long SFINAE(...);

		enum { Value = sizeof(SFINAE<Type>(0)) == sizeof(char) };
	};

	template<typename Type>
	struct SC_RawHashingAllowed
	{
		enum
		{
			Value = !std::is_class<Type>::value || SC_HasAllowRawHashing<Type>::Value
		};
	};

#define SC_ENABLE_RAW_HASHING(Type) template<> struct SC_RawHashingAllowed<Type> { enum { Value = 1 }; }

	uint SC_HashData(const void* aData, uint aSize, uint aSeed = 0);

	template<int N> 
	inline uint SC_HashDataFixedSize(const void* aData) { return SC_HashData(aData, N); }

	template<> 
	inline uint SC_HashDataFixedSize<4>(const void* aData)
	{
		const uint64 t = *(uint*)(aData) * 0x9e3779b97f4a7c19ull;
		return static_cast<uint>(t ^ (t >> 32));
	}

	template<> 
	inline uint SC_HashDataFixedSize<8>(const void* aData)
	{
		const uint64 t = *(uint64*)(aData) * 0x9e3779b97f4a7c19ull;
		return static_cast<uint>(t ^ (t >> 32));
	}

	template<> 
	inline uint SC_HashDataFixedSize<16>(const void* aData)
	{
		uint64* data = (uint64*)aData;
		const uint data1 = static_cast<uint>((data[0] * 0x9e3779b97f4a7c19ull) >> 32);
		const uint data2 = static_cast<uint>((data[1] * 0x9e3779b97f4a7c19ull) >> 32);
		return data1 ^ data2;
	}

	template<class Type>
	inline uint SC_Hash(const Type& aData)
	{
		return SC_HashDataFixedSize<sizeof(aData)>(
			static_cast<const SC_EnableIfT<SC_RawHashingAllowed<Type>::Value, Type>*>(&aData)
			);
	}

	inline uint SC_Hash(const char* const& aString) { return SC_HashData(aString, (uint)strlen(aString)); }
	inline uint SC_Hash(const std::string& aString) { return SC_HashData(aString.data(), (uint)aString.length()); }
	inline uint SC_Hash(const SC_String& aString) { return SC_HashData(aString.GetBuffer(), aString.Length()); }

	template<class T0, class T1>	inline bool SC_Compare(const T0& aA, const T1& aB) { return (aA == aB); }
	inline bool SC_Compare(const char* const& aA, const char* const& aB) { return (strcmp(aA, aB) == 0); }
	inline bool SC_Compare(const char* const& aA, const std::string& aB) { return (strcmp(aA, aB.c_str()) == 0); }
	inline bool SC_Compare(const std::string& aA, const char* const& aB) { return (strcmp(aA.c_str(), aB) == 0); }

	//added the float stuff as special cases since 0 == -0 if comparing floats normally which can crash the hashmap when resizing
	inline bool SC_Compare(const float& aA, const float& aB) { return memcmp(&aA, &aB, sizeof(aA)) == 0; }
	inline bool SC_Compare(const SC_Vector2f& aA, const SC_Vector2f& aB) { return memcmp(&aA, &aB, sizeof(aA)) == 0; }
	inline bool SC_Compare(const SC_Vector3f& aA, const SC_Vector3f& aB) { return memcmp(&aA, &aB, sizeof(aA)) == 0; }
	inline bool SC_Compare(const SC_Vector4f& aA, const SC_Vector4f& aB) { return memcmp(&aA, &aB, sizeof(aA)) == 0; }

}