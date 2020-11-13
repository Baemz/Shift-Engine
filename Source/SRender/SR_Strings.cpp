#include "SRender_Precompiled.h"
#include "SR_Strings.h"

namespace Shift
{
	const char* SR_GetName(SR_ContextType aContext)
	{
		switch (aContext)
		{
		case SR_ContextType_Render:
			return "Render";
		case SR_ContextType_Compute:
			return "Compute";
		case SR_ContextType_CopyInit:
			return "CopyInit";
		case SR_ContextType_CopyFast:
			return "CopyFast";
		case SR_ContextType_CopyStream:
			return "CopyStream";
		}

		assert(false && "Not a valid context");
		return nullptr;
	}
}