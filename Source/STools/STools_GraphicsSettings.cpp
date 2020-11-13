#include "STools_Precompiled.h"
#include "STools_GraphicsSettings.h"

#include "SClient_Base.h"

#include "SG_View.h"

namespace Shift
{
	STools_GraphicsSettings::STools_GraphicsSettings()
	{
	}

	STools_GraphicsSettings::~STools_GraphicsSettings()
	{
	}

	void STools_GraphicsSettings::Render()
	{
		SClient_Base* client = STools_Editor::GetInstance()->GetClient();
		SG_View* view = client->GetView();

		if (ImGui::TreeNode("Ambient Occlusion"))
		{
			ImGui::Checkbox("Enable", &view->myAmbientOcclusionSettings.myEnabled);
			if (view->myAmbientOcclusionSettings.myEnabled)
			{
				const char* types[] = { "SSAO", "RTAO" };
				const char* currentType = types[view->myAmbientOcclusionSettings.myType];
				if (ImGui::BeginCombo("Type", currentType))
				{
					for (uint n = 0; n < 2; ++n)
					{
						bool isSelected = (currentType == types[n]);
						if (ImGui::Selectable(types[n], isSelected))
							view->myAmbientOcclusionSettings.myType = SG_View::AmbientOcclusionSettings::Type(n);

						if (isSelected)
							ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}

				if (view->myAmbientOcclusionSettings.myType == SG_View::AmbientOcclusionSettings::SSAO)
				{
					ImGui::DragFloat("Intensity", &view->myAmbientOcclusionSettings.mySSAOIntensity, 0.1f, 0.01f, 5.0f);
					ImGui::DragFloat("Bias", &view->myAmbientOcclusionSettings.mySSAOBias, 0.1f, 0.01f, 1.0f);
					ImGui::DragFloat("Sampling Radius", &view->myAmbientOcclusionSettings.mySSAOSamplingRadius, 0.1f, 0.01f, 4.0f);
					ImGui::DragFloat("Scale", &view->myAmbientOcclusionSettings.mySSAOScale, 0.1f, 0.01f, 4.0f);
				}
				else if (view->myAmbientOcclusionSettings.myType == SG_View::AmbientOcclusionSettings::RTAO)
				{
					ImGui::DragFloat("Radius", &view->myAmbientOcclusionSettings.myRTAORadius, 0.5f, 0.5f, 100.0f);
					int nRays = view->myAmbientOcclusionSettings.myRTAONumRaysPerPixel;
					ImGui::DragInt("Num Rays Per Pixel", &nRays, 1.0f, 1, 64);
					view->myAmbientOcclusionSettings.myRTAONumRaysPerPixel = (uint)nRays;
				}

			}


			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Global Illumination"))
		{
			const char* types[] = { "Disabled", "VXGI", "RTGI" };
			static const char* currentType = nullptr;

			if (ImGui::BeginCombo("Type", currentType))
			{
				for (uint n = 0; n < 3; ++n)
				{
					bool isSelected = (currentType == types[n]);
					if (ImGui::Selectable(types[n], isSelected))
						currentType = types[n];

					if (isSelected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
			ImGui::TreePop();
		}
		
		if (ImGui::TreeNode("Debug"))
		{
			ImGui::Checkbox("Light Heatmap Debug", &view->myDebugLightHeatmap);
			ImGui::TreePop();
		}
	}
}