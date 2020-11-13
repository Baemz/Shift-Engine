#include "SEntitySystem_Precompiled.h"
#include "SES_SystemManager.h"

namespace Shift
{
	void SES_SystemManager::OnEntityDestroyed(const SES_Entity& aEntity)
	{
		for (auto& pair : mySystems)
		{
			auto& system = pair.second;

			system->myEntities.erase(aEntity);
		}
	}
	void SES_SystemManager::OnEntitySignatureChanged(const SES_Entity& aEntity, const SES_EntitySignature& aSignature)
	{
		for (auto& pair : mySystems)
		{
			const char* type = pair.first;
			SES_System* system = pair.second;
			const SES_EntitySignature& systemSignature = mySignatures[type];

			if ((aSignature & systemSignature) == systemSignature)
				system->myEntities.insert(aEntity);
			else
				system->myEntities.erase(aEntity);
		}
	}
}