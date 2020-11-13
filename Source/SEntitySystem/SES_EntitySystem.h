#pragma once
#include "SES_Entity.h"
#include "SES_EntityManager.h"
#include "SES_ComponentManager.h"
#include "SES_SystemManager.h"

namespace Shift
{
	class SES_EntitySystem
	{
	public:
		static void Create();
		static void Destroy();
		static SES_EntitySystem* Get();

		SES_EntitySystem();
		~SES_EntitySystem();

		SES_Entity CreateEntity();
		void DestroyEntity(SES_Entity& aEntity);

		template<class T>
		inline void RegisterComponent() { myComponentManager->RegisterComponent<T>(); }

		template<class T> 
		void AddComponent(SES_Entity& aEntity, const T& aComponent);

		template<class T>
		void RemoveComponent(SES_Entity& aEntity);

		template<class T>
		inline T* GetComponent(const SES_Entity& aEntity) { return myComponentManager->GetComponent<T>(aEntity);	}

		template<class T>
		inline SES_ComponentType GetComponentType() { return myComponentManager->GetComponentType<T>(); }

		template<class T>
		inline SC_Ref<T> RegisterSystem() { return mySystemManager->RegisterSystem<T>(); }

		template<class T>
		inline void SetSystemSignature(const SES_EntitySignature& aSignature) { mySystemManager->SetSignature<T>(aSignature); }

	private:
		static SES_EntitySystem* ourInstance;

		SC_UniquePtr<SES_EntityManager> myEntityManager;
		SC_UniquePtr<SES_ComponentManager> myComponentManager;
		SC_UniquePtr<SES_SystemManager> mySystemManager;

	};

	template<class T>
	inline void SES_EntitySystem::AddComponent(SES_Entity& aEntity, const T& aComponent)
	{
		myComponentManager->AddComponent<T>(aEntity, aComponent);

		aEntity.mySignature.set(myComponentManager->GetComponentType<T>(), true);
		mySystemManager->OnEntitySignatureChanged(aEntity, aEntity.mySignature);
	}

	template<class T>
	inline void SES_EntitySystem::RemoveComponent(SES_Entity& aEntity)
	{
		myComponentManager->RemoveComponent<T>(aEntity);

		aEntity.mySignature.set(myComponentManager->GetComponentType<T>(), false);
		mySystemManager->OnEntitySignatureChanged(aEntity, aEntity.mySignature);
	}
}