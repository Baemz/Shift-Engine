#pragma once

// Includes
#include "SC_CommonDefines.h"
#include "SC_CommonIncludes.h"
#include "SC_Profiler.h"
#include "SC_EngineInterface.h"

#include "SR_RenderDefines.h"
#include "SR_RenderInterface.h"
#include "SR_RenderInclude.h"
#include "SR_ImGUI.h"
#include "SR_RenderEnums.h"

#define ENABLE_RENDER_STATS 0 //IS_EDITOR_BUILD
#if ENABLE_RENDER_STATS
#	include "SG_RenderStats.h"
#endif

#include "SG_GraphicsSettings.h"

#include "SG_DebugPrimitives.h"

