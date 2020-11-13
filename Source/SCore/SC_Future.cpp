#include "SCore_Precompiled.h"
#include "SC_Future.h"

#include "SC_Thread.h"
#include "SC_ThreadPool.h"

namespace Shift
{
	struct SC_FutureBase::SC_FutureThread final : public SC_Thread
	{
		DataBase* myData;

		SC_FutureThread(DataBase* aData) : myData(aData)
		{
			myShouldAutoDelete = true;
			myShouldCloseHandleOnDelete = false;
		}

		void Run() override
		{
			myData->Invoke();
			myData->RemoveRef();
		}
	};

	SC_FutureBase::DataBase::~DataBase()
	{
		assert(myRefCount == 0);

		assert(myThreadHandle == SC_ThreadInvalidHandle || (myState & State::Finished));
		if (myThreadHandle != SC_ThreadInvalidHandle)
		{
			SC_Thread::CloseThreadHandle(myThreadHandle);
		}
	}
	void SC_FutureBase::DataBase::Stop()
	{
		for (;;)
		{
			int prevState = SC_Atomic_CompareExchange_GetOld(myState, State::Started|State::Locked, State::Started);

			if (prevState & State::Finished)
				break;

			if (prevState == State::Started)
			{
				myThread->Stop();
				int old = SC_Atomic_Exchange_GetOld(myState, State::Started);
				assert(old == (State::Started|State::Locked));
				SC_UNUSED(old);
				break;
			}
		}
	}
	void SC_FutureBase::DataBase::Finish()
	{
		for (;;)
		{
			int prevState = SC_Atomic_CompareExchange_GetOld(myState, State::Started|State::Finished|State::Locked, State::Started);
			assert(prevState & State::Started);
			assert((prevState & State::Finished) == 0);

			if (prevState == State::Started)
			{
				for (uint i = 0, count = myWaitingSemaphores.Count(); i != count; ++i)
					myWaitingSemaphores[i]->Release();
				myWaitingSemaphores.RemoveAll();

				int old = SC_Atomic_Exchange_GetOld(myState, State::Started|State::Finished);
				assert(old == (State::Started | State::Finished | State::Locked));
				SC_UNUSED(old);
				return;
			}
		}
	}
	bool SC_FutureBase::Wait(bool aBlock) const
	{
		if (myData)
		{
			for (;;)
			{
				int prevState = SC_Atomic_CompareExchange_GetOld(myData->myState, DataBase::State::Started, 0);
				if (prevState == 0)
				{
					myData->Invoke();
					break;
				}
				else if (prevState & DataBase::State::Finished)
					break;
				else if (prevState & DataBase::State::Started)
				{
					if (!aBlock)
						return false;

					SC_CurrentThreadSemaphoreRef semaphore;
					for (;;)
					{
						prevState = SC_Atomic_CompareExchange_GetOld(myData->myState, DataBase::State::Started | DataBase::State::Locked, DataBase::State::Started);
						if (prevState == DataBase::State::Started)
						{
							myData->myWaitingSemaphores.Add(semaphore);

							int old = SC_Atomic_Exchange_GetOld(myData->myState, DataBase::State::Started);
							assert(old == (DataBase::State::Started | DataBase::State::Locked));
							SC_UNUSED(old);

							semaphore->Acquire();
							break;
						}
						else if (prevState & DataBase::State::Finished)
							break;
					}
					break;

				}
			}
			assert(myData->myState & DataBase::State::Finished);
			return true;
		}
		return false;
	}

	void SC_FutureBase::WaitForThread() const
	{
		if (myData)
		{
			Wait();

			SC_Thread::Wait(myData->myThreadHandle);
		}
	}

	bool SC_FutureBase::IsActive() const
	{
		if (myData)
			return (myData->myState & DataBase::State::Finished) == 0;
		
		return false;
	}

	void SC_FutureBase::Stop(bool aBlock) const
	{
		if (myData)
			myData->Stop();

		if (aBlock)
			WaitForThread();
	}

	void SC_FutureBase::BeginThread(const SC_ThreadProperties& aProperties)
	{
		DataBase* data = myData;
		SC_FutureThread* thread = new SC_FutureThread(data);
		data->myThread = thread;
		data->myState = DataBase::State::Started;

		assert(data->myRefCount == 2);
		data->myThreadHandle = SC_Thread::Start(thread, aProperties);
	}

	void SC_FutureBase::BeginTask(bool aIsLongTask, bool aIsLowPrio, int aThreadIdx)
	{
		SC_ThreadPool::BeginTasks(&myData, 1, aIsLongTask, aIsLowPrio, aThreadIdx);
	}

}