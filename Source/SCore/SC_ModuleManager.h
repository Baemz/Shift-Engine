#pragma once
#include "SC_Module.h"

namespace Shift
{
	class SC_ModuleManager
	{
	public:
		SC_ModuleManager();
		~SC_ModuleManager();

		/* 
			Register you module.
			The registering is order-dependent, first in is first inited/updated/renderd etc.
		*/
		void RegisterModule(SC_Module* aModule);

		/* Unregister you module */
		void UnregisterModule(SC_Module* aModule);

		void Update();
		void Render();

	private:
		SC_GrowingArray<SC_Module*> myActiveModules;

	};
}
