#pragma once
#include "SC_Relocate.h"
#include "SC_Aligned.h"

namespace Shift
{
	template<class Type>
	class SC_DynamicCircularArray
	{
	public:
		SC_DynamicCircularArray()
			: myData(nullptr)
			, myCapacity(1)
			, myRead(0)
			, myWrite(0)
		{

		}

		SC_DynamicCircularArray(SC_DynamicCircularArray&& aOther)
			: myData(aOther.myData)
			, myCapacity(aOther.myCapacity)
			, myRead(aOther.myRead)
			, myWrite(aOther.myWrite)
		{
			aOther.myData = nullptr;
			aOther.myCapacity = 0;
			aOther.myRead = 0;
			aOther.myWrite = 0;
		}

		~SC_DynamicCircularArray()
		{
			RemoveAll();
			delete[](char*)myData;
		}

		Type& Add()
		{
			uint write = myWrite;
			uint capacity = myCapacity;
			uint next = write + 1;
			next = (next == capacity) ? 0 : next;

			if (next == myRead)
			{
				uint newCapacity = (capacity == 1) ? 32 : capacity * 2;
				PreAllocSize(newCapacity);

				write = myWrite;
				next = write + 1;
				next = (next == newCapacity) ? 0 : next;
			}

			new (static_cast<void*>(myData + write)) Type();
			myWrite = next;

			return *(myData + write);
		}

		Type& Add(const Type& aItem)
		{
			uint write = myWrite;
			uint capacity = myCapacity;
			uint next = write + 1;
			next = (next == capacity) ? 0 : next;

			if (next == myRead)
			{
				uint newCapacity = (capacity == 1) ? 32 : capacity * 2;
				PreAllocate(newCapacity);

				write = myWrite;
				next = write + 1;
				next = (next == newCapacity) ? 0 : next;
			}

			new (static_cast<void*>(myData + write)) Type(aItem);
			myWrite = next;

			return *(myData + write);
		}

		void Remove()
		{
			uint read = myRead;
			SC_ASSERT(read != myWrite);

			(myData + read)->~Type();
			++read;
			read = (read == myCapacity) ? 0 : read;
			myRead = read;
		}

		void RemoveAll()
		{
			while (myRead != myWrite)
				Remove();
		}

		void PreAllocate(uint aCapacity)
		{
			if (aCapacity < myCapacity)
				return;

			Type* data = (Type*) new char[aCapacity * sizeof(Type)];
			uint read = myRead;
			uint write = myWrite;
			uint count;
			if (read < write)
			{
				count = write - read;
				SC_RelocateN(data, myData + read, write - read);
			}
			else if (write < read)
			{
				uint end = myCapacity - read;
				count = end + write;
				SC_ASSERT(count == Count());
				SC_RelocateN(data, myData + read, end);
				SC_RelocateN(data + end, myData, write);

			}
			else
				count = 0;

			delete[] (char*)myData;
			myData = data;
			myCapacity = aCapacity;
			myRead = 0;
			myWrite = count;
		}

		void Clear() { myRead = 0; myWrite = 0; }
		uint Count() const { return (myWrite + myCapacity - myRead) % myCapacity; }
		uint GetCapacity() const { return myCapacity != 1 ? myCapacity : 0; }
		bool Empty() const { return myRead == myWrite; }

		Type& Peek() const { SC_ASSERT(myRead != myWrite); return myData[myRead]; }
		Type& operator[](uint aIndex) { return myData[(myRead + aIndex) % myCapacity]; }
		const Type& operator[](uint aIndex) const { return myData[(myRead + aIndex) % myCapacity]; }

	private:
		Type* myData;
		uint myCapacity;
		uint myRead;
		uint myWrite;


		SC_DynamicCircularArray(const SC_DynamicCircularArray& aOther);
		void operator=(const SC_DynamicCircularArray& aOther);
	};

	template<class Type>
	struct SC_AllowMemcpyRelocation<SC_DynamicCircularArray<Type>>
	{
		static const bool value = true;
	};

}