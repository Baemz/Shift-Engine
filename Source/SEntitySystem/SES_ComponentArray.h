#pragma once
#include "SES_Entity.h"

#include "SC_RefCounted.h"

namespace Shift
{
	class SES_ComponentArrayBase : public SC_RefCounted
	{
	public:
		virtual ~SES_ComponentArrayBase() = default;
		virtual void OnEntityDestroyed(const SES_Entity& aEntity) = 0;
	};

	template<class T>
	class SES_ComponentArray : public SES_ComponentArrayBase
	{
	public:
		void Insert(const SES_Entity& aEntity, const T& aComponent);
		void Remove(const SES_Entity& aEntity);

		T* GetComponent(const SES_Entity& aEntity);

		void OnEntityDestroyed(const SES_Entity& aEntity) override;

	private:
		T myComponents[SES_MaxNumEntities];

		SC_HashMap<SES_EntityId, size_t> myEntityToIndexMapping;
		SC_HashMap<size_t, SES_EntityId> myIndexToEntityMapping;

		size_t mySize;
	};

	template<class T>
	inline void SES_ComponentArray<T>::Insert(const SES_Entity& aEntity, const T& aComponent)
	{
		assert(myEntityToIndexMapping.find(aEntity.myId) == myEntityToIndexMapping.end() && "Component added to same entity more than once.");

		size_t newIndex = mySize;
		myEntityToIndexMapping[aEntity.myId] = newIndex;
		myIndexToEntityMapping[newIndex] = aEntity.myId;
		myComponents[newIndex] = aComponent;
		++mySize;
	}

	template<class T>
	inline void SES_ComponentArray<T>::Remove(const SES_Entity& aEntity)
	{
		assert(myEntityToIndexMapping.find(aEntity.myId) != myEntityToIndexMapping.end() && "Removing non-existent component.");

		size_t indexOfRemovedEntity = myEntityToIndexMapping[aEntity.myId];
		size_t indexOfLastElement = mySize - 1;
		myComponents[indexOfRemovedEntity] = myComponents[indexOfLastElement];

		SES_EntityId entityOfLastElement = myIndexToEntityMapping[indexOfLastElement];
		myEntityToIndexMapping[entityOfLastElement] = indexOfRemovedEntity;
		myIndexToEntityMapping[indexOfRemovedEntity] = entityOfLastElement;

		myEntityToIndexMapping.RemoveByKey(aEntity.myId);
		myIndexToEntityMapping.RemoveByKey(indexOfLastElement);

		--mySize;
	}

	template<class T>
	inline T* SES_ComponentArray<T>::GetComponent(const SES_Entity& aEntity)
	{
		if (myEntityToIndexMapping.Find(aEntity.myId))
			return &myComponents[myEntityToIndexMapping[aEntity.myId]];
		else
			return nullptr;
	}

	template<class T>
	inline void SES_ComponentArray<T>::OnEntityDestroyed(const SES_Entity& aEntity)
	{
		if (myEntityToIndexMapping.Find(aEntity.myId))
			Remove(aEntity);
	}
}