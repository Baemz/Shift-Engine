#pragma once
#include <type_traits>

namespace SC_DetectionIdiom 
{
	struct SC_NoneSuch 
	{
		~SC_NoneSuch() = delete;
		SC_NoneSuch(SC_NoneSuch const&) = delete;
		void operator=(SC_NoneSuch const&) = delete;
	};

	template <class Default, class AlwaysVoid, template<class...> class Op, class... Args>
	struct SC_Detector 
	{
		using value_t = std::false_type;
		using type = Default;
	};

	template <class Default, template<class...> class Op, class... Args>
	struct SC_Detector<Default, std::void_t<Op<Args...>>, Op, Args...> 
	{
		using value_t = std::true_type;
		using type = Op<Args...>;
	};

}

template <template<class...> class Op, class... Args>
using SC_IsDetected = typename SC_DetectionIdiom::SC_Detector<SC_DetectionIdiom::SC_NoneSuch, void, Op, Args...>::value_t;