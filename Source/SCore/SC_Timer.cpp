#include "SCore_Precompiled.h"
#include "SC_Timer.h"

namespace Shift
{
	thread_local float SC_Timer::ourDelta = 0.f;
	thread_local float SC_Timer::ourTotalTime = 0.f;
	thread_local uint64 SC_Timer::ourFrames = 0;
	thread_local uint64 SC_Timer::ourFrameIndex = 0;
	thread_local std::chrono::time_point<std::chrono::high_resolution_clock> SC_Timer::ourLastTick;
	thread_local uint SC_Timer::ourFramerateSamples[SC_FPS_SAMPLE_COUNT] = {0,0,0,0,0,0,0,0,0,0};
	thread_local uint SC_Timer::ourFramerateSampleIndex = 0;

	uint64 SC_Timer::ourGlobalFrameIndex = 0;
	uint64 SC_Timer::ourRenderFrameIndex = 0;
	float SC_Timer::ourGlobalDeltaTime = 0.0f;
	float SC_Timer::ourGlobalTotalTime = 0.0f;

	void SC_Timer::Tick(bool aIsGlobalUpdate)
	{
		const auto newTick = std::chrono::high_resolution_clock::now();
		const std::chrono::duration<float> deltaTime = std::chrono::duration_cast<std::chrono::duration<float>>(newTick - ourLastTick);
		ourLastTick = newTick;
		ourDelta = SC_Min(deltaTime.count(), 0.5f);
		ourTotalTime += ourDelta;
		ourFrames++;
		ourFrameIndex = ourFrames;

		ourFramerateSamples[ourFramerateSampleIndex] = uint(1 / ourDelta);
		ourFramerateSampleIndex = ourFrames % SC_FPS_SAMPLE_COUNT;

		if (aIsGlobalUpdate)
		{
			ourGlobalFrameIndex = ourFrames;
			ourGlobalDeltaTime = ourDelta;
			ourGlobalTotalTime += ourDelta;
		}
	}

	long long SC_Timer::GetCurrentTimeStamp()
	{
		const auto t1 = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now());
		return t1.time_since_epoch().count();
	}

	float SC_Timer::GetDeltaTime()
	{
		return ourDelta;
	}

	float SC_Timer::GetGlobalDeltaTime()
	{
		return ourGlobalDeltaTime;
	}

	float SC_Timer::GetGlobalTotalTime()
	{
		return ourGlobalTotalTime;
	}

	float SC_Timer::GetTotalTime()
	{
		return ourTotalTime;
	}

	uint64 SC_Timer::GetNumFrames()
	{
		return ourFrames;
	}

	uint64 SC_Timer::GetCurrentFrameIndex()
	{
		return ourFrameIndex;
	}

	uint64 SC_Timer::GetGlobalFrameIndex()
	{
		return ourGlobalFrameIndex;
	}

	uint64 SC_Timer::GetRenderFrameIndex()
	{
		return ourRenderFrameIndex;
	}

	uint SC_Timer::GetFPS()
	{
		uint fps = 0;
		for (uint i = 0; i < SC_FPS_SAMPLE_COUNT; ++i)
			fps += ourFramerateSamples[i];

		fps /= SC_FPS_SAMPLE_COUNT;
		return fps;
	}

	void SC_Timer::IncrementRenderFrameCount()
	{
		assert(SC_ThreadTools::IsRenderThread);
		++ourRenderFrameIndex;
	}
}