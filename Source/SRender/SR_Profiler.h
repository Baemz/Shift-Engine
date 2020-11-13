#pragma once
#include "SC_Profiler.h"
#include "SR_GraphicsEngineEnums.h"

#define SE_INTERNAL_SCOPED_PROFILER_FUNCTION_GPU(aVarName, aID)

#define SE_GPU_PROFILER_FUNCTION()
#define SE_GPU_PROFILER_FUNCTION_TAGGED(aID)

#define SE_GPU_PROFILER_EVENT_START
#define SE_GPU_PROFILER_EVENT_END

namespace Shift
{
	class SR_GraphicsProfiler
	{
	public:
		SR_GraphicsProfiler();

		void BeginSnapshot();
		void EndSnapshot();

		void Render();
	private:
		struct GPUSnapshot
		{
			std::string myTag;
			float myTimeElapsedMS;
		};
		struct GPUFrameCapture
		{
			SC_GrowingArray<GPUSnapshot> myPreviousSnapshots;
		};
		GPUFrameCapture myFrames[SContext_Count];
	};
}
