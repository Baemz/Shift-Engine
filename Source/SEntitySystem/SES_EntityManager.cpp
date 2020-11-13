#include "SEntitySystem_Precompiled.h"
#include "SES_EntityManager.h"

namespace Shift
{
	SES_EntityManager::SES_EntityManager()
	{
		for (uint entityId = 0; entityId < SES_MaxNumEntities; ++entityId)
		{
			myAvailableEntities.push(entityId);
		}
	}

	SES_EntityManager::~SES_EntityManager()
	{
	}

	SES_Entity SES_EntityManager::CreateEntity()
	{
		assert(myAliveCount < SES_MaxNumEntities);

		SES_Entity entity;
		entity.myId = myAvailableEntities.front();
		myAvailableEntities.pop();
		++myAliveCount;

		return entity;
	}

	void SES_EntityManager::DestroyEntity(SES_Entity& aEntity)
	{
		assert(aEntity.myId < SES_MaxNumEntities && "Entity out of range.");

		aEntity.mySignature.reset();
		myAvailableEntities.push(aEntity.myId);
		--myAliveCount;
	}
}