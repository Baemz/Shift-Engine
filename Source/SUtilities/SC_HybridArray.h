#pragma once
#include "SC_GrowingArray.h"
#include "SC_Aligned.h"

namespace Shift
{
	template<class Type, int StaticSize>
	class SC_HybridArray : public SC_GrowingArray<Type>
	{
		using Super = SC_GrowingArray<Type>;

	public:
		SC_HybridArray()
		{
			this->myData = (Type*)myStaticData;
			this->myCount = 0;
			this->myAllocatedCount = StaticSize;
			this->myIsHybrid = 1;
		}

		SC_HybridArray(const SC_HybridArray& aOther)
		{
			this->myData = (Type*)myStaticData;
			this->myCount = 0;
			this->myAllocatedCount = StaticSize;
			this->myIsHybrid = 1;

			Super::Add(aOther.myData, aOther.myCount);
		}

		SC_HybridArray(SC_HybridArray&& aOther)
		{
			this->myCount = aOther.myCount;
			this->myAllocatedCount = aOther.myAllocatedCount;
			this->myIsHybrid = 1;

			if (aOther.IsHybridBuffer(aOther.myData))
			{
				this->myData = (Type*)myStaticData;
				SC_RelocateN(this->myData, aOther.myData, aOther.myCount);
			}
			else
			{
				this->myData = aOther.myData;
				aOther.myData = (Type*)aOther.myStaticData;
				aOther.myAllocatedCount = StaticSize;
			}
			aOther.myCount = 0;
		}

		~SC_HybridArray()
		{
			this->Reset();
			this->myData = nullptr;
		}

		SC_HybridArray& operator=(const SC_HybridArray& aOther)
		{
			SC_GrowingArray<Type>::operator=(aOther);
			return *this;
		}

		SC_HybridArray& operator=(const SC_GrowingArray<Type>& aOther)
		{
			SC_GrowingArray<Type>::operator=(aOther);
			return *this;
		}

		bool operator==(const SC_GrowingArray<Type>& aOther) const
		{
			return SC_GrowingArray<Type>::operator==(aOther);
		}

		bool operator!=(const SC_GrowingArray<Type>& aOther) const
		{
			return SC_GrowingArray<Type>::operator!=(aOther);
		}

		uint GetHybridBufferSize() override
		{
			return StaticSize;
		}
		Type* GetHybridBuffer() override
		{
			return myStaticData[0].Ptr();
		}

	protected:
		SC_AlignedData<Type> myStaticData[StaticSize];
	};

	template<class Type, int StaticSize>
	struct SC_AllowMemcpyRelocation<SC_HybridArray<Type, StaticSize>>
	{
		static const bool value = false;
	};
}
