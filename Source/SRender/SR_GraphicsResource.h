#pragma once
#include "SC_Resource.h"
#include "SC_Misc.h"

namespace Shift
{
	class SR_TrackedResource;
	class SR_Resource : public SC_Resource
	{
	public:
		SR_Resource();
		virtual ~SR_Resource() {}
		virtual bool CanRelease(volatile uint& aRefCount) override;
	};
}
