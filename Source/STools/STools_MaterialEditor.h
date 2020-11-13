#pragma once
#include "STools_EditorSubModule.h"
#include "SG_Material.h"

namespace ImGui
{
	class FileBrowser;
}

namespace Shift
{
	class STools_MaterialEditor final : public STools_EditorSubModule
	{
		STOOLS_DECLARESUBMODULE(STools_MaterialEditor);
	public:
		STools_MaterialEditor();
		~STools_MaterialEditor();

		void Render() override;

		const char* GetWindowName() const override { return "Material Editor"; }

	private:
		SC_UniquePtr<ImGui::FileBrowser> myBrowser;
		std::string mySelectedFile;
		std::string myMaterialSaveLoc;
		SG_MaterialDesc myMaterialDesc;

		int mySelectedTexture;
		bool mySaveSuccess;
	};
}
