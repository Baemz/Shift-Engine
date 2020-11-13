#pragma once

namespace Shift
{
	template <class Type, uint StaticSize>
	class SC_Array : public SC_Container<Type>
	{
		using iterator = Type * ;
		using const_iterator = const Type*;

	public:
		SC_Array();
		~SC_Array();
		Type* GetBuffer() const { return myData; }

		Type* Begin() const override { return (myData) ? &myData[0] : nullptr; }
		Type* End() const override { return (myData) ? &myData[this->myCount] : nullptr; }

		int Find(const Type& aItem, uint aLookFromIndex = 0) override;

		__forceinline iterator begin() { return myData; };
		__forceinline const_iterator begin() const { return myData; };
		__forceinline iterator end() { return &myData[this->myCount]; };
		__forceinline const_iterator end() const { return &myData[this->myCount]; };

		Type& Insert(const Type& aItem, uint aIndex) override;

		Type& operator[](uint aIndex) override;
		const Type& operator[](uint aIndex) const override;

	private:
		Type* myData;
	};
	template<class Type, uint StaticSize>
	inline SC_Array<Type, StaticSize>::SC_Array()
		: SC_Container<Type>()
	{
		this->myAllocatedCount = StaticSize;
		myData = new Type[StaticSize];
	}
	template<class Type, uint StaticSize>
	inline SC_Array<Type, StaticSize>::~SC_Array()
	{
		delete[] myData;
	}
	template<class Type, uint StaticSize>
	inline int SC_Array<Type, StaticSize>::Find(const Type & aItem, uint aLookFromIndex)
	{
		SC_ASSERT((aLookFromIndex < this->myAllocatedCount), "Index out of bounds!");
		for (uint i = 0; i < this->myAllocatedCount; ++i)
		{
			if (myData[i] == aItem)
				return i;
		}

		return this->FoundNone;
	}
	template<class Type, uint StaticSize>
	inline Type & SC_Array<Type, StaticSize>::Insert(const Type & aItem, uint aIndex)
	{
		SC_ASSERT((aIndex < this->myAllocatedCount), "Index out of bounds!");
		myData[aIndex] = aItem;
		return myData[aIndex];
	}
	template<class Type, uint StaticSize>
	inline Type & SC_Array<Type, StaticSize>::operator[](uint aIndex)
	{
		SC_ASSERT((aIndex < this->myAllocatedCount), "Index out of bounds!");
		return myData[aIndex];
	}
	template<class Type, uint StaticSize>
	inline const Type & SC_Array<Type, StaticSize>::operator[](uint aIndex) const
	{
		SC_ASSERT((aIndex < this->myAllocatedCount), "Index out of bounds!");
		return myData[aIndex];
	}
}