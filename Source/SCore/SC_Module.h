#pragma once
#include "SC_RefCounted.h"
#include "SC_Stopwatch.h"

namespace Shift
{

#define SC_MODULE_DECL(aName)														\
	public:																		\
	const char* GetModuleName() const override { return SC_STRINGIFY(aName); }


	class SC_Module : public SC_RefCounted
	{
	public:
		SC_Module();
		virtual ~SC_Module();

		virtual bool OnLoad() { return true; }

		void PreUpdate();
		virtual void Update() {}
		void PostUpdate();

		virtual void Render() {}

		virtual const char* GetModuleName() const = 0;

	protected:
		SC_Stopwatch myUpdateTiming;
		float myUpdateTime;
	};
}