#include "STools_Precompiled.h"
#include "STools_TextureViewer.h"
#include "SClient_Base.h"
#include "SG_View.h"
#include "SG_Shadows.h"
#include "SG_World.h"
#include "SG_Raytracer.h"

namespace Shift
{

	STools_TextureViewer::STools_TextureViewer()
	{

	}

	STools_TextureViewer::~STools_TextureViewer()
	{

	}

	void STools_TextureViewer::Render()
	{
		SG_View* view = myEditor->GetClient()->GetView();

		// List of available textures
		// Render only one texture at the time
		// Make rendered texture scale to window size
		// Scrolling zooms in and out


		if (ImGui::TreeNode("CSM Shadows"))
		{
			for (uint i = 0; i < 4; i++)
			{
				SR_Texture* csm = view->myShadows->GetSunShadowMapSlice(i);
				ImGui::Image(csm, ImVec2(128, 128), ImVec2(0, 0), ImVec2(1, 1));

				if (i < 3)
					ImGui::SameLine();
			}
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Fullscreen Textures"))
		{
			SR_Texture* tex = view->myFrameResources.myFullscreenHDR.myTexture;
			ImGui::Image(tex, ImVec2(240, 135));
			ImGui::SameLine();

			tex = view->myFrameResources.myAmbientOcclusion.myTexture;
			ImGui::Image(tex, ImVec2(240, 135));

			tex = view->myFrameResources.myBloomTexture;
			ImGui::Image(tex, ImVec2(240, 135));
			ImGui::SameLine();

			tex = view->myFrameResources.myGBuffer.myColors.myTexture;
			ImGui::Image(tex, ImVec2(240, 135));

			tex = view->myFrameResources.myGBuffer.myNormals.myTexture;
			ImGui::Image(tex, ImVec2(240, 135));
			ImGui::SameLine();

			tex = view->myFrameResources.myGBuffer.myMaterials.myTexture;
			ImGui::Image(tex, ImVec2(240, 135));

			tex = view->myFrameResources.myDepth.myTexture;
			ImGui::Image(tex, ImVec2(240, 135));
			ImGui::SameLine();

			tex = view->myFrameResources.myDepthLinear.myTexture;
			ImGui::Image(tex, ImVec2(240, 135));

			if (view->GetViewSettings().myEnableRaytracing && view->myWorld->myRaytracer)
			{
				tex = view->myWorld->myRaytracer->GetResultTexture();
				ImGui::Image(tex, ImVec2(240, 135));
			}

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("PostFX Misc"))
		{
			SR_Texture* tex = view->myFrameResources.myAvgLuminance.myTexture;
			ImGui::Image(tex, ImVec2(128, 128));
			ImGui::SameLine();

			ImGui::TreePop();
		}
		//ImGuiIO& io = ImGui::GetIO();
		//ImGui::TextWrapped("Below we are displaying the font texture (which is the only texture we have access to in this demo). Use the 'ImTextureID' type as storage to pass pointers or identifier to your own texture data. Hover the texture for a zoomed view!");
		//
		//// Here we are grabbing the font texture because that's the only one we have access to inside the demo code.
		//// Remember that ImTextureID is just storage for whatever you want it to be, it is essentially a value that will be passed to the render function inside the ImDrawCmd structure.
		//// If you use one of the default imgui_impl_XXXX.cpp renderer, they all have comments at the top of their file to specify what they expect to be stored in ImTextureID.
		//// (for example, the imgui_impl_dx11.cpp renderer expect a 'ID3D11ShaderResourceView*' pointer. The imgui_impl_opengl3.cpp renderer expect a GLuint OpenGL texture identifier etc.)
		//// If you decided that ImTextureID = MyEngineTexture*, then you can pass your MyEngineTexture* pointers to ImGui::Image(), and gather width/height through your own functions, etc.
		//// Using ShowMetricsWindow() as a "debugger" to inspect the draw data that are being passed to your render will help you debug issues if you are confused about this.
		//// Consider using the lower-level ImDrawList::AddImage() API, via ImGui::GetWindowDrawList()->AddImage().
		//
		//
		//ImTextureID my_tex_id = io.Fonts->TexID;
		//float my_tex_w = (float)io.Fonts->TexWidth;
		//float my_tex_h = (float)io.Fonts->TexHeight;
		//
		//ImGui::Text("%.0fx%.0f", my_tex_w, my_tex_h);
		//ImVec2 pos = ImGui::GetCursorScreenPos();
		//ImGui::Image(my_tex_id, ImVec2(my_tex_w, my_tex_h), ImVec2(0, 0), ImVec2(1, 1), ImVec4(1.0f, 1.0f, 1.0f, 1.0f), ImVec4(1.0f, 1.0f, 1.0f, 0.5f));
		//if (ImGui::IsItemHovered())
		//{
		//	ImGui::BeginTooltip();
		//	float region_sz = 32.0f;
		//	float region_x = io.MousePos.x - pos.x - region_sz * 0.5f; if (region_x < 0.0f) region_x = 0.0f; else if (region_x > my_tex_w - region_sz) region_x = my_tex_w - region_sz;
		//	float region_y = io.MousePos.y - pos.y - region_sz * 0.5f; if (region_y < 0.0f) region_y = 0.0f; else if (region_y > my_tex_h - region_sz) region_y = my_tex_h - region_sz;
		//	float zoom = 4.0f;
		//	ImGui::Text("Min: (%.2f, %.2f)", region_x, region_y);
		//	ImGui::Text("Max: (%.2f, %.2f)", region_x + region_sz, region_y + region_sz);
		//	ImVec2 uv0 = ImVec2((region_x) / my_tex_w, (region_y) / my_tex_h);
		//	ImVec2 uv1 = ImVec2((region_x + region_sz) / my_tex_w, (region_y + region_sz) / my_tex_h);
		//	ImGui::Image(my_tex_id, ImVec2(region_sz * zoom, region_sz * zoom), uv0, uv1, ImVec4(1.0f, 1.0f, 1.0f, 1.0f), ImVec4(1.0f, 1.0f, 1.0f, 0.5f));
		//	ImGui::EndTooltip();
		//}
		//
		//ImGui::TextWrapped("And now some textured buttons..");
	}

}