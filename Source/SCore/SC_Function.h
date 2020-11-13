#pragma once
#include "SC_Tuple.h"

namespace Shift
{
	template<typename Signature>
	class SC_Function;

	namespace SC_FunctionInternal
	{

	}

	template<typename ReturnType, typename... Args>
	class SC_Function<ReturnType(Args...)>
	{
	public:
		typedef Return(Signature)(Args...);
		using ArgsTuple = SC_Tuple<Args&&...>;

		typedef ReturnType(*Func)(ArgsTuple&);

		struct Data
		{
			Data() {}
			Data(const Func& aFunction, const SC_FunctionInternal::FunctionAny& aUserFunc, void* aUserData) {}

			SC_FunctionInternal::FunctionVoidData myUserFunc;
			Func myFunction;
			void* myUserData;
		};

		SC_Function() {}
		SC_Function(std::nullptr_t)	{}

		template<typename Func>
		SC_Function(const Func& aFunc)
		{
			Set(aFunc);
		}

		template<typename Obj, typename MemberFunc>
		SC_Function(Obj* aObj, MemberFunc aMemberFunc)
		{
			static_assert(SC_IsMemberFunctionPointer<MemberFunc>::ourValue, "Function is not a member function of Obj");
			Set(aObj, aMemberFunc);
		}

		template<typename Obj1, typename ReturnVal, typename Obj2, typename... FunctionArgs>
		void Set(Obj1* aObject, ReturnVal(Obj2::*aMemberFunction)(FunctionArgs...))
		{
			static_assert(sizeof...(FunctionArgs) <= sizeof...(Args), "The function has more arguments than available");

		}

		template<typename Obj1, typename ReturnVal, typename Obj2, typename... FunctionArgs>
		void Set(Obj1* aObject, ReturnVal(Obj2::* aMemberFunction)(FunctionArgs...) const)
		{
			static_assert(sizeof...(FunctionArgs) <= sizeof...(Args), "The function has more arguments than available");

		}

		template<typename ReturnVal, typename... FunctionArgs>
		void Set(ReturnVal(*aMemberFunction)(FunctionArgs...), void* aUserData = nullptr)
		{
			static_assert(sizeof...(FunctionArgs) <= sizeof...(Args), "The function has more arguments than available");

		}

		template<typename ReturnVal, typename... FunctionArgs>
		void Set(const SC_Function<ReturnVal(FunctionArgs...)>& aFunction, void* aUserData = nullptr)
		{
			static_assert(sizeof...(FunctionArgs) <= sizeof...(Args), "The function has more arguments than available");

		}

		template<typename FuncObj>
		void Set(const FuncObj& aFuncObj, void* aUserData = nullptr)
		{
			// Incorrect argument to 'decltype' - means that you tried to use a function object with an overloaded operator().
			// Wrap in, or use, a lambda instead.

			typedef decltype(&FuncObj::operator()) FuncObjSignature;
			//SetFuncObj(aFuncObj, static_cast<FuncObjSignature>(nullptr), aUserData);
		}

		ReturnType operator()(Args... aArgs)
		{
			//assert(myData.myFunction);

			ArgsTuple args = SC_TupleUtils::ForwardAsTuple(SC_Forward<Args>(aArgs)...);
			return myData.myFunction(myData.myUserData, &myData, args);
		}

		ReturnType CallWithObject(void* aObject, Args... aArgs)
		{
			//assert(myData.myFunction);
			ArgsTuple args = SC_TupleUtils::ForwardAsTuple(SC_Forward<Args>(aArgs)...);
			return myData.myFunction(aObject, &myData, args);
		}

		void Reset()
		{
			this->myData = Data();
		}

		bool IsSet() const
		{
			return this->myData.myFunction != nullptr;
		}

		void* GetUserData() const
		{
			return this->myData.myUserData;
		}

		Data myData;
	private:

		template<typename FuncObj, typename ReturnVal, typename... FunctionArgs>
		void SetFuncObj(const FuncObj& aFuncObj, ReturnVal(*)(FunctionArgs...), void* aUserData)
		{
			static_assert(sizeof...(FunctionArgs) <= sizeof...(Args), "The function has more arguments than available");
		}

		template<typename FuncObj, typename ReturnVal, typename Obj, typename... FunctionArgs>
		void SetFuncObj(const FuncObj& aFuncObj, ReturnVal(Obj::*)(FunctionArgs...), void* aUserData)
		{
			static_assert(sizeof...(FunctionArgs) <= sizeof...(Args), "The function has more arguments than available");
		}
		
		template<typename FuncObj, typename ReturnVal, typename Obj, typename... FunctionArgs>
			void SetFuncObj(const FuncObj& aFuncObj, ReturnVal(Obj::*)(FunctionArgs...) const, void* aUserData)
		{
			static_assert(sizeof...(FunctionArgs) <= sizeof...(Args), "The function has more arguments than available");
		}
	};
}
