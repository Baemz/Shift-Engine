#include "SCore_Precompiled.h"
#include "SC_ModuleManager.h"

namespace Shift
{
	SC_ModuleManager::SC_ModuleManager()
	{
	}

	SC_ModuleManager::~SC_ModuleManager()
	{
		myActiveModules.RemoveAll();
	}

	void SC_ModuleManager::RegisterModule(SC_Module* aModule)
	{
		myActiveModules.AddUnique(aModule);
	}

	void SC_ModuleManager::UnregisterModule(SC_Module* aModule)
	{
		aModule;
	}

	void SC_ModuleManager::Update()
	{
		for (uint i = 0, max = myActiveModules.Count(); i < max; ++i)
		{
			SC_Module* module = myActiveModules[i];

			module->PreUpdate();
			module->Update();
			module->PostUpdate();
		}
	}
	void SC_ModuleManager::Render()
	{
		for (uint i = 0, max = myActiveModules.Count(); i < max; ++i)
		{
			SC_Module* module = myActiveModules[i];

			module->Render();
		}
	}
}