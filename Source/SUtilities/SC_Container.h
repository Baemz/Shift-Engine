#pragma once

using uint = unsigned int;

namespace Shift
{
	template<class Type>
	class SC_Container
	{
	public:
		SC_Container();
		virtual ~SC_Container();

		uint Count() const { return myCount; }
		uint GetByteSize() const { return sizeof(Type)* myCount; }
		bool Empty() const { return (myCount == 0); }

		virtual Type* Begin() const = 0;
		virtual Type* End() const = 0;

		virtual int Find(const Type& aItem, uint aLookFromIndex = 0) = 0;

		virtual Type& Add() { return myTrashValue; };
		virtual Type& Add(const Type& aItem) { (void)aItem; return myTrashValue; };
		virtual Type& Add(Type&& aItem) { (void)(aItem); return myTrashValue; };
		virtual void AddN(Type* aList, uint aCount) { (void)(aList); (void)(aCount); return; };
		virtual Type& Insert(const Type& aItem, uint aIndex) { (void)(aItem); (void)(aIndex); return myTrashValue; };

		virtual void Remove(const Type& aItem) { (void)(aItem); return; };
		virtual void RemoveCyclic(const Type& aItem) { (void)(aItem); return; };
		virtual void RemoveByIndex(uint aIndex) { (void)(aIndex); return; };
		virtual void RemoveByIndexCyclic(uint aIndex) { (void)(aIndex); return; };
		virtual void RemoveAll() { return; };
		virtual void RemoveLast() { return; };

		virtual Type& operator[](uint aIndex) = 0;
		virtual const Type& operator[](uint aIndex) const = 0;

		static constexpr int FoundNone = -1;

	protected:
		Type myTrashValue;
		uint myCount;
		uint myAllocatedCount;
	};
	template<class Type>
	inline SC_Container<Type>::SC_Container()
		: myCount(0)
		, myAllocatedCount(0)
	{
	}
	template<class Type>
	inline SC_Container<Type>::~SC_Container()
	{
	}
}