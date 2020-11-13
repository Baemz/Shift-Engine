#pragma once
#include <queue>
#include <mutex>

#ifndef SC_MAX_NUM_CORES
#	if IS_PC_PLATFORM
#		if IS_LINUX
#			define SC_MAX_NUM_CORES 16
#		else
#			define SC_MAX_NUM_CORES 64
#		endif
#	elif IS_XBOX_PLATFORM
#		define SC_MAX_NUM_CORES 7
#	elif IS_PS4_PLATFORM
#		define SC_MAX_NUM_CORES 7
#	endif
#endif

namespace Shift
{

#if IS_WINDOWS

	using SC_ThreadHandle = HANDLE;
	using SC_ThreadID = DWORD;
	using SC_ThreadAffinity = DWORD_PTR;
	#define	SC_ThreadNullHandle 0
	#define	SC_ThreadInvalidHandle 0

#elif IS_LINUX
#elif IS_PS4_PLATFORM
#endif

	void SC_Sleep(uint aMilliseconds);
	void SC_Sleep_NonProfiled(uint aMilliseconds);
	void SC_Yield();
	void SC_Yield_NonProfiled();

	enum SC_ThreadPriority
	{
		SC_ThreadPriority_Critical,
		SC_ThreadPriority_Highest,
		SC_ThreadPriority_High,
		SC_ThreadPriority_Medium,
		SC_ThreadPriority_Low,

		SC_ThreadPriority_Unknown,

		SC_ThreadPriority_COUNT = SC_ThreadPriority_Unknown
	};

	namespace SC_ThreadTools
	{
		void RegisterMainThread();

		void SetCurrentThreadName(const char* aName);

		SC_ThreadID GetCurrentThreadId();

		bool SetThreadAffinityMask(SC_ThreadHandle aThread, SC_ThreadAffinity aAffinity);

		void SetCurrentThreadPriority(SC_ThreadPriority aPriority);
		SC_ThreadPriority GetCurrentThreadPriority();

		uint GetLogicalThreadCount();

		extern const SC_ThreadAffinity ourDefaultThreadAffinity;
		extern const SC_ThreadAffinity ourMainThreadAffinity;
		extern const SC_ThreadAffinity ourCriticalThreadAffinity;
		extern const SC_ThreadAffinity ourRenderThreadAffinity;
		extern const SC_ThreadAffinity ourLongTaskThreadAffinity;
		extern const SC_ThreadAffinity ourFrameTaskThreadAffinity;

		extern thread_local bool IsMainThread;
		extern thread_local bool IsRenderThread;
		extern thread_local bool IsFrameTaskThread;
		extern thread_local bool IsTextureStreamerThread;
		extern thread_local bool IsMeshStreamerThread;
	}

	template <typename T>
	class SC_ThreadSafeQueue
	{
	public:
		/**
		* Destructor.
		*/
		~SC_ThreadSafeQueue(void)
		{
			invalidate();
		}

		/**
		* Attempt to get the first value in the queue.
		* Returns true if a value was successfully written to the out parameter, false otherwise.
		*/
		bool tryPop(T& out)
		{
			std::lock_guard<std::mutex> lock{ m_mutex };
			if (m_queue.empty() || !m_valid)
			{
				return false;
			}
			out = std::move(m_queue.front());
			m_queue.pop();
			return true;
		}

		/**
		* Attempt to get all values in the queue.
		* Returns true if a value was successfully written to the out parameter, false otherwise.
		*/
		bool TryPopAll(SC_GrowingArray<T>& aOut)
		{
			std::lock_guard<std::mutex> lock{ m_mutex };
			if (m_queue.empty() || !m_valid)
			{
				return false;
			}
			aOut.PreAllocate((uint)m_queue.size());
			for (uint i = 0; i < m_queue.size(); ++i)
			{
				T& obj = aOut.Add();
				obj = SC_Move(m_queue.front());
				m_queue.pop();
			}
			return true;
		}

		/**
		* Get the first value in the queue.
		* Will block until a value is available unless clear is called or the instance is destructed.
		* Returns true if a value was successfully written to the out parameter, false otherwise.
		*/
		bool waitPop(T& out)
		{
			std::unique_lock<std::mutex> lock{ m_mutex };
			m_condition.wait(lock, [this]()
			{
				return !m_queue.empty() || !m_valid;
			});
			/*
			* Using the condition in the predicate ensures that spurious wakeups with a valid
			* but empty queue will not proceed, so only need to check for validity before proceeding.
			*/
			if (!m_valid)
			{
				return false;
			}
			out = std::move(m_queue.front());
			m_queue.pop();
			return true;
		}

		/**
		* Get all the values in the queue (Sorted: front -> back)
		* Will block until a value is available unless clear is called or the instance is destructed.
		* Returns true if a value was successfully written to the out parameter, false otherwise.
		*/
		bool WaitPopAll(SC_GrowingArray<T>& aOut)
		{
			std::unique_lock<std::mutex> lock{ m_mutex };
			m_condition.wait(lock, [this]()
				{
					return !m_queue.empty() || !m_valid;
				});
			/*
			* Using the condition in the predicate ensures that spurious wakeups with a valid
			* but empty queue will not proceed, so only need to check for validity before proceeding.
			*/
			if (!m_valid)
			{
				return false;
			}
			aOut.PreAllocate((uint)m_queue.size());
			for (uint i = 0; i < m_queue.size(); ++i)
			{
				T& obj = aOut.Add();
				obj = std::move(m_queue.front());
				m_queue.pop();
			}
			return true;
		}

		/**
		* Push a new value onto the queue.
		*/
		void push(T value)
		{
			std::lock_guard<std::mutex> lock{ m_mutex };
			m_queue.push(std::move(value));
			m_condition.notify_one();
		}

		/**
		* Check whether or not the queue is empty.
		*/
		bool empty(void) const
		{
			std::lock_guard<std::mutex> lock{ m_mutex };
			return m_queue.empty();
		}

		/**
		* Clear all items from the queue.
		*/
		void clear(void)
		{
			std::lock_guard<std::mutex> lock{ m_mutex };
			while (!m_queue.empty())
			{
				m_queue.pop();
			}
			m_condition.notify_all();
		}

		/**
		* Invalidate the queue.
		* Used to ensure no conditions are being waited on in waitPop when
		* a thread or the application is trying to exit.
		* The queue is invalid after calling this method and it is an error
		* to continue using a queue after this method has been called.
		*/
		void invalidate(void)
		{
			std::lock_guard<std::mutex> lock{ m_mutex };
			m_valid = false;
			m_condition.notify_all();
		}

		/**
		* Returns whether or not this queue is valid.
		*/
		bool isValid(void) const
		{
			std::lock_guard<std::mutex> lock{ m_mutex };
			return m_valid;
		}

	private:
		std::atomic_bool m_valid{ true };
		mutable std::mutex m_mutex;
		std::queue<T> m_queue;
		std::condition_variable m_condition;
	};
}