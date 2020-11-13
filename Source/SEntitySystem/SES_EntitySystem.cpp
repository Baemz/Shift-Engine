#include "SEntitySystem_Precompiled.h"
#include "SES_EntitySystem.h"

namespace Shift
{
	SES_EntitySystem* SES_EntitySystem::ourInstance = nullptr;



	void SES_EntitySystem::Create()
	{
		assert(!ourInstance);

		if (!ourInstance)
			ourInstance = new SES_EntitySystem();
	}

	void SES_EntitySystem::Destroy()
	{
		delete ourInstance;
	}

	SES_EntitySystem* SES_EntitySystem::Get()
	{
		assert(ourInstance);
		return ourInstance;
	}

	SES_EntitySystem::SES_EntitySystem()
	{
		myEntityManager = new SES_EntityManager();
		myComponentManager = new SES_ComponentManager();
		mySystemManager = new SES_SystemManager();
	}

	SES_EntitySystem::~SES_EntitySystem()
	{
	}

	SES_Entity SES_EntitySystem::CreateEntity()
	{
		return myEntityManager->CreateEntity();
	}

	void SES_EntitySystem::DestroyEntity(SES_Entity& aEntity)
	{
		myEntityManager->DestroyEntity(aEntity);
		myComponentManager->OnEntityDestroyed(aEntity);
		mySystemManager->OnEntityDestroyed(aEntity);
	}
}