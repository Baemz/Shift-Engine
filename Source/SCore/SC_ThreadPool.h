#pragma once

namespace Shift
{
	class SC_ThreadPool
	{
	public:
		static void SetNumTaskThreads(uint aNumFrameTaskThreads = SC_UINT_MAX, uint aNumLongTaskThreads = SC_UINT_MAX);
		static void StopTaskThreads();

		static void BeginTasks(SC_FutureBase::DataBase** aData, uint aNumTasks, bool aIsLongTask, bool aIsLowPrioTask, int aThreadIdx);

	private:
		static void WorkerThreadFunc(bool aIsLongTask, uint aThreadIdx);

		static volatile uint ourNumIdleThreads[2];
	};
}
