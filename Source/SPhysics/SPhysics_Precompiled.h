#pragma once

#define SP_INTERNAL_SAFE_REALEASE(ptr) if(ptr != nullptr) { ptr->release(); }

// Includes
#include "SC_CommonDefines.h"
#include "SC_CommonIncludes.h"
#include "SC_Profiler.h"

#include "PxPhysicsAPI.h"
#include "PxRigidBody.h"
#include "PxRigidDynamic.h"

using namespace physx;

#include "SP_PhysicsEnums.h"

