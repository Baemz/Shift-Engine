#pragma once
#include "SES_Entity.h"

#include <queue>

namespace Shift
{
	class SES_EntityManager
	{
	public:
		SES_EntityManager();
		~SES_EntityManager();

		SES_Entity CreateEntity();
		void DestroyEntity(SES_Entity& aEntity);

	private:
		std::queue<SES_EntityId> myAvailableEntities;
		uint myAliveCount;
	};

}