#pragma once
#include "SES_ComponentArray.h"
#include "SC_RefPtr.h"
#include <unordered_map>

namespace Shift
{
	class SES_ComponentManager
	{
	public:
		template<class T>
		void RegisterComponent();

		template<class T>
		SES_ComponentType GetComponentType();

		template<class T>
		void AddComponent(const SES_Entity& aEntity, const T& aComponent);

		template<class T>
		void RemoveComponent(const SES_Entity& aEntity);

		template<class T>
		T* GetComponent(const SES_Entity& aEntity);

		void OnEntityDestroyed(SES_Entity& aEntity);

	private:
		std::unordered_map<const char*, SES_ComponentType> myComponentTypes;
		std::unordered_map<const char*, SC_Ref<SES_ComponentArrayBase>> myComponentArrays;
		SES_ComponentType myNextComponentType;

		template<class T>
		SC_Ref<SES_ComponentArray<T>> GetComponentArray();
	};

	template<class T>
	inline void SES_ComponentManager::RegisterComponent()
	{
		const char* typeName = typeid(T).name();

		assert(myComponentTypes.find(typeName) == myComponentTypes.end() && "Cannot register a component type more than once.");

		myComponentTypes.insert({ typeName, myNextComponentType });
		myComponentArrays.insert({ typeName, new SES_ComponentArray<T>() });
		++myNextComponentType;
	}

	template<class T>
	inline SES_ComponentType SES_ComponentManager::GetComponentType()
	{
		const char* typeName = typeid(T).name();
		assert(myComponentTypes.find(typeName) != myComponentTypes.end() && "Component type is not registered.");

		return myComponentTypes[typeName];
	}

	template<class T>
	inline void SES_ComponentManager::AddComponent(const SES_Entity& aEntity, const T& aComponent)
	{
		GetComponentArray<T>()->Insert(aEntity, aComponent);
	}

	template<class T>
	inline void SES_ComponentManager::RemoveComponent(const SES_Entity& aEntity)
	{
		GetComponentArray<T>()->Remove(aEntity);
	}

	template<class T>
	inline T* SES_ComponentManager::GetComponent(const SES_Entity& aEntity)
	{
		return GetComponentArray<T>()->GetComponent(aEntity);
	}

	template<class T>
	inline SC_Ref<SES_ComponentArray<T>> SES_ComponentManager::GetComponentArray()
	{
		const char* typeName = typeid(T).name();

		assert(myComponentTypes.find(typeName) != myComponentTypes.end() && "Component type is not registered.");

		SC_Ref<SES_ComponentArray<T>> result = static_cast<SES_ComponentArray<T>*>(myComponentArrays[typeName].Get());
		return result;
	}
}
