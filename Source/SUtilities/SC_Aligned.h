#pragma once

namespace Shift
{
	inline uint32 SC_Align(uint32 aValue, uint32 aAlignment)
	{
		return (aValue + aAlignment - 1) & ~(aAlignment - 1);
	}

	inline uint64 SC_Align(uint64 aValue, uint64 aAlignment)
	{
		return (aValue + aAlignment - 1) & ~(aAlignment - 1);
	}

	template<uint Size, uint Alignment> struct SC_AlignedData_Internal;

	template<uint Size>
	struct SC_AlignedData_Internal<Size, 1>
	{
		char myBuffer[Size];
	};

	template<uint Size>
	struct SC_AlignedData_Internal<Size, 2>
	{
		SC_ALIGN(2) char myBuffer[Size];
	};

	template<uint Size>
	struct SC_AlignedData_Internal<Size, 4>
	{
		SC_ALIGN(4) char myBuffer[Size];
	};

	template<uint Size>
	struct SC_AlignedData_Internal<Size, 8>
	{
		SC_ALIGN(8) char myBuffer[Size];
	};

	template<uint Size>
	struct SC_AlignedData_Internal<Size, 16>
	{
		SC_ALIGN(16) char myBuffer[Size];
	};

	template<uint Size>
	struct SC_AlignedData_Internal<Size, 32>
	{
		SC_ALIGN(32) char myBuffer[Size];
	};

	template<uint Size>
	struct SC_AlignedData_Internal<Size, 64>
	{
		SC_ALIGN(64) char myBuffer[Size];
	};

	template<class Type>
	struct SC_AlignedData : SC_AlignedData_Internal<sizeof(Type), __alignof(Type)>
	{
		Type* Ptr() { return reinterpret_cast<Type*>(&this->myBuffer[0]); }
		const Type* Ptr() const { return reinterpret_cast<const Type*>(&this->myBuffer[0]); }
	};
}