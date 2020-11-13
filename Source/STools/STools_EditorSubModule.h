#pragma once
#include "SC_RefCounted.h"

namespace Shift
{

#define STOOLS_DECLARESUBMODULE(T)											\
		public:																\
			const char* GetId() const override { return SC_STRINGIFY(T); }

	class STools_EditorSubModule : public SC_RefCounted
	{
	public:
		STools_EditorSubModule();
		virtual ~STools_EditorSubModule();

		void UpdateModule();
		virtual void Update() {}

		void RenderModule();
		virtual void PreRender() {}
		virtual void Render() = 0;
		virtual void PostRender() {}


		void UnregisterTool();

		virtual const char* GetId() const = 0;
		virtual const char* GetWindowName() const { return "Unknown Window Name"; }

	protected:

		STools_Editor* myEditor;
		bool myIsOpen;
	};
}