#include "SEntitySystem_Precompiled.h"
#include "SES_ComponentManager.h"

namespace Shift
{
	void SES_ComponentManager::OnEntityDestroyed(SES_Entity& aEntity)
	{
		for (auto& pair : myComponentArrays)
		{
			auto& component = pair.second;
			component->OnEntityDestroyed(aEntity);
		}
	}
}