#include "SCore_Precompiled.h"
#include "SC_Profiler.h"

#if ENABLE_PROFILER
namespace Shift
{
	thread_local SC_Profiler::SInternalProfilerScope* SC_Profiler::ourCurrentScope = nullptr;
	thread_local SProfilerSection* SC_Profiler::ourCurrentSection = nullptr;

	SC_Profiler::SC_Profiler()
	{
	}

	SC_Profiler::~SC_Profiler()
	{
	}

	SC_Profiler& SC_Profiler::Get()
	{
		static SC_Profiler instance;
		return instance;
	}

	void SC_Profiler::WriteSnapshot(const SC_ProfilerSnapshot& aSnapshot)
	{
		SC_MutexLock lock(myMutex);
		SProfilerSection section;

		if (ourCurrentSection)
			section = *ourCurrentSection;

		myProfilingData[section].Add(aSnapshot);
	}
	void SC_Profiler::BeginSection(const SProfilerSection& aTag)
	{
		if (ourCurrentSection)
			delete ourCurrentSection;

		ourCurrentSection = new SProfilerSection(aTag);
	}
	void SC_Profiler::EndSection()
	{
		if (ourCurrentSection)
			delete ourCurrentSection;

		ourCurrentSection = nullptr;
	}
}
#endif