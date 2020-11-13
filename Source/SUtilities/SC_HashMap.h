#pragma once

#pragma warning(disable : 6385)		// C6385: Reading invalid data from '_variable_', the readable size is '_x_' bytes, but '_x_' bytes may be read.

#include "SC_Hash.h"
#include "SC_Relocate.h"
#include "SC_Aligned.h"
#include <string>

#if ENABLE_ASSERTS && !IS_FINAL_BUILD
#define ENABLE_HASH_MAP_RACE_DETECTION 1
#else
#define ENABLE_HASH_MAP_RACE_DETECTION 0
#endif

#if ENABLE_HASH_MAP_RACE_DETECTION
#define HASH_MAP_RACE_DETECT(x) x
#else
#define HASH_MAP_RACE_DETECT(x)
#endif

namespace Shift
{

	////////////////////////////////////
	// KeyData
	template<class T>
	struct SC_HashMapKeyData
	{
		using Type = T;
		using ParamType = const T&;
		using ReturnType = const T&;
	};

	template<>
	struct SC_HashMapKeyData<std::string>
	{
		using Type = std::string;
		using ParamType = const char*;
		using ReturnType = const std::string&;
	};

	template<>
	struct SC_HashMapKeyData<const char*>
	{
		using Type = std::string;
		using ParamType = const char*;
		using ReturnType = const char*;
	};
	////////////////////////////////////

	////////////////////////////////////
	// NodeData
	template<typename T, bool IsAligned>
	struct SC_HashMapNodeData;

	template<typename T>
	struct SC_HashMapNodeData<T, true> : public SC_AlignedData<T> {};

	template<typename T>
	struct SC_HashMapNodeData<T, false>
	{
		char myBuffer[sizeof(T)];

		T* Ptr() { return reinterpret_cast<T*>(&this->myBuffer[0]); }
		const T* Ptr() const { return reinterpret_cast<const T*>(&this->myBuffer[0]); }
	};
	////////////////////////////////////

	template<class Key, class Value, bool Aligned = true>
	class SC_HashMap
	{
		using KeyDataType = typename SC_HashMapKeyData<Key>::Type;
		using KeyParamType = typename SC_HashMapKeyData<Key>::ParamType;
		using KeyReturnType = typename SC_HashMapKeyData<Key>::ReturnType;

		struct Node
		{
			void Set(KeyParamType aKey, const Value& anItem)
			{
				new (static_cast<void*>(&myKey.myBuffer[0])) KeyDataType(aKey);
				new (static_cast<void*>(&myItem.myBuffer[0])) Value(anItem);
			}

			void Set(KeyParamType aKey)
			{
				new (static_cast<void*>(&myKey.myBuffer[0])) KeyDataType(aKey);
				new (static_cast<void*>(&myItem.myBuffer[0])) Value();
			}

			void Set(KeyParamType aKey, Value&& anItem)
			{
				new (static_cast<void*>(&myKey.myBuffer[0])) KeyDataType(aKey);
				new (static_cast<void*>(&myItem.myBuffer[0])) Value(SC_Move(anItem));
			}

			void Set(KeyDataType&& aKey)
			{
				new (static_cast<void*>(&myKey.myBuffer[0])) KeyDataType(SC_Move(aKey));
				new (static_cast<void*>(&myItem.myBuffer[0])) Value();
			}

			void Clear()
			{
				myKey.Ptr()->~KeyDataType();
				myItem.Ptr()->~Value();
			}

			void Relocate(Node& aDest)
			{
				SC_Relocate(aDest.myKey.Ptr(), myKey.Ptr());
				SC_Relocate(aDest.myItem.Ptr(), myItem.Ptr());
			}

			SC_HashMapNodeData<KeyDataType, Aligned> myKey;
			SC_HashMapNodeData<Value, Aligned> myItem;
		};

	public:

		class Iterator
		{
		public:
			Iterator() : myMap(0), myIndex(0) {}
			Iterator(const SC_HashMap& aMap) : myMap(&aMap), myIndex(0) {}
			Iterator(const SC_HashMap& aMap, uint anIndex) : myMap(&aMap), myIndex(anIndex) {}
			Iterator(const Iterator& aRhs) : myMap(aRhs.myMap), myIndex(aRhs.myIndex) {}

			bool operator==(const Iterator& anOther) const { return myMap == anOther.myMap && myIndex == anOther.myIndex; }
			bool operator!=(const Iterator& anOther) const { return myMap != anOther.myMap || myIndex != anOther.myIndex; }

			Iterator& operator+=(uint aRhs)						{ this->myIndex += aRhs; return *this; }
			Iterator& operator-=(uint aRhs)						{ this->myIndex -= aRhs; return *this; }
			friend Iterator operator+(Iterator aLhs, uint aRhs) { aLhs += aRhs; return aLhs; }
			friend Iterator operator-(Iterator aLhs, uint aRhs) { aLhs -= aRhs; return aLhs; }
			friend uint operator-(Iterator aLhs, Iterator aRhs) { SC_ASSERT(aLhs.myMap == aRhs.myMap); return aLhs.myIndex - aRhs.myIndex; }
			bool operator >(const Iterator& anOther) const		{ SC_ASSERT(anOther.myMap == myMap); return myIndex > anOther.myIndex; }
			bool operator <(const Iterator& anOther) const		{ SC_ASSERT(anOther.myMap == myMap); return myIndex < anOther.myIndex; }

			operator bool() const				{ return myMap && (myIndex < myMap->myNumNodes); }
			KeyReturnType	Key() const			{ return *myMap->myNodes[myIndex].myKey.Ptr(); }
			KeyDataType* KeyPtr() const			{ return myMap->myNodes[myIndex].myKey.Ptr(); }
			Value& Item()						{ return *myMap->myNodes[myIndex].myItem.Ptr(); }
			const Value& Item() const			{ return *myMap->myNodes[myIndex].myItem.Ptr(); }
			Value* operator ->()				{ return myMap->myNodes[myIndex].myItem.Ptr(); }
			const Value* operator ->() const	{ return myMap->myNodes[myIndex].myItem.Ptr(); }

			Value& operator*()				{ return *myMap->myNodes[myIndex].myItem.Ptr(); }
			const Value& operator*() const	{ return *myMap->myNodes[myIndex].myItem.Ptr(); }

			void operator++()		{ ++myIndex; }	// prefix
			void operator++(int)	{ ++myIndex; }	// postfix
			void operator--()		{ --myIndex; }	// prefix
			void operator--(int)	{ --myIndex; }	// postfix

		private:
			friend class SC_HashMap;
			const SC_HashMap* myMap;
			uint myIndex;
		};

		SC_FORCEINLINE Iterator Begin() const		{ return Iterator(*this); }
		SC_FORCEINLINE Iterator End() const			{ return Iterator(*this, myNumNodes); }
		SC_FORCEINLINE Iterator begin()	const		{ return Begin(); };
		SC_FORCEINLINE Iterator end() const			{ return End(); };

		explicit SC_HashMap(uint aNodeCount = 0)
		{
			if (aNodeCount)
			{
				myMaxNodes = aNodeCount;
				const uint bucketArraySize = aNodeCount * 2 - 1;

				char* buf = new char[myMaxNodes * (sizeof(Node) + sizeof(uint)) + bucketArraySize * sizeof(uint)];
				myNodes = reinterpret_cast<Node*>(buf);
				uint* bucketArray = reinterpret_cast<uint*>(buf + myMaxNodes * (sizeof(Node) + sizeof(uint)));

				memset(bucketArray, 0xFF, bucketArraySize * sizeof(uint));
			}
			else
			{
				myMaxNodes = 0;
				myNodes = nullptr;
			}

			myNumNodes = 0;

			HASH_MAP_RACE_DETECT(myChangeCounter = 0);
		}

		SC_HashMap(const SC_HashMap& aOther)
		{
			HASH_MAP_RACE_DETECT(CounterCheckRead check(aOther.myChangeCounter));

			myNodes = nullptr;
			myNumNodes = 0;
			myMaxNodes = 0;

			*this = aOther;
			HASH_MAP_RACE_DETECT(myChangeCounter = 0);
		}

		SC_HashMap(SC_HashMap&& aOther)
		{
			HASH_MAP_RACE_DETECT(CounterCheckWrite check(aOther.myChangeCounter));

			myNodes = aOther.myNodes;
			myNumNodes = aOther.myNumNodes;
			myMaxNodes = aOther.myMaxNodes;

			aOther.myNodes = nullptr;
			aOther.myNumNodes = 0;
			aOther.myMaxNodes = 0;

			HASH_MAP_RACE_DETECT(myChangeCounter = 0);
		}
		
		~SC_HashMap()
		{
			HASH_MAP_RACE_DETECT(CounterCheckWrite check(myChangeCounter));

			for (uint i = 0; i < myNumNodes; ++i)
				myNodes[i].Clear();

			delete[] reinterpret_cast<char*>(myNodes);
		}

		void Reset()
		{
			HASH_MAP_RACE_DETECT(CounterCheckWrite check(myChangeCounter));

			for (uint i = 0; i < myNumNodes; ++i)
				myNodes[i].Clear();

			delete[] reinterpret_cast<char*>(myNodes);

			myNodes = nullptr;
			myNumNodes = 0;
			myMaxNodes = 0;
		}

		Value& operator[](KeyParamType aKey)
		{
			uint keyHash = SC_Hash(aKey);
			uint bucketIndex = 0;
			Value* item = FindBucketItem(aKey, keyHash, bucketIndex);
			if (item)
				return *item;

			HASH_MAP_RACE_DETECT(CounterCheckWrite check(myChangeCounter));
			GrowIfNeeded(keyHash, bucketIndex);

			const uint nodeIndex = myNumNodes++;
			myNodes[nodeIndex].Set(aKey);
			uint* bucketArray = BucketArray();
			uint* next = NextArray();
			next[nodeIndex] = bucketArray[bucketIndex];
			bucketArray[bucketIndex] = nodeIndex;


			return *myNodes[nodeIndex].myItem.Ptr();
		}

		Value& operator[](KeyDataType&& aKey)
		{
			uint keyHash = SC_Hash(aKey);
			uint bucketIndex = 0;
			Value* item = FindBucketItem(aKey, keyHash, bucketIndex);
			if (item)
				return *item;

			HASH_MAP_RACE_DETECT(CounterCheckWrite check(myChangeCounter));
			GrowIfNeeded(keyHash, bucketIndex);

			const uint nodeIndex = myNumNodes++;
			myNodes[nodeIndex].Set(SC_Move(aKey));
			uint* bucketArray = BucketArray();
			uint* next = NextArray();
			next[nodeIndex] = bucketArray[bucketIndex];
			bucketArray[bucketIndex] = nodeIndex;

			return *myNodes[nodeIndex].myItem.Ptr();
		}

		Value& Insert(KeyParamType aKey, const Value& aItem)
		{
			uint keyHash = SC_Hash(aKey);
			uint bucketIndex = 0;
			Value* item = FindBucketItem(aKey, keyHash, bucketIndex);
			if (item)
			{
				*item = aItem;
				return *item;
			}

			HASH_MAP_RACE_DETECT(CounterCheckWrite check(myChangeCounter));
			GrowIfNeeded(keyHash, bucketIndex);

			const uint nodeIndex = myNumNodes++;
			myNodes[nodeIndex].Set(aKey, aItem);
			uint* bucketArray = BucketArray();
			uint* next = NextArray();
			next[nodeIndex] = bucketArray[bucketIndex];
			bucketArray[bucketIndex] = nodeIndex;

			return *myNodes[nodeIndex].myItem.Ptr();
		}

		Value& Insert(KeyParamType aKey, Value&& aItem)
		{
			uint keyHash = SC_Hash(aKey);
			uint bucketIndex = 0;
			Value* item = FindBucketItem(aKey, keyHash, bucketIndex);
			if (item)
			{
				*item = SC_Move(aItem);
				return *item;
			}

			HASH_MAP_RACE_DETECT(CounterCheckWrite check(myChangeCounter));
			GrowIfNeeded(keyHash, bucketIndex);

			const uint nodeIndex = myNumNodes++;
			myNodes[nodeIndex].Set(aKey, SC_Move(aItem));
			uint* bucketArray = BucketArray();
			uint* next = NextArray();
			next[nodeIndex] = bucketArray[bucketIndex];
			bucketArray[bucketIndex] = nodeIndex;

			return *myNodes[nodeIndex].myItem.Ptr();
		}

		bool RemoveByKey(KeyParamType aKey)
		{
			HASH_MAP_RACE_DETECT(CounterCheckWrite check(myChangeCounter));

			if (!myMaxNodes)
				return false;

			uint bucketIndex = GetBucketIndex(SC_Hash(aKey));
			uint prevnodeIndex = 0xFFFFFFFF;
			uint* next = NextArray();
			uint* bucketArray = BucketArray();
			uint nodeIndex = bucketArray[bucketIndex];
			while (nodeIndex < myNumNodes)
			{
				if (SC_Compare(aKey, *myNodes[nodeIndex].myKey.Ptr()))
				{
					if (prevnodeIndex < myNumNodes)
						next[prevnodeIndex] = next[nodeIndex];
					else
						bucketArray[bucketIndex] = next[nodeIndex];

					RemoveAtIndexInternal(nodeIndex);
					return true;
				}
				prevnodeIndex = nodeIndex;
				nodeIndex = next[nodeIndex];
			}
			return false;
		}

		void RemoveAtIndex(uint anIndex)
		{
			HASH_MAP_RACE_DETECT(CounterCheckWrite check(myChangeCounter));

			const uint bucketIndex = GetBucketIndex(SC_Hash(*myNodes[anIndex].myKey.Ptr()));
			uint* next = NextArray();
			uint* bucketArray = BucketArray();

			uint i = bucketArray[bucketIndex];
			if (i == anIndex)
				bucketArray[bucketIndex] = next[anIndex];
			else
			{
				for (; i < myNumNodes; i = next[i])
				{
					if (next[i] == anIndex)
					{
						next[i] = next[anIndex];
						break;
					}
				}
			}
			RemoveAtIndexInternal(anIndex);
		}

		bool Find(KeyParamType aKey, Value& anItem) const
		{
			HASH_MAP_RACE_DETECT(CounterCheckRead check(myChangeCounter));

			int nodeIndex = GetIndexWithKey(aKey);
			if (nodeIndex != -1)
			{
				anItem = *myNodes[nodeIndex].myItem.Ptr();
				return true;
			}
			return false;
		}

		Value* Find(KeyParamType aKey)
		{
			HASH_MAP_RACE_DETECT(CounterCheckRead check(myChangeCounter));

			int nodeIndex = GetIndexWithKey(aKey);
			if (nodeIndex != -1)
				return myNodes[nodeIndex].myItem.Ptr();
			return nullptr;
		}

		const Value* Find(KeyParamType aKey) const
		{
			HASH_MAP_RACE_DETECT(CounterCheckRead check(myChangeCounter));

			int nodeIndex = GetIndexWithKey(aKey);
			if (nodeIndex != -1)
				return myNodes[nodeIndex].myItem.Ptr();
			return nullptr;
		}

		int GetIndexWithKey(KeyParamType aKey) const
		{
			HASH_MAP_RACE_DETECT(CounterCheckRead check(myChangeCounter));

			if (myNumNodes)
			{
				uint bucketIndex = GetBucketIndex(SC_Hash(aKey));
				uint* next = NextArray();
				for (uint entryIndex = BucketArray()[bucketIndex]; entryIndex < myNumNodes; entryIndex = next[entryIndex])
				{
					if (SC_Compare(aKey, *myNodes[entryIndex].myKey.Ptr()))
						return entryIndex;
				}
			}
			return -1;
		}

		SC_HashMap& operator=(const SC_HashMap& aOther)
		{
			if (this != &aOther)
			{
				HASH_MAP_RACE_DETECT(CounterCheckWrite check(myChangeCounter));
				HASH_MAP_RACE_DETECT(CounterCheckRead check2(aOther.myChangeCounter));

				for (uint i = 0; i < myNumNodes; ++i)
				{
					myNodes[i].Clear();
				}

				if (myMaxNodes != aOther.myMaxNodes)
				{
					myMaxNodes = aOther.myMaxNodes;

					delete[] reinterpret_cast<char*>(myNodes);

					if (myMaxNodes)
					{
						char* buf = new char[myMaxNodes * (sizeof(Node) + sizeof(uint)) + BucketArraySize() * sizeof(uint)];
						myNodes = reinterpret_cast<Node*>(buf);
					}
					else
					{
						myNodes = nullptr;
						myNumNodes = 0;
					}
				}

				if (myMaxNodes)
				{
					myNumNodes = aOther.myNumNodes;

					SC_Memcpy(NextArray(), aOther.NextArray(), myMaxNodes * sizeof(uint));
					SC_Memcpy(BucketArray(), aOther.BucketArray(), BucketArraySize() * sizeof(uint));

					for (uint i = 0; i < myNumNodes; ++i)
						myNodes[i].Set(*aOther.myNodes[i].myKey.Ptr(), *aOther.myNodes[i].myItem.Ptr());
				}
			}
			return *this;
		}

		SC_HashMap& operator=(SC_HashMap&& aOther)
		{
			if (this != &aOther)
			{
				for (uint i = 0; i < myNumNodes; ++i)
					myNodes[i].Clear();

				delete[] reinterpret_cast<char*>(myNodes);

				myNodes = aOther.myNodes;
				myNumNodes = aOther.myNumNodes;
				myMaxNodes = aOther.myMaxNodes;

				aOther.myNodes = nullptr;
				aOther.myNumNodes = 0;
				aOther.myMaxNodes = 0;
			}
			return *this;
		}

		uint Count() const { return myNumNodes; }

	private:
		Value* FindBucketItem(KeyParamType aKey, uint aKeyHash, uint& aBucketIndexOut)
		{
			HASH_MAP_RACE_DETECT(CounterCheckRead check(myChangeCounter));

			if (myMaxNodes)
			{
				aBucketIndexOut = GetBucketIndex(aKeyHash);
				uint* next = NextArray();
				for (uint nodeIndex = BucketArray()[aBucketIndexOut]; nodeIndex < myNumNodes; nodeIndex = next[nodeIndex])
				{
					if (SC_Compare(aKey, *myNodes[nodeIndex].myKey.Ptr()))
						return myNodes[nodeIndex].myItem.Ptr();
				}
			}
			else
				aBucketIndexOut = 0;

			return nullptr;
		}

		void GrowIfNeeded(uint aKeyHash, uint& aOutBucketIndex)
		{
			const uint newSize = CalcNewArraySize();
			if (newSize > myMaxNodes)
			{
				ResizeBucketsInternal(newSize);
				aOutBucketIndex = GetBucketIndex(aKeyHash);
			}
		}

		uint CalcNewArraySize() const
		{
			return (myNumNodes >= myMaxNodes) ? (myMaxNodes * 2 + 1) : myMaxNodes;
		}

		uint* BucketArray() const
		{
			return reinterpret_cast<uint*>(reinterpret_cast<char*>(myNodes) + myMaxNodes * (sizeof(Node) + sizeof(uint)));
		}
		
		uint BucketArraySize() const
		{
			return myMaxNodes * 2 - 1;
		}

		uint* NextArray() const
		{
			return reinterpret_cast<uint*>(myNodes + myMaxNodes);
		}

		inline uint GetBucketIndex(uint aHash, uint aBucketArraySize) const
		{
			return aHash % aBucketArraySize;
		}

		inline uint GetBucketIndex(uint aHash) const
		{
			return GetBucketIndex(aHash, BucketArraySize());
		}

		void ResizeBucketsInternal(uint aNewMaxNodes)
		{
			const uint newMaxEntries = aNewMaxNodes;
			const uint aNewBucketArraySize = aNewMaxNodes * 2 - 1;

			char* buf = new char[newMaxEntries * (sizeof(Node) + sizeof(uint)) + aNewBucketArraySize * sizeof(uint)];
			Node* newEntries = reinterpret_cast<Node*>(buf);
			uint* newNextArray = reinterpret_cast<uint*>(buf + newMaxEntries * sizeof(Node));
			uint* newBucketArray = reinterpret_cast<uint*>(buf + newMaxEntries * (sizeof(Node) + sizeof(uint)));

			memset(newBucketArray, 0xFF, aNewBucketArraySize * sizeof(uint));

			for (uint readIndex = 0; readIndex < myNumNodes; ++readIndex)
			{
				const uint newBucketIndex = GetBucketIndex(SC_Hash(*myNodes[readIndex].myKey.Ptr()), aNewBucketArraySize);

				myNodes[readIndex].Relocate(newEntries[readIndex]);
				newNextArray[readIndex] = newBucketArray[newBucketIndex];
				newBucketArray[newBucketIndex] = readIndex;
			}

			delete[] reinterpret_cast<char*>(myNodes);

			myNodes = newEntries;
			myMaxNodes = newMaxEntries;
		}

		void RemoveAtIndexInternal(uint anIndex)
		{
			const uint lastNode = myNumNodes - 1;
			if (lastNode != anIndex)
			{
				// Fill in the gap and update "next" pointers
				const uint bucketIndex = GetBucketIndex(SC_Hash(*myNodes[lastNode].myKey.Ptr()));
				uint* next = NextArray();
				uint* bucketArray = BucketArray();
				bool lastNodeListUpdated = false;
				if (bucketArray[bucketIndex] == lastNode)
				{
					bucketArray[bucketIndex] = anIndex;
					lastNodeListUpdated = true;
				}
				else
				{
					for (uint i = bucketArray[bucketIndex]; i < myNumNodes; i = next[i])
					{
						if (next[i] == lastNode)
						{
							next[i] = anIndex;
							lastNodeListUpdated = true;
							break;
						}
					}
				}
				SC_ASSERT(lastNodeListUpdated, "Couldn't find the last node.");
				myNodes[anIndex].Clear();
				myNodes[lastNode].Relocate(myNodes[anIndex]);
				next[anIndex] = next[lastNode];
			}
			else
			{
				myNodes[lastNode].Clear();
			}
			--myNumNodes;
		}

		Node* myNodes;

		uint myNumNodes;
		uint myMaxNodes;


#if ENABLE_HASH_MAP_RACE_DETECTION
		volatile int myChangeCounter;

		struct CounterCheckRead
		{
			CounterCheckRead(volatile const int& aCounter) : myCounter(aCounter), myCounterOriginalValue(aCounter) {}
			~CounterCheckRead() { SC_ASSERT(myCounter == myCounterOriginalValue); }

			volatile const int& myCounter;
			const int myCounterOriginalValue;
		};

		struct CounterCheckWrite
		{
			CounterCheckWrite(volatile int& aCounter) : myCounter(aCounter), myCounterOriginalValue(SC_Atomic_Increment_GetNew(aCounter)) {}
			~CounterCheckWrite() { SC_ASSERT(SC_Atomic_Increment_GetNew(myCounter) == myCounterOriginalValue + 1); }

			volatile int& myCounter;
			const int myCounterOriginalValue;
		};
#endif

	};

	template<class Key, class Value>
	struct SC_AllowMemcpyRelocation<SC_HashMap<Key, Value>>
	{
		static const bool value = true;
	};
}