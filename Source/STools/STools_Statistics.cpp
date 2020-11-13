#include "STools_Precompiled.h"
#include "STools_Statistics.h"

#include "SR_GraphicsDevice.h"

namespace Shift
{

	STools_Statistics::STools_Statistics()
	{

	}

	STools_Statistics::~STools_Statistics()
	{

	}

	void STools_Statistics::Render()
	{
		if (ImGui::TreeNode("Graphics Device"))
		{
			SR_GraphicsDeviceStats gfxStats = SR_GraphicsDevice::GetDevice()->GetStats();
			ImGui::Text("Descriptors (GPU): %u", gfxStats.myNumDescriptorsUsedGPU);
			ImGui::Text("Descriptors (CPU): %u", gfxStats.myNumDescriptorsUsedCPU);
			ImGui::Text("Descriptors (Staged): %u", gfxStats.myNumDescriptorsUsedStaged);

			if (gfxStats.myNumTrianglesDrawn > 1000)
			{
				if (gfxStats.myNumTrianglesDrawn >= 1000000)
					ImGui::Text("Triangles Drawn: %.2f M (%u)", gfxStats.myNumTrianglesDrawn / 1000000.f, gfxStats.myNumTrianglesDrawn);
				else
					ImGui::Text("Triangles Drawn: %u K (%u)", gfxStats.myNumTrianglesDrawn / 1000, gfxStats.myNumTrianglesDrawn);
			}
			else
				ImGui::Text("Triangles Drawn: %u", gfxStats.myNumTrianglesDrawn);

			ImGui::Text("Instances Drawn: %u", gfxStats.myNumInstancesDrawn);
			ImGui::Text("Draw Calls: %u", gfxStats.myNumDrawCalls);
			ImGui::Text("Dispatch Calls: %u", gfxStats.myNumDispatchCalls);
			ImGui::TreePop();
		}

		if (ImGui::TreeNode("__Other__"))
		{
			ImGui::Text("NOT USED");
			ImGui::TreePop();
		}

		if (ImGui::TreeNode("__Other2__"))
		{
			ImGui::Text("NOT USED");
			ImGui::TreePop();
		}
	}

}