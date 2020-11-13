#pragma once
#include "SC_Hash.h"

namespace Shift
{
	template<typename Type1, typename Type2>
	class SC_Pair
	{
	public:
		SC_Pair()
		{
		}

		SC_Pair(const Type1& aFirst, const Type2& aSecond)
			: myFirst(aFirst), mySecond(aSecond)
		{
		}

		SC_Pair(Type1&& aFirst, Type2&& aSecond)
			: myFirst(SC_Move(aFirst)), mySecond(SC_Move(aSecond))
		{
		}

		template<typename OtherType1, typename OtherType2>
		SC_Pair(const SC_Pair<OtherType1, OtherType2>& aPair)
			: myFirst(static_cast<Type1>(aPair.myFirst)), mySecond(static_cast<Type2>(aPair.mySecond))
		{
		}

		SC_Pair(const SC_Pair<Type1, Type2>& aPair)
			: myFirst(aPair.myFirst), mySecond(aPair.mySecond)
		{
		}

		SC_Pair<Type1, Type2>& operator=(const SC_Pair<Type1, Type2>& aPair)
		{
			myFirst = aPair.myFirst;
			mySecond = aPair.mySecond;
			return *this;
		}

		SC_Pair(SC_Pair<Type1, Type2>&& aPair) noexcept
			: myFirst(SC_Move(aPair.myFirst)), mySecond(SC_Move(aPair.mySecond))
		{
		}

		SC_Pair<Type1, Type2>& operator=(SC_Pair<Type1, Type2>&& aPair) noexcept
		{
			myFirst = SC_Move(aPair.myFirst);
			mySecond = SC_Move(aPair.mySecond);
			return *this;
		}

		bool operator==(const SC_Pair& aPair) const
		{
			return myFirst == aPair.myFirst && mySecond == aPair.mySecond;
		}

		bool operator!=(const SC_Pair& aPair) const
		{
			return !(*this == aPair);
		}

		bool operator<(const SC_Pair& aPair) const
		{
			return myFirst != aPair.myFirst ? myFirst < aPair.myFirst : mySecond < aPair.mySecond;
		}

		bool operator>(const SC_Pair& aPair) const
		{
			return myFirst != aPair.myFirst ? myFirst > aPair.myFirst : mySecond > aPair.mySecond;
		}

		Type1 myFirst;
		Type2 mySecond;
	};

	template<typename Type1, typename Type2> 
	SC_Pair<Type1, Type2> SC_MakePair(const Type1& aFirst, const Type2& aSecond)
	{
		return SC_Pair<Type1, Type2>(aFirst, aSecond);
	}

	template<typename Type1, typename Type2>
	struct SC_AllowMemcpyRelocation<SC_Pair<Type1, Type2>>
	{
		static const bool value = SC_AllowMemcpyRelocation<Type1>::value && SC_AllowMemcpyRelocation<Type2>::value;
	};

	template<typename Type1, typename Type2>
	inline uint SC_Hash(const SC_Pair<Type1, Type2>& anItem)
	{
		uint hash = SC_Hash(anItem.myFirst);
		hash ^= SC_Hash(anItem.mySecond);
		return hash;
	}
}