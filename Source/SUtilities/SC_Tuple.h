#pragma once
#include "SC_CommonFuncs.h"

namespace Shift
{
	template<typename... Args>
	struct SC_Tuple {};

	template<typename Type, typename... OtherTypes>
	class SC_Tuple<Type, OtherTypes...> : public SC_Tuple<OtherTypes...>
	{
	public:
		SC_Tuple(Type aValue, OtherTypes... someValues)
			: SC_Tuple<OtherTypes...>(SC_Forward<OtherTypes>(someValues)...)
			, myValue(SC_Forward<Type>(aValue))
		{ }

		Type myValue;
	};
}