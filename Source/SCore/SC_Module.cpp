#include "SCore_Precompiled.h"
#include "SC_Module.h"

namespace Shift
{
	SC_Module::SC_Module()
		: myUpdateTime(0.f)
	{
	}
	
	SC_Module::~SC_Module()
	{

	}

	void SC_Module::PreUpdate()
	{
		myUpdateTiming.Start("Update Module");
	}

	void SC_Module::PostUpdate()
	{
		myUpdateTime = myUpdateTiming.Stop();

	}
}