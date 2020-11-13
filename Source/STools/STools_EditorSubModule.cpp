#include "STools_Precompiled.h"
#include "STools_EditorSubModule.h"

namespace Shift
{
	STools_EditorSubModule::STools_EditorSubModule()
		: myEditor(STools_Editor::GetInstance())
		, myIsOpen(true)
	{
	}

	STools_EditorSubModule::~STools_EditorSubModule()
	{
	}

	void STools_EditorSubModule::UpdateModule()
	{
		if (myIsOpen == false)
			UnregisterTool();

		Update();
	}

	void STools_EditorSubModule::RenderModule()
	{
		PreRender();
		ImGui::Begin(GetWindowName(), &myIsOpen);
		Render();
		ImGui::End();
		PostRender();
	}

	void STools_EditorSubModule::UnregisterTool()
	{
		myEditor->UnregisterTool(this);
	}
}