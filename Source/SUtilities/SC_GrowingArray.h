#pragma once
#include "SC_Container.h"
#include "SC_DetectionIdiom.h"
#include <type_traits>
#include "SC_Relocate.h"

namespace Shift
{
	template<class Type>
	class SC_GrowingArray : public SC_Container<Type>
	{
		template<class Type>
		using Equal_Type = decltype(std::declval<const Type>() == std::declval<const Type>());

		template<class Type>
		static constexpr bool Has_Equal_Type = SC_IsDetected<Equal_Type, Type>::value;

		template<class Type>
		using Assignment_Type = decltype(std::declval<const Type>() = std::declval<const Type>());

		template<class Type>
		static constexpr bool Has_Assignment_Type = SC_IsDetected<Assignment_Type, Type>::value;

		using iterator = Type*;
		using const_iterator = const Type*;

	public:
		SC_GrowingArray();
		virtual ~SC_GrowingArray();
		SC_GrowingArray(const SC_GrowingArray& aArray);
		SC_GrowingArray(SC_GrowingArray&& aArray) noexcept;

		void PreAllocate(uint aCount);
		void AllocateAdditional(uint aCount);
		void Respace(uint aCount);
		Type* GetBuffer() const { return myData; }

		SC_FORCEINLINE Type* Begin() const override { return (myData) ? &myData[0] : nullptr; }
		SC_FORCEINLINE Type* End() const override { return (myData) ? &myData[this->myCount] : nullptr; }
		SC_FORCEINLINE iterator begin() { return Begin(); };
		SC_FORCEINLINE const_iterator begin() const { return Begin(); };
		SC_FORCEINLINE iterator end() { return End(); };
		SC_FORCEINLINE const_iterator end() const { return End(); };

		int Find(const Type& aItem, uint aLookFromIndex = 0) override;

		Type& GetLast();
		const Type& GetLast() const;
		Type& GetFirst();
		const Type& GetFirst() const;

		Type& Add() override;
		Type& Add(const Type& aItem) override;
		Type& Add(Type&& aItem) override;
		void AddN(const SC_GrowingArray<Type>& aArray);
		void AddN(Type* aList, uint aCount) override;
		void AddN(const Type& aItem, uint aCount);
		bool AddUnique(const Type& aItem);

		void FillN(const Type& aItem, uint aCount); // Empties the list and fills it with the provided item.

		void Remove(const Type& aItem) override;
		void RemoveCyclic(const Type& aItem) override;
		void RemoveByIndex(uint aIndex) override;
		void RemoveByIndexCyclic(uint aIndex) override;
		void RemoveAll() override;
		void RemoveLast() override;
		void RemoveFirst();
		void RemoveFirstN(uint aNumItemsToRemove);

		void Swap(SC_GrowingArray<Type>& aOther);

		Type& operator[](uint aIndex) override;
		const Type& operator[](uint aIndex) const override;
		void operator=(const SC_GrowingArray<Type>& aOther);
		void operator=(SC_GrowingArray<Type>&& aOther);

		bool operator==(const SC_GrowingArray<Type>& aOther) const
		{
			if (this->myCount != aOther.myCount)
				return false;

			for (uint i = 0; i < this->myCount; ++i)
			{
				if (myData[i] != aOther.myData[i])
					return false;
			}

			return true;
		}

		bool IsHybridBuffer(Type* aBuffer);
		virtual uint GetHybridBufferSize();
		virtual Type* GetHybridBuffer();

		void Reset();

	protected:
		Type* Allocate(uint aCount);
		void Deallocate(Type* aPtr);

		void Construct(Type* aPointer, uint aCount);
		void Destruct(Type* aStart, uint aCount);

		Type* myData;
		uint myIsHybrid : 1;
	};

	template<class Type>
	inline uint SC_Hash(const SC_GrowingArray<Type>& anArray)
	{
		return SC_HashData(anArray.GetBuffer(), anArray.GetByteSize());
	}

	template<class Type>
	inline SC_GrowingArray<Type>::SC_GrowingArray()
		: SC_Container<Type>()
		, myData(nullptr)
		, myIsHybrid(0)
	{
	}

	template<class Type>
	inline SC_GrowingArray<Type>::~SC_GrowingArray()
	{
		Reset();
	}

	template<class Type>
	inline SC_GrowingArray<Type>::SC_GrowingArray(const SC_GrowingArray& aArray)
		: myData(nullptr)
		, myIsHybrid(0)
	{
		this->myCount = aArray.myCount;
		PreAllocate(this->myCount);
		if (this->myCount)
			SC_CopyConstruct(myData, aArray.myData, this->myCount);
	}

	template<class Type>
	inline SC_GrowingArray<Type>::SC_GrowingArray(SC_GrowingArray&& aArray) noexcept
	{
		myData = aArray.myData;
		this->myCount = aArray.myCount;
		this->myAllocatedCount = aArray.myAllocatedCount;
		myIsHybrid = 0;

		aArray.myData = nullptr;
		aArray.myCount = 0;
		aArray.myAllocatedCount = 0;
	}

	template<class Type>
	inline void SC_GrowingArray<Type>::PreAllocate(uint aCount)
	{
		if (aCount <= this->myAllocatedCount)
			return;

		Type* oldData = myData;
		myData = Allocate(aCount);
		this->myAllocatedCount = aCount;

		if (oldData)
		{
			SC_RelocateN(myData, oldData, this->myCount);

			if (!IsHybridBuffer(oldData))
				Deallocate(oldData);
		}

	}

	template<class Type>
	inline void SC_GrowingArray<Type>::AllocateAdditional(uint aCount)
	{
		const uint requestedCount = this->myCount + aCount;

		if (requestedCount <= this->myAllocatedCount)
			return;

		uint increaseCount = this->myCount + ((this->myCount + 1) / 2);
		uint newAllocatedCount = SC_Max(increaseCount, requestedCount);

		PreAllocate(newAllocatedCount);
	}

	template<class Type>
	inline void SC_GrowingArray<Type>::Respace(uint aCount)
	{
		if (aCount == this->myCount)
			return;

		PreAllocate(aCount);

		if (aCount > this->myCount)
			Construct(myData + this->myCount, aCount - this->myCount);
		else
			Destruct(myData + aCount, this->myCount - aCount);

		this->myCount = aCount;
	}

	template<class Type>
	inline int SC_GrowingArray<Type>::Find(const Type & aItem, uint aLookFromIndex)
	{
		if (aLookFromIndex >= this->myCount)
			return this->FoundNone;

		if constexpr (Has_Equal_Type<Type>)
		{
			for (uint i = aLookFromIndex; i < this->myCount; ++i)
			{
				if (myData[i] == aItem)
					return i;
			}
		}
		else
		{
			(void)(aItem);
			(void)(aLookFromIndex);
		}
		return this->FoundNone;
	}

	template<class Type>
	inline Type& SC_GrowingArray<Type>::GetLast()
	{
		return myData[this->myCount - 1];
	}

	template<class Type>
	inline const Type& SC_GrowingArray<Type>::GetLast() const
	{
		return myData[this->myCount - 1];
	}

	template<class Type>
	inline Type& SC_GrowingArray<Type>::GetFirst()
	{
		return myData[0];
	}

	template<class Type>
	inline const Type& SC_GrowingArray<Type>::GetFirst() const
	{
		return myData[0];
	}

	template<class Type>
	inline Type & SC_GrowingArray<Type>::Add()
	{
		AllocateAdditional(1);

		const uint count = this->myCount;
		new (const_cast<typename SC_RemoveConst<Type>::Type*>(myData + count)) Type();
		this->myCount = count + 1;
		return myData[count];
	}

	template<class Type>
	inline Type & SC_GrowingArray<Type>::Add(const Type & aItem)
	{
		AllocateAdditional(1);

		const uint count = this->myCount;
		new (const_cast<typename SC_RemoveConst<Type>::Type*>(myData + count)) Type(aItem);
		this->myCount = count + 1;
		return myData[count];
	}

	template<class Type>
	inline Type& SC_GrowingArray<Type>::Add(Type&& aItem)
	{
		AllocateAdditional(1);

		const uint count = this->myCount;
		new (const_cast<typename SC_RemoveConst<Type>::Type*>(myData + count)) Type(SC_Move(aItem));
		this->myCount = count + 1;
		return myData[count];
	}

	template<class Type>
	inline void SC_GrowingArray<Type>::AddN(const SC_GrowingArray<Type>& aArray)
	{
		if (aArray.myCount == 0)
			return;

		AllocateAdditional(aArray.myCount);
		SC_CopyConstruct(myData + this->myCount, aArray.myData, aArray.myCount);
		this->myCount += aArray.myCount;
	}

	template<class Type>
	inline void SC_GrowingArray<Type>::AddN(Type* aList, uint aCount)
	{
		if (aList == nullptr)
			return; 

		AllocateAdditional(aCount);

		SC_CopyConstruct(myData + this->myCount, aList, aCount);

		this->myCount += aCount;
	}

	template<class Type>
	inline void SC_GrowingArray<Type>::AddN(const Type& aItem, uint aCount)
	{
		AllocateAdditional(aCount);
		
		uint newCount = this->myCount + aCount;

		for (uint i = this->myCount; i < newCount; ++i)
			new (const_cast<typename SC_RemoveConst<Type>::Type*>(myData + i)) Type(aItem);

		this->myCount = newCount;
	}
		
	template<class Type>
	inline bool SC_GrowingArray<Type>::AddUnique(const Type& aItem)
	{
		for (uint i = 0; i < this->myCount; ++i)
		{
			if (myData[i] == aItem)
				return false; // Object already added
		}

		Add(aItem);
		return true;
	}

	template<class Type>
	inline void SC_GrowingArray<Type>::FillN(const Type & aItem, uint aCount)
	{
		RemoveAll();
		for (uint i = 0; i < aCount; ++i)
		{
			Add(aItem);
		}
	}

	template<class Type>
	inline void SC_GrowingArray<Type>::Remove(const Type & aItem)
	{
		int index = Find(aItem);
		if (index == this->FoundNone)
			return;

		RemoveByIndex(index);
	}

	template<class Type>
	inline void SC_GrowingArray<Type>::RemoveCyclic(const Type & aItem)
	{
		int index = Find(aItem);
		if (index == this->FoundNone)
			return;

		RemoveByIndexCyclic((uint)index);
	}

	template<class Type>
	inline void SC_GrowingArray<Type>::RemoveByIndex(uint aIndex)
	{
		SC_ASSERT((aIndex < this->myCount), "Index out of bounds!");

		Destruct((myData + aIndex), 1);
		SC_RelocateN((myData + aIndex), (myData + aIndex + 1), (this->myCount - aIndex - 1));

		--this->myCount;
	}

	template<class Type>
	inline void SC_GrowingArray<Type>::RemoveByIndexCyclic(uint aIndex)
	{
		SC_ASSERT((aIndex < this->myCount), "Index out of bounds!");

		Destruct((myData + aIndex), 1);
		SC_Relocate((myData + aIndex), (myData + this->myCount - 1));
		--(this->myCount);
	}

	template<class Type>
	inline void SC_GrowingArray<Type>::RemoveAll()
	{
		Destruct(myData , this->myCount);
		this->myCount = 0;
	}
	template<class Type>
	inline void SC_GrowingArray<Type>::RemoveLast()
	{
		--(this->myCount);
		(myData + this->myCount)->~Type();
	}
	
	template<class Type>
	inline void SC_GrowingArray<Type>::RemoveFirst()
	{
		RemoveByIndex(0);
	}

	template<class Type>
	inline void SC_GrowingArray<Type>::RemoveFirstN(uint aNumItemsToRemove)
	{
		SC_ASSERT((aNumItemsToRemove <= this->myCount), "Trying to remove more items than exists!");

		if (aNumItemsToRemove == 0)
			return;
		else if (aNumItemsToRemove == this->myCount)
			RemoveAll();

		Destruct(myData, aNumItemsToRemove);
		SC_RelocateN(myData, (myData + aNumItemsToRemove), aNumItemsToRemove);
		this->myCount -= aNumItemsToRemove;
	}

	template<class Type>
	inline void SC_GrowingArray<Type>::Swap(SC_GrowingArray<Type>& aOther)
	{
		const size_t sizeofThis = sizeof(*this);
		char tmp[sizeofThis];
		memcpy(tmp, &aOther, sizeofThis);
		memcpy(&aOther, this, sizeofThis);
		memcpy(this, tmp, sizeofThis);
	}

	template<class Type>
	inline Type& SC_GrowingArray<Type>::operator[](uint aIndex)
	{
		SC_ASSERT((aIndex < this->myCount), "Index out of bounds!");
		return myData[aIndex];
	}

	template<class Type>
	inline const Type& SC_GrowingArray<Type>::operator[](uint aIndex) const
	{
		SC_ASSERT((aIndex < this->myCount), "Index out of bounds!");
		return myData[aIndex];
	}

	template<class Type>
	inline void SC_GrowingArray<Type>::operator=(const SC_GrowingArray<Type>& aOther)
	{
		if (this == &aOther)
			return;

		this->myCount = aOther.myCount;
		PreAllocate(this->myCount);
		if (this->myCount)
			SC_CopyConstruct(myData, aOther.myData, this->myCount);
	}

	template<class Type>
	inline void SC_GrowingArray<Type>::operator=(SC_GrowingArray<Type>&& aOther)
	{
		if (this == &aOther)
			return;

		myData = aOther.myData;
		this->myCount = aOther.myCount;
		this->myAllocatedCount = aOther.myAllocatedCount;
		myIsHybrid = 0;

		aOther.myData = nullptr;
		aOther.myCount = 0;
		aOther.myAllocatedCount = 0;
	}

	template<class Type>
	inline bool SC_GrowingArray<Type>::IsHybridBuffer(Type* aBuffer)
	{
		if (!this->myIsHybrid)
			return false;

		return aBuffer == GetHybridBuffer();
	}

	template<class Type>
	inline uint SC_GrowingArray<Type>::GetHybridBufferSize()
	{
		SC_ASSERT(this->myIsHybrid, "This is not a Hybrid Buffer");
		return 0;
	}

	template<class Type>
	inline Type* SC_GrowingArray<Type>::GetHybridBuffer()
	{
		SC_ASSERT(this->myIsHybrid, "This is not a Hybrid Buffer");
		return nullptr;
	}

	template<class Type>
	inline void SC_GrowingArray<Type>::Reset()
	{
		RemoveAll();

		if (myData && !IsHybridBuffer(myData))
		{
			Deallocate(myData);
			myData = nullptr;

			this->myCount = 0;
			this->myAllocatedCount = 0;
		}

		if (myIsHybrid)
		{
			myData = GetHybridBuffer();
			this->myAllocatedCount = GetHybridBufferSize();
		}
	}

	template<class Type>
	inline Type* SC_GrowingArray<Type>::Allocate(uint aCount)
	{
		if (aCount == 0)
			return nullptr;

		return (Type*) new char[sizeof(Type) * aCount];
	}

	template<class Type>
	inline void SC_GrowingArray<Type>::Deallocate(Type* aPtr)
	{
		delete[](char*)aPtr;
	}

	template<class Type>
	inline void SC_GrowingArray<Type>::Construct(Type* aStart, uint aCount)
	{
		for (uint i = 0; i < aCount; ++i)
			new (const_cast<typename SC_RemoveConst<Type>::Type*>(aStart + i)) Type();
	}

	template<class Type>
	inline void SC_GrowingArray<Type>::Destruct(Type* aStart, uint aCount)
	{
		(void)aStart;
		for (int i = aCount - 1; i >= 0; --i)
			(aStart + i)->~Type();
	}
}

