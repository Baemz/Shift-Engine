#include "SCore_Precompiled.h"
#include "SC_ThreadPool.h"
#include "SC_Future.h"

#include <sstream>

namespace Shift
{
	static SC_Semaphore locThreadsStartedSemaphore;
	static thread_local int locThreadPlusOneThreadIdx;
	volatile uint SC_ThreadPool::ourNumIdleThreads[2] = {};

	struct SC_TaskWorkItem
	{
		void* myItem;
	};

	struct SC_TaskWorkQueue
	{
		SC_GrowingArray<SC_TaskWorkItem> myItems;
		SC_Mutex myLock;
		SC_Semaphore mySemaphore;
		int myIsWorking;

		SC_TaskWorkQueue() : myIsWorking(0) {  }

		bool Add(void** aItems, uint aNumItems)
		{
			{
				SC_MutexLock lock(myLock);
				myItems.AllocateAdditional(aNumItems);
			}
			for (uint i = 0; i < aNumItems; ++i)
			{
				SC_TaskWorkItem item;
				item.myItem = aItems[i];
				{
					SC_MutexLock lock(myLock);
					myItems.Add(item);
				}
			}

			bool wasIdle = SC_Atomic_CompareExchange(myIsWorking, 1, 0);
			if (wasIdle)
				mySemaphore.Release();

			return wasIdle;
		}

		void* GetItem()
		{
			SC_MutexLock lock(myLock);
			if (myItems.Count() == 0)
				return nullptr;

			void* item = myItems[0].myItem;
			myItems.RemoveByIndex(0);
			
			return item;
		}
	};

	struct SC_TaskQueue
	{
		SC_TaskWorkQueue myWorkQueues[SC_MAX_NUM_CORES];
		SC_TaskWorkQueue myLowPrioQueue;
		SC_GrowingArray<SC_Future<void>> myThreads;
		uint myNumThreads;
		uint myNextPostThreadIndexExternal;
		bool myEnd;

		void Init(uint aNumThreads)
		{
			myNumThreads = aNumThreads;
			myNextPostThreadIndexExternal = 0;
			myEnd = false;
		}

		void Destroy()
		{
			myNumThreads = 0;
		}

		void Post(void** aItems, uint aNumItems, int aThreadIdx)
		{
			uint idx;
			if ((uint)aThreadIdx < myNumThreads)
				idx = aThreadIdx;
			else
			{
				int threadIdx = int(locThreadPlusOneThreadIdx) - 1;
				if (threadIdx != -1 && threadIdx < int(myNumThreads))
					idx = threadIdx;
				else
				{
					idx = myNextPostThreadIndexExternal;
					myNextPostThreadIndexExternal = (idx + 1) % myNumThreads;
				}
			}

			SC_TaskWorkQueue& queue = myWorkQueues[idx];
			uint numRemaining = aNumItems;
			if (queue.Add(aItems, aNumItems))
				--numRemaining;

			if (numRemaining)
				WakeThreads(idx, numRemaining);
		}

		void PostLowPrio(void** aItems, uint aNumItems, int aThreadIdx)
		{
			myLowPrioQueue.Add(aItems, aNumItems);
			WakeThreads(SC_Max(0, aThreadIdx), aNumItems);
		}

		void WakeThreads(uint aSelfIdx, uint aMaxThreadsToWake)
		{
			uint numThreads = myNumThreads;
			uint numThreadsToWake = SC_Min(numThreads, aMaxThreadsToWake);
			uint numWokenThreads = 0;

			for (uint i = 0; i < numThreads; ++i)
			{
				uint wakeIdx = (aSelfIdx + numThreads - 1 - i) % numThreads;

				if (SC_Atomic_CompareExchange(myWorkQueues[wakeIdx].myIsWorking, 1, 0))
				{
					myWorkQueues[wakeIdx].mySemaphore.Release();
					++numWokenThreads;
					if (numWokenThreads == numThreadsToWake)
						return;
				}
			}
		}

		void PostEnd()
		{
			myEnd = true;
			for (uint i = 0; i < myNumThreads; ++i)
				myWorkQueues[i].mySemaphore.Release();
		}

		void* Get(uint aThreadIdx, bool aBlocking, bool& aOutWouldBlock)
		{
			SC_TaskWorkQueue& localQueue = myWorkQueues[aThreadIdx];

			void* item;
			for (;;)
			{
				item = localQueue.GetItem();
				if (item)
					break;

				if (myNumThreads > 1)
				{
					bool stoleItem = false;
					for (uint i = 0; i < myNumThreads; ++i)
					{
						uint stolenIndex = (aThreadIdx + i) % myNumThreads;
						item = myWorkQueues[stolenIndex].GetItem();
						if (item)
						{
							stoleItem = true;
							break;
						}
					}
					if (stoleItem)
						break;

					item = myLowPrioQueue.GetItem();
					if (item)
						break;

					if (myEnd)
						return nullptr;

					if (!aBlocking)
					{
						aOutWouldBlock = true;
						return nullptr;
					}

					if (localQueue.myIsWorking)
						SC_Atomic_Exchange_GetOld(localQueue.myIsWorking, 0);
					else
					{
					#if IS_WINDOWS
						WaitForSingleObjectEx(localQueue.mySemaphore.mySemaphoreImpl.mySemaphore, INFINITE, FALSE);
					#elif IS_PS4_PLATFORM
					#else
						localQueue.mySemaphore.mySemaphoreImpl.Acquire();
					#endif
					}
				}
			}

			if (!localQueue.myIsWorking)
				SC_Atomic_Exchange_GetOld(localQueue.myIsWorking, 1);

			return item;
		}

		operator bool() const
		{
			return true;
		}
	};

	SC_TaskQueue locTaskQueues[2];
	void SC_ThreadPool::SetNumTaskThreads(uint aNumFrameTaskThreads, uint aNumLongTaskThreads)
	{
		for (uint i = 0; i < 2; ++i)
		{
			bool isLongTask = i != 0;

			SC_TaskQueue& taskQueue = locTaskQueues[i];

			uint count = (isLongTask) ? aNumLongTaskThreads : aNumFrameTaskThreads;
			if (count == SC_UINT_MAX)
				count = SC_Clamp(SC_ThreadTools::GetLogicalThreadCount() - 1, 2u, (uint)SC_MAX_NUM_CORES);

			if (taskQueue.myThreads.Count() != count)
			{
				if (taskQueue)
				{
					for (uint idx = 0; idx < taskQueue.myThreads.Count(); ++idx)
						taskQueue.PostEnd();
					for (uint idx = 0; idx < taskQueue.myThreads.Count(); ++idx)
						taskQueue.myThreads[idx].WaitForThread();

					taskQueue.myThreads.RemoveAll();
					taskQueue.Destroy();
				}

				if (count)
				{
					taskQueue.Init(count);
					SC_ThreadProperties threadProps;
					if (isLongTask)
					{
						threadProps.myAffinity = SC_ThreadTools::ourLongTaskThreadAffinity;
						threadProps.myPriority = SC_ThreadPriority::SC_ThreadPriority_Low;
					}
					else
					{
						threadProps.myAffinity = SC_ThreadTools::ourFrameTaskThreadAffinity;
						threadProps.myPriority = SC_ThreadPriority::SC_ThreadPriority_High;
					}

					taskQueue.myThreads.PreAllocate(count);
					for (uint threadIdx = 0; threadIdx < count; ++threadIdx)
					{
						std::stringstream sstring;
						if (isLongTask)
							sstring << "Long Task " << std::to_string(threadIdx);
						else

							sstring << "Frame Task " << std::to_string(threadIdx);

						threadProps.myName = sstring.str();
						taskQueue.myThreads.Add(SC_CreateThread(threadProps, &SC_ThreadPool::WorkerThreadFunc, isLongTask, threadIdx));
					}

					for (uint semIdx = 0; semIdx < count; ++semIdx)
						locThreadsStartedSemaphore.Acquire();
				}
			}
		}
	}

	void SC_ThreadPool::StopTaskThreads()
	{
		SetNumTaskThreads(0, 0);
	}

	void SC_ThreadPool::BeginTasks(SC_FutureBase::DataBase** aData, uint aNumTasks, bool aIsLongTask, bool aIsLowPrioTask, int aThreadIdx)
	{
		assert(aNumTasks);

		SC_TaskQueue& taskQueue = locTaskQueues[int(aIsLongTask)];
		assert(taskQueue && "No SC_TaskQueue available, did you call SC_ThreadPool::SetNumTaskThreads?");

		int threadIdx = aThreadIdx;
		if (aIsLowPrioTask)
			taskQueue.PostLowPrio(reinterpret_cast<void**>(aData), aNumTasks, threadIdx);
		else
			taskQueue.Post(reinterpret_cast<void**>(aData), aNumTasks, threadIdx);
	}

	void SC_ThreadPool::WorkerThreadFunc(bool aIsLongTask, uint aThreadIdx)
	{
		SC_ThreadTools::IsFrameTaskThread = !aIsLongTask;
		const uint taskQueueIdx = aIsLongTask ? 1 : 0;

		SC_TaskQueue& taskQueue = locTaskQueues[taskQueueIdx];
		volatile uint& idleCounter = ourNumIdleThreads[taskQueueIdx];
		locThreadPlusOneThreadIdx = aThreadIdx + 1;

		locThreadsStartedSemaphore.Release();

		SC_Atomic_Increment(idleCounter);

		bool inWait = true;
		for (;;)
		{
			bool wouldBlock = false;
			void* item = taskQueue.Get(aThreadIdx, inWait, wouldBlock);

			if (wouldBlock)
			{
				if (!inWait)
				{
					SC_Atomic_Increment(idleCounter);
					inWait = true;
				}

				item = taskQueue.Get(aThreadIdx, true, wouldBlock);
			}

			SC_FutureBase::DataBase* data = (SC_FutureBase::DataBase*)item;
			if (!data)
				break;

			for (;;)
			{
				int prevState = SC_Atomic_CompareExchange_GetOld(data->myState, SC_FutureBase::DataBase::Started, 0);
				if (prevState & SC_FutureBase::DataBase::Started)
					break;
				else if (prevState == 0)
				{
					if (inWait)
					{
						SC_Atomic_Decrement(idleCounter);
						inWait = false;
					}

					data->Invoke();
					break;
				}
			}
			data->RemoveRef();
		}

		if (inWait)
			SC_Atomic_Decrement(idleCounter);
	}
}