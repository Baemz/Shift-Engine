#pragma once
#include "SES_Entity.h"
#include "SES_System.h"

namespace Shift
{
    class SES_SystemManager
    {
	public:
		template<typename T>
		SC_Ref<T> RegisterSystem();

		template<typename T>
		void SetSignature(const SES_EntitySignature& aSignature);

		void OnEntityDestroyed(const SES_Entity& aEntity);

		void OnEntitySignatureChanged(const SES_Entity& aEntity, const SES_EntitySignature& aSignature);

	private:
		std::unordered_map<const char*, SES_EntitySignature> mySignatures;
		std::unordered_map<const char*, SC_Ref<SES_System>> mySystems;
    };

	template<typename T>
	inline SC_Ref<T> SES_SystemManager::RegisterSystem()
	{
		const char* typeName = typeid(T).name();

		assert(mySystems.find(typeName) == mySystems.end() && "Cannot register a system more than once.");

		SC_Ref<SES_System> system = new T();
		mySystems.insert({ typeName, system });
		return system;
	}

	template<typename T>
	inline void SES_SystemManager::SetSignature(const SES_EntitySignature& aSignature)
	{
		const char* typeName = typeid(T).name();

		assert(mySystems.find(typeName) != mySystems.end() && "System isn´t registered.");
		mySignatures.insert({ typeName, aSignature });
	}
}
