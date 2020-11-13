#pragma once
#include <bitset>

namespace Shift
{
	static constexpr uint SES_ComponentType_MaxCount = 0xff;
	static constexpr uint SES_MaxNumEntities = 128 * 1024;

	using SES_ComponentType = uint8;
	using SES_EntityId = uint;
	using SES_EntitySignature = std::bitset<SES_ComponentType_MaxCount>;

	struct SES_Entity
	{
		SES_Entity() : myId(SC_UINT32_MAX) {}


		SES_EntitySignature mySignature;
		SES_EntityId myId;
	};
	inline bool operator<(const SES_Entity& aSelf, const SES_Entity& aOther)
	{
		return aSelf.myId < aOther.myId;
	}
}