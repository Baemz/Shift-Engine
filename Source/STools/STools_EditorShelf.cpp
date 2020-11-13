#include "STools_Precompiled.h"
#include "STools_EditorShelf.h"
#include "SC_EngineInterface.h"

Shift::STools_EditorShelf::STools_EditorShelf()
{
}

Shift::STools_EditorShelf::~STools_EditorShelf()
{
}

void Shift::STools_EditorShelf::Render()
{
	ImGui::BeginMainMenuBar();

	if (ImGui::BeginMenu("File"))
	{
		if (ImGui::MenuItem("Close", "Ctrl+W")) { SC_EngineInterface::Exit(); }
		ImGui::EndMenu();
	}

	ImGui::EndMainMenuBar();
}
