#pragma once

// Includes

#define _ENFORCE_MATCHING_ALLOCATORS 0
#include <cmath>
#include <iostream>
#include <functional>
#include <algorithm>
#include <vector>
#include <array>
#include <map>
#include <chrono>
#include <fstream>
#include <type_traits>
#include <thread>
#include <atomic>
#include <mutex>

#include "SC_Types.h"
#include "SC_Math.h"
#include "SC_MathTypes.h"
#include "SC_CommonFuncs.h"
#include "SC_Relocate.h"
#include "SC_Pair.h"

#include "SC_Singleton.h"
#include "SC_Assert.h"
#include "SC_Enum.h"
#include "SC_Random.h"
#include "SC_Memory.h"
#include "SC_Hash.h"
#include "SC_AABB.h"
#include "SC_Sphere.h"
#include "SC_String.h"
#include "SC_Stopwatch.h"
#include "SC_StringHelpers.h"
#include "SC_RefPtr.h"
#include "SC_UniquePtr.h"

#include "SC_EnginePaths.h"
#include "SC_EngineInterface.h"

// Tools
#include "SC_GrowingArray.h"
#include "SC_Array.h"
#include "SC_HybridArray.h"
#include "SC_DynamicCircularArray.h"
#include "SC_HashMap.h"

#include "SC_CommandLineManager.h"
#include "SC_Logger.h"
#include "SC_Timer.h"
#include "SC_Json.hpp"

using SC_Json = nlohmann::json;


