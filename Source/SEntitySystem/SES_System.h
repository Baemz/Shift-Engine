#pragma once
#include "SES_Entity.h"
#include "SC_RefCounted.h"
#include <set>

namespace Shift
{
	class SES_System : public SC_RefCounted
	{
	public:
		virtual ~SES_System() {}

		std::set<SES_Entity> myEntities;
	};
}