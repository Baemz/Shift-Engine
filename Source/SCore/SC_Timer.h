#pragma once
#include <chrono>

namespace Shift
{
	class SC_Timer
	{
	public:
		static float GetDeltaTime();
		static float GetGlobalDeltaTime();
		static float GetGlobalTotalTime();
		static float GetTotalTime();
		static uint64 GetNumFrames();
		static uint64 GetCurrentFrameIndex();
		static uint64 GetGlobalFrameIndex();
		static uint64 GetRenderFrameIndex();
		static uint GetFPS();

		static void IncrementRenderFrameCount();

		static void Tick(bool aIsGlobalUpdate = false);
		static long long GetCurrentTimeStamp(); // Returns current timestamp in milliseconds

	private:
		static constexpr uint SC_FPS_SAMPLE_COUNT = 25;
		static thread_local float ourDelta;
		static thread_local float ourTotalTime;
		static thread_local uint64 ourFrames;
		static thread_local uint64 ourFrameIndex;
		static thread_local std::chrono::time_point<std::chrono::high_resolution_clock> ourLastTick;
		static thread_local uint ourFramerateSamples[SC_FPS_SAMPLE_COUNT];
		static thread_local uint ourFramerateSampleIndex;

		static uint64 ourGlobalFrameIndex;
		static uint64 ourRenderFrameIndex;
		static float ourGlobalDeltaTime;
		static float ourGlobalTotalTime;
	};
}
