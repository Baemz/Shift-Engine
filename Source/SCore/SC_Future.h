#pragma once
#include "SC_Thread.h"
#include "SC_Atomics.h"
#include <functional>

namespace Shift
{
	template<typename FuncReturnType> class SC_Future;

	class SC_Semaphore;
	class SC_FutureBase
	{
		friend class SC_ThreadPool;

	public:
		SC_FutureBase() : myData(nullptr) {}
		~SC_FutureBase() { if (myData) myData->RemoveRef(); }
		SC_FutureBase(const SC_FutureBase& aOther) : myData(aOther.myData) { if (myData) myData->AddRef(); }
		SC_FutureBase(SC_FutureBase&& aOther) noexcept : myData(aOther.myData) { aOther.myData = nullptr; }

		SC_FutureBase& operator=(const SC_FutureBase& aOther)
		{
			DataBase* newData = aOther.myData;
			DataBase* oldData = myData;
			if (newData != oldData)
			{
				myData = newData;

				if (oldData)
					oldData->RemoveRef();
				if (newData)
					newData->AddRef();
			}
			return *this;
		}

		SC_FutureBase& operator=(SC_FutureBase&& aOther) noexcept
		{
			DataBase* newData = aOther.myData;
			DataBase* oldData = myData;
			if (newData != oldData)
			{
				aOther.myData = nullptr;
				myData = newData;

				if (oldData)
					oldData->RemoveRef();
			}
			return *this;
		}

		bool Wait(bool aBlock = true) const;
		void WaitForThread() const;
		bool IsActive() const;
		void Stop(bool aBlock = true) const;
		void Reset() { if (myData) { myData->RemoveRef(); myData = nullptr; } }
	protected:
		struct SC_FutureThread;
		struct DataBase
		{
			enum State
			{
				Started = 0x1,
				Finished = 0x2,
				Locked = 0x4,
			};

			DataBase() : myState(0), myRefCount(2), myThread(nullptr) {}
			virtual ~DataBase();

			virtual void Invoke() = 0;
			void Stop();
			void Finish();

			void AddRef()
			{
				SC_Atomic_Increment(myRefCount);
			}

			void RemoveRef()
			{
				if (SC_Atomic_Decrement_GetNew(myRefCount) == 0)
					delete this;
			}

			SC_GrowingArray<SC_Semaphore*> myWaitingSemaphores;
			SC_FutureThread* myThread;
			SC_ThreadHandle myThreadHandle;
			volatile int myState;
			volatile int myRefCount;

		};

		DataBase* myData;
	private:
		void BeginThread(const SC_ThreadProperties& aProperties);
		void BeginTask(bool aIsLongTask, bool aIsLowPrio, int aThreadIdx);

		template<class FuncReturnType> friend SC_Future<FuncReturnType> SC_CreateFrameTask(const std::function<FuncReturnType()>& aFunction);
		template<class FuncReturnType> friend SC_Future<FuncReturnType> SC_CreateLongTask(const std::function<FuncReturnType()>& aFunction);
		template<class FuncReturnType> friend SC_Future<FuncReturnType> SC_CreateThread(const SC_ThreadProperties& aProperties, const std::function<FuncReturnType()>& aFunction);
	};

	template <typename FuncReturnType>
	class SC_Future : public SC_FutureBase
	{
	public:
		using FunctionSignature = std::function<FuncReturnType()>; // TODO: Change to SC_Function

		SC_Future() {}
		SC_Future(const SC_Future& aOther) { myData = aOther.myData; if (myData) myData->AddRef(); }
		SC_Future(SC_Future&& aOther) noexcept
		{
			myData = aOther.myData;
			aOther.myData = nullptr;
		}

		SC_FutureBase& operator=(const SC_Future& aOther)
		{
			SC_FutureBase::operator=(aOther);
			return *this;
		}

		SC_FutureBase& operator=(SC_Future&& aOther) noexcept
		{
			SC_FutureBase::operator=(SC_Move(aOther));
			return *this;
		}

		FuncReturnType& GetResult() const
		{
			Data* data = static_cast<Data*>(myData);
			Wait();
			return data->myResult;
		}

	private:
		struct Data final : DataBase
		{
			FunctionSignature myFunc;
			FuncReturnType myResult;

			void Invoke() override
			{
				myResult = myFunc();
				//myFunc.Reset();
				Finish();
			}
		};

		SC_Future(const FunctionSignature& aFunc)
		{
			Data* data = new Data();
			data->myFunc = aFunc;
			myData = data;
		}

		template<class FuncReturnType> friend SC_Future<FuncReturnType> SC_CreateFrameTask(const std::function<FuncReturnType()>& aFunction);
		template<class FuncReturnType> friend SC_Future<FuncReturnType> SC_CreateLongTask(const std::function<FuncReturnType()>& aFunction);
		template<class FuncReturnType> friend SC_Future<FuncReturnType> SC_CreateThread(const SC_ThreadProperties& aProperties, const std::function<FuncReturnType()>& aFunction);
	};

	// Specific implementation for void return values.
	template<>
	class SC_Future<void> : public SC_FutureBase
	{
	public:
		using FunctionSignature = std::function<void()>; // TODO: Change to SC_Function
		SC_Future() {}

		template<class FuncReturnType>
		SC_Future(const SC_Future<FuncReturnType>& aOther) 
		{
			myData = aOther.myData;
			if (myData)
				myData->AddRef();
		}

		template<class FuncReturnType>
		SC_Future(SC_Future<FuncReturnType>&& aOther)
		{
			static_assert(sizeof(*this) == sizeof(aOther), "Sizes doesn't match.");
			myData = aOther.myData;
			aOther.myData = nullptr;
		}

		SC_Future(const SC_Future& aOther)
		{
			myData = aOther.myData;
			if (myData)
				myData->AddRef();
		}
		SC_Future(SC_Future&& aOther) noexcept
		{
			myData = aOther.myData;
			aOther.myData = nullptr;
		}

		SC_FutureBase& operator=(const SC_Future& aOther)
		{
			SC_FutureBase::operator=(aOther);
			return *this;
		}

		SC_FutureBase& operator=(SC_Future&& aOther) noexcept
		{
			SC_FutureBase::operator=(SC_Move(aOther));
			return *this;
		}

		void GetResult()
		{
			Wait();
		}
	private:
		struct Data final : DataBase
		{
			FunctionSignature myFunc;

			void Invoke() override
			{
				myFunc();
				//myFunc.Reset();
				Finish();
			}
		};

		SC_Future(const FunctionSignature& aFunc)
		{
			Data* data = new Data();
			data->myFunc = aFunc;
			myData = data;
		}

		template<class FuncReturnType> friend SC_Future<FuncReturnType> SC_CreateFrameTask(const std::function<FuncReturnType()>& aFunction);
		template<class FuncReturnType> friend SC_Future<FuncReturnType> SC_CreateLongTask(const std::function<FuncReturnType()>& aFunction);
		template<class FuncReturnType> friend SC_Future<FuncReturnType> SC_CreateThread(const SC_ThreadProperties& aProperties, const std::function<FuncReturnType()>& aFunction);
	};

	// FRAME/LONG TASK PERMUTATIONS
#define SC_FUTURE_SETUP_CREATETASK_PERMUTATIONS(Func, Func2)\
	/* Funtion pointers */\
	template<class FuncReturnType>\
	SC_Future<FuncReturnType> Func(FuncReturnType(*aFunction)())\
	{\
		std::function<FuncReturnType()> function = std::bind(aFunction);\
		return Func2(function);\
	}\
	template<class FuncReturnType, class Arg1a, class Arg1b>\
	SC_Future<FuncReturnType> Func(FuncReturnType(*aFunction)(Arg1a), const Arg1b& aArg1)\
	{\
		std::function<FuncReturnType()> function = std::bind(aFunction, aArg1);\
		return Func2(function);\
	}\
	template<class FuncReturnType, class Arg1a, class Arg1b, class Arg2a, class Arg2b>\
	SC_Future<FuncReturnType> Func(FuncReturnType(*aFunction)(Arg1a, Arg2a), const Arg1b& aArg1, const Arg2b& aArg2)\
	{\
		std::function<FuncReturnType()> function = std::bind(aFunction, aArg1, aArg2);\
		return Func2(function);\
	}\
	template<class FuncReturnType, class Arg1a, class Arg1b, class Arg2a, class Arg2b, class Arg3a, class Arg3b>\
	SC_Future<FuncReturnType> Func(FuncReturnType(*aFunction)(Arg1a, Arg2a, Arg3a), const Arg1b& aArg1, const Arg2b& aArg2, const Arg3b& aArg3)\
	{\
		std::function<FuncReturnType()> function = std::bind(aFunction, aArg1, aArg2, aArg3);\
		return Func2(function);\
	}\
	template<class FuncReturnType, class Arg1a, class Arg1b, class Arg2a, class Arg2b, class Arg3a, class Arg3b, class Arg4a, class Arg4b>\
	SC_Future<FuncReturnType> Func(FuncReturnType(*aFunction)(Arg1a, Arg2a, Arg3a, Arg4a), const Arg1b& aArg1, const Arg2b& aArg2, const Arg3b& aArg3, const Arg4b& aArg4)\
	{\
		std::function<FuncReturnType()> function = std::bind(aFunction, aArg1, aArg2, aArg3, aArg4);\
		return Func2(function);\
	}\
	template<class FuncReturnType, class Arg1a, class Arg1b, class Arg2a, class Arg2b, class Arg3a, class Arg3b, class Arg4a, class Arg4b, class Arg5a, class Arg5b>\
	SC_Future<FuncReturnType> Func(FuncReturnType(*aFunction)(Arg1a, Arg2a, Arg3a, Arg4a, Arg5a), const Arg1b& aArg1, const Arg2b& aArg2, const Arg3b& aArg3, const Arg4b& aArg4, const Arg5b& aArg5)\
	{\
		std::function<FuncReturnType()> function = std::bind(aFunction, aArg1, aArg2, aArg3, aArg4, aArg5);\
		return Func2(function);\
	}\
	/* Member Funtions */\
	template<class FuncReturnType, class ObjA, class ObjB>\
	SC_Future<FuncReturnType> Func(ObjB* aObj, FuncReturnType(ObjA::*aFunction)())\
	{\
		std::function<FuncReturnType()> function = std::bind(aFunction, aObj);\
		return Func2(function);\
	}\
	template<class FuncReturnType, class ObjA, class ObjB, class Arg1a, class Arg1b>\
	SC_Future<FuncReturnType> Func(ObjB* aObj, FuncReturnType(ObjA::*aFunction)(Arg1a), const Arg1b& aArg1)\
	{\
		std::function<FuncReturnType()> function = std::bind(aFunction, aObj, aArg1);\
		return Func2(function);\
	}\
	template<class FuncReturnType, class ObjA, class ObjB, class Arg1a, class Arg1b, class Arg2a, class Arg2b>\
	SC_Future<FuncReturnType> Func(ObjB* aObj, FuncReturnType(ObjA::*aFunction)(Arg1a, Arg2a), const Arg1b& aArg1, const Arg2b& aArg2)\
	{\
		std::function<FuncReturnType()> function = std::bind(aFunction, aObj, aArg1, aArg2);\
		return Func2(function);\
	}\
	template<class FuncReturnType, class ObjA, class ObjB, class Arg1a, class Arg1b, class Arg2a, class Arg2b, class Arg3a, class Arg3b>\
	SC_Future<FuncReturnType> Func(ObjB* aObj, FuncReturnType(ObjA::*aFunction)(Arg1a, Arg2a, Arg3a), const Arg1b& aArg1, const Arg2b& aArg2, const Arg3b& aArg3)\
	{\
		std::function<FuncReturnType()> function = std::bind(aFunction, aObj, aArg1, aArg2, aArg3);\
		return Func2(function);\
	}\
	template<class FuncReturnType, class ObjA, class ObjB, class Arg1a, class Arg1b, class Arg2a, class Arg2b, class Arg3a, class Arg3b, class Arg4a, class Arg4b>\
	SC_Future<FuncReturnType> Func(ObjB* aObj, FuncReturnType(ObjA::*aFunction)(Arg1a, Arg2a, Arg3a, Arg4a), const Arg1b& aArg1, const Arg2b& aArg2, const Arg3b& aArg3, const Arg4b& aArg4)\
	{\
		std::function<FuncReturnType()> function = std::bind(aFunction, aObj, aArg1, aArg2, aArg3, aArg4);\
		return Func2(function);\
	}\

	// CREATE FRAME TASK
	template<class FuncReturnType>
	SC_Future<FuncReturnType> SC_CreateFrameTask(const std::function<FuncReturnType()>& aFunction)
	{
		SC_Future<FuncReturnType> future(aFunction);
		future.BeginTask(false, false, -1);
		return future;
	}
	SC_FUTURE_SETUP_CREATETASK_PERMUTATIONS(SC_CreateFrameTask, SC_CreateFrameTask);

	// CREATE LONG TASK
	template<class FuncReturnType>
	SC_Future<FuncReturnType> SC_CreateLongTask(const std::function<FuncReturnType()>& aFunction)
	{
		SC_Future<FuncReturnType> future(aFunction);
		future.BeginTask(true, false, -1);
		return future;
	}
	SC_FUTURE_SETUP_CREATETASK_PERMUTATIONS(SC_CreateLongTask, SC_CreateLongTask);

	// CREATE THREAD
	template<class FuncReturnType>
	SC_Future<FuncReturnType> SC_CreateThread(const SC_ThreadProperties& aProperties, const std::function<FuncReturnType()>& aFunction)
	{
		SC_Future<FuncReturnType> future(aFunction);
		future.BeginThread(aProperties);
		return future;
	}

	// Function pointers
	template<class FuncReturnType>
	SC_Future<FuncReturnType> SC_CreateThread(const SC_ThreadProperties& aProperties, FuncReturnType(*aFunc)())
	{
		std::function<FuncReturnType()> func = std::bind(aFunc);
		return SC_CreateThread(aProperties, func);
	}

	template<class FuncReturnType, class Arg1A, class Arg1B>
	SC_Future<FuncReturnType> SC_CreateThread(const SC_ThreadProperties& aProperties, FuncReturnType(*aFunc)(Arg1A), const Arg1B& aArg1)
	{
		std::function<FuncReturnType()> func = std::bind(aFunc, aArg1);
		return SC_CreateThread(aProperties, func);
	}

	template<class FuncReturnType, class Arg1A, class Arg1B, class Arg2A, class Arg2B>
	SC_Future<FuncReturnType> SC_CreateThread(const SC_ThreadProperties& aProperties, FuncReturnType(*aFunc)(Arg1A, Arg2A), const Arg1B& aArg1, const Arg2B& aArg2)
	{
		std::function<FuncReturnType()> func = std::bind(aFunc, aArg1, aArg2);
		return SC_CreateThread(aProperties, func);
	}

	// Member functions
	template<class FuncReturnType, class Obj1, class Obj2>
	SC_Future<FuncReturnType> SC_CreateThread(const SC_ThreadProperties& aProperties, Obj2* aObj, FuncReturnType(Obj1::*aMemberFunc)())
	{
		std::function<FuncReturnType()> func = std::bind(aMemberFunc, aObj);
		return SC_CreateThread(aProperties, func);
	}

	template<class FuncReturnType, class Obj1, class Obj2, class Arg1A, class Arg1B>
	SC_Future<FuncReturnType> SC_CreateThread(const SC_ThreadProperties& aProperties, Obj2* aObj, FuncReturnType(Obj1::*aMemberFunc)(Arg1A), const Arg1B& aArg1)
	{
		std::function<FuncReturnType()> func = std::bind(aMemberFunc, aObj, aArg1);
		return SC_CreateThread(aProperties, func);
	}

	template<class FuncReturnType, class Obj1, class Obj2, class Arg1A, class Arg1B, class Arg2A, class Arg2B>
	SC_Future<FuncReturnType> SC_CreateThread(const SC_ThreadProperties& aProperties, Obj2* aObj, FuncReturnType(Obj1::* aMemberFunc)(Arg1A, Arg2A), const Arg1B& aArg1, const Arg2B& aArg2)
	{
		std::function<FuncReturnType()> func = std::bind(aMemberFunc, aObj, aArg1, aArg2);
		return SC_CreateThread(aProperties, func);
	}



}
