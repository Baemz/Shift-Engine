#include "SEntitySystem_Precompiled.h"
#include "Component.h"

namespace Shift
{

	CComponent::CComponent()
		: myHandle(UINT_MAX)
		, myIsActive(false)
	{
	}


	CComponent::~CComponent()
	{
	}
	const bool CComponent::IsActive() const
	{
		return myIsActive;
	}
	void CComponent::SetActiveState(const bool aBool)
	{
		myIsActive = aBool;
	}
}