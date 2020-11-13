#include "SRender_Precompiled.h"
#include "SR_ShaderState.h"

namespace Shift
{
	SR_ShaderState::SR_ShaderState()
		: myIsCompute(false)
		, myIsRaytracing(false)
	{
	}

	SR_ShaderState::~SR_ShaderState()
	{}
}