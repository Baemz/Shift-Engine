#include "STools_Precompiled.h"
#include "STools_MaterialEditor.h"
#include "SG_Material.h"

#include "imguifilebrowser.h"

namespace Shift
{
	STools_MaterialEditor::STools_MaterialEditor()
		: mySaveSuccess(false)
		, mySelectedTexture(-1)
	{
		myBrowser = new ImGui::FileBrowser();
		myBrowser->SetTitle("Asset Browser");
	}

	STools_MaterialEditor::~STools_MaterialEditor()
	{
	}
	void STools_MaterialEditor::Render()
	{
		if (ImGui::Button("Open..."))
		{
			mySelectedFile.clear();
			myBrowser->SetTypeFilters({ ".smtf" });
			myBrowser->Open();
		}

		myBrowser->Display();

		if (mySelectedFile.empty() && myBrowser->HasSelected())
		{
			mySelectedFile = myBrowser->GetSelected().string();
			myBrowser->ClearSelected();

			myMaterialDesc.FromFile(mySelectedFile.c_str());
			mySaveSuccess = false;
		}

		ImGui::NewLine();
		ImGui::Text("Material File: %s", GetFileWithExtension(mySelectedFile).c_str());
		myMaterialDesc.myVertexShader.reserve(256);
		ImGui::InputText(" :Vertex Shader", myMaterialDesc.myVertexShader.data(), 256);
		myMaterialDesc.myPixelShader.reserve(256);
		ImGui::InputText(" :Pixel Shader", myMaterialDesc.myPixelShader.data(), 256);

		ImGui::Checkbox("Is Opaque", &myMaterialDesc.myIsOpaque);
		ImGui::Checkbox("Needs Forward", &myMaterialDesc.myNeedsForward);
		ImGui::Checkbox("Needs GBuffer", &myMaterialDesc.myNeedsGBuffer);

		uint numTextures = myMaterialDesc.myTextures.Count();
		if (numTextures > 0)
		{ 
			SC_GrowingArray<const char*> textures;
			textures.PreAllocate(numTextures);
			for (uint i = 0; i < numTextures; ++i)
				textures.Add(myMaterialDesc.myTextures[i].c_str());

			ImGui::ListBox(" :Textures", &mySelectedTexture, textures.GetBuffer(), numTextures, SC_Min(textures.Count(), 6));

			if (mySelectedTexture >= 0)
			{
				ImGui::InputText(" :Texture", myMaterialDesc.myTextures[mySelectedTexture].data(), 256);
			}
		}

		ImGui::NewLine();
		if (ImGui::Button("Save"))
		{
			mySaveSuccess = myMaterialDesc.ToFile(mySelectedFile.c_str());
		}
		if (ImGui::Button("Save As"))
		{
			ImGui::OpenPopup("##MaterialSaveDialog");
			//mySaveSuccess = myMaterialDesc.ToFile(mySelectedFile.c_str());
		}
		if (ImGui::IsPopupOpen(ImGui::GetID("##MaterialSaveDialog")))
		{
			ImGui::BeginPopup("##MaterialSaveDialog");
			myMaterialSaveLoc.reserve(256);
			ImGui::InputText(":Material File", myMaterialSaveLoc.data(), 256);
			if (ImGui::Button("Ok"))
			{
				mySaveSuccess = myMaterialDesc.ToFile(myMaterialSaveLoc.c_str());
				ImGui::CloseCurrentPopup();
			}
			else if (ImGui::Button("Cancel"))
				ImGui::CloseCurrentPopup();
			ImGui::EndPopup();
		}
		if (mySaveSuccess)
			ImGui::TextColored({ 0.0f, 1.0f, 0.0f, 1.0f }, "Saved Sccessfully!");
	}
}