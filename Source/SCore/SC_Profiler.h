#pragma once

#include "SC_Mutex.h"
#include "SC_ProfilerTags.h"
#include <fstream>

#if ENABLE_PROFILER
#define SC_INTERNAL_SCOPED_PROFILER_FUNCTION_CPU(aTag, aID, aWriteSnapshotOnDestroy) Shift::SC_Profiler::SInternalProfilerScope _internalProfilerScopedVar##aID(aTag, aWriteSnapshotOnDestroy)

#define SC_PROFILER_FUNCTION() SC_INTERNAL_SCOPED_PROFILER_FUNCTION_CPU(__FUNCTION__, __LINE__, true)

#define SC_PROFILER_EVENT_START(aTag) SC_INTERNAL_SCOPED_PROFILER_FUNCTION_CPU(aTag, __LINE__, false)
#define SC_PROFILER_EVENT_END() Shift::SC_Profiler::Get().WriteSnapshot({ Shift::SC_Profiler::Get().ourCurrentScope->myTag, "Unknown Thread", (uint)std::hash<std::thread::id>{}(std::this_thread::get_id()), Shift::SC_Profiler::Get().ourCurrentScope->End(), Shift::SC_Profiler::Get().ourCurrentScope.myStart})

#define SC_PROFILER_BEGIN_SECTION(aTag) Shift::SC_Profiler::Get().BeginSection(aTag)
#define SC_PROFILER_END_SECTION() Shift::SC_Profiler::Get().EndSection()

namespace Shift
{
	using SProfilerSection = std::string;
	struct SC_ProfilerSnapshot
	{
		std::string myTag;
		std::string myThreadName;
		uint myThreadID;
		float myTimeElapsedMS;
		std::chrono::high_resolution_clock::time_point myStart;

		bool operator<(const SC_ProfilerSnapshot& aOther)
		{
			if (myStart < aOther.myStart)
				return true;

			return false;
		}
	};

	class SC_Profiler
	{
	public:
		SC_Profiler();
		~SC_Profiler();

		static SC_Profiler& Get();

		void WriteSnapshot(const SC_ProfilerSnapshot& aSnapshot);
		std::unordered_map<SProfilerSection, SC_GrowingArray<SC_ProfilerSnapshot>>& GetData() { return myProfilingData; }

		void BeginSection(const SProfilerSection& aTag);
		void EndSection();

	public:
		struct SInternalProfilerScope
		{
			SInternalProfilerScope(const char* aTag, bool aShouldWriteSnapshotOnDestroy)
				: myTag(aTag)
				, myShouldWriteSnapshotOnDestroy(aShouldWriteSnapshotOnDestroy)
			{
				myStart = std::chrono::high_resolution_clock::now();

				if (!myShouldWriteSnapshotOnDestroy)
					SC_Profiler::Get().ourCurrentScope = this;
			}
			~SInternalProfilerScope()
			{
				if (myShouldWriteSnapshotOnDestroy)
					SC_Profiler::Get().WriteSnapshot({ myTag, "Unknown Thread", (uint)std::hash<std::thread::id>{}(std::this_thread::get_id()), End(), myStart });
				else
					SC_Profiler::Get().ourCurrentScope = nullptr;
			}
			float End()
			{
				const auto newTick = std::chrono::high_resolution_clock::now();
				const std::chrono::microseconds deltaTime = std::chrono::duration_cast<std::chrono::microseconds>(newTick - myStart);
				return static_cast<float>(deltaTime.count() / 1000.f);
			}

			const std::string myTag;
			std::chrono::high_resolution_clock::time_point myStart;
			const bool myShouldWriteSnapshotOnDestroy : 1;
		};

		static thread_local SInternalProfilerScope* ourCurrentScope;
	private:
		static thread_local SProfilerSection* ourCurrentSection;
		std::unordered_map<SProfilerSection, SC_GrowingArray<SC_ProfilerSnapshot>> myProfilingData;
		SC_Mutex myMutex;
		std::ofstream myOutputStream;
	};
}
#else
#define SC_INTERNAL_SCOPED_PROFILER_FUNCTION_CPU(aTag, aID, aWriteSnapshotOnDestroy) { SC_UNUSED(aTag); SC_UNUSED(aID); SC_UNUSED(aWriteSnapshotOnDestroy); }

#define SC_PROFILER_FUNCTION()

#define SC_PROFILER_EVENT_START(aTag) { SC_UNUSED(aTag); }
#define SC_PROFILER_EVENT_END()

#define SC_PROFILER_BEGIN_SECTION(aTag) { SC_UNUSED(aTag); }
#define SC_PROFILER_END_SECTION()

#endif