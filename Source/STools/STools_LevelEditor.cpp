#include "STools_Precompiled.h"
#include "STools_LevelEditor.h"

#include "SG_World.h"
#include "SG_View.h"
#include "SG_ModelLoader.h"
#include "SG_Model.h"
#include "SG_Mesh.h"
#include "SG_Pointlight.h"

#include "SR_GraphicsDevice.h"

namespace Shift
{
	STools_LevelEditor::STools_LevelEditor()
	{
		myWorld = new SG_World();
		myView = myWorld->CreateView();
		//myView2 = myWorld->CreateView();

		if (!myWorld->Init())
			assert(false);

		const SC_Vector2f resolution = SR_GraphicsDevice::GetDevice()->GetResolution();
		myCamera.InitAsPerspective(resolution, 75.0f, 0.01f, 5000.f);
		myCamera.SetPosition(SC_Vector3f(-4.0f, 2.0f, 0.0f));
		myCamera.LookAt(SC_Vector3f(0.0f, 1.0f, 0.0f));
		myView->SetCamera(myCamera);


		//myCamera2.InitAsPerspective(resolution, 75.0f, 0.01f, 5000.f);
		//myCamera2.SetPosition(SC_Vector3f(4.0f, 2.0f, 0.0f));
		//myCamera2.LookAt(SC_Vector3f(0.0f, 1.0f, 0.0f));
		//myView2->SetCamera(myCamera2);

		LoadLevel("Assets/Levels/TestLevel.smap");
	}

	STools_LevelEditor::~STools_LevelEditor()
	{
	}

	void STools_LevelEditor::Update()
	{
		// Update camera controls here

		// Check for screen picking

		// Highlight meshes

		for (auto& obj : myObjects)
		{
			SC_Vector3f pos = obj->myModel->myTransform.GetPosition();
			obj->myLocator.Interact(pos, myRelativeCursorPos);
			obj->myModel->myTransform.SetPosition(pos);
			obj->myModel->SetTransform(obj->myModel->myTransform);
		}


		myWorld->Update();
		myView->SetCamera(myCamera);
	}

	void STools_LevelEditor::Render()
	{
		for (auto& obj : myObjects)
		{
			obj->myLocator.Render();
		}

		const char* items[] = { "Lit", "Unlit", "Depth", "AO", "Normals", "Bloom", "Luminance" };
		static const char* current_item = items[0];

		if (ImGui::BeginCombo("##vpDisplayCombo", current_item)) // The second parameter is the label previewed before opening the combo.
		{
			for (int n = 0; n < IM_ARRAYSIZE(items); n++)
			{
				bool is_selected = (current_item == items[n]); // You can store your selection however you want, outside or inside your objects
				if (ImGui::Selectable(items[n], is_selected))
				{
					current_item = items[n];
					myVPDisplayMode = ViewportDisplayMode(n);
					if (is_selected)
						ImGui::SetItemDefaultFocus();   // You may set the initial focus when opening the combo (scrolling + for keyboard navigation support)
				}
			}
			ImGui::EndCombo();
		}

		SR_Texture* viewportTexture = myView->GetLastFinishedFrame();
		switch (myVPDisplayMode)
		{
		default:
		case ViewportDisplayMode::Lit:
			break;
		case ViewportDisplayMode::Unlit:
			viewportTexture = myView->myFrameResources.myGBuffer.myColors.myTexture;
			break;
		case ViewportDisplayMode::Depth:
			viewportTexture = myView->myFrameResources.myDepth.myTexture;
			break;
		case ViewportDisplayMode::AO:
			viewportTexture = myView->myFrameResources.myAmbientOcclusion.myTexture;
			break;
		case ViewportDisplayMode::GbufferNormals:
			viewportTexture = myView->myFrameResources.myGBuffer.myNormals.myTexture;
			break;
		case ViewportDisplayMode::Bloom:
			viewportTexture = myView->myFrameResources.myBloomTexture;
			break;
		case ViewportDisplayMode::Luminance:
			viewportTexture = myView->myFrameResources.myAvgLuminance.myTexture;
			break;
		}

		ImVec2 viewportSize = SR_ImGUI::GetCurrentWindowClientSize();
		ImGui::Image(viewportTexture, viewportSize);
		myRelativeCursorPos = SR_ImGUI::GetCurrentRelativeCursorPos();
	}

	bool STools_LevelEditor::LoadLevel(const char* aLevelFile)
	{
		std::ifstream levelFile(aLevelFile);

		SC_Json levelData;
		if (levelFile.is_open())
		{
			SG_ModelLoader loader;
			SC_Stopwatch timer;
			levelFile >> levelData;
			levelFile.close();

			std::string name;

			if (levelData.find("Name") != levelData.end())
				name = levelData["Name"].get<std::string>();
			else
				name = "Unnamed Level";


			if (levelData.find("Objects") != levelData.end())
			{
				for (const SC_Json& object : levelData["Objects"])
				{
					std::string objFile;
					objFile = object["File"].get<std::string>();

					std::ifstream objFileStream(objFile);
					if (!objFileStream.is_open())
						continue;

					SC_Json gameObjectData;
					objFileStream >> gameObjectData;
					objFileStream.close();

					SC_Json gameObjectMeshData = gameObjectData["Meshes"][0];
					std::string gameObjectMeshPath = gameObjectMeshData["Mesh"].get<std::string>();
					std::string gameObjectMeshMaterialPath = gameObjectMeshData["Material"].get<std::string>();

					std::string objName;
					objName = object["Name"].get<std::string>();

					SC_Vector3f position;
					position.x = object["Position"]["x"].get<float>();
					position.y = object["Position"]["y"].get<float>();
					position.z = object["Position"]["z"].get<float>();

					SC_Vector3f rotation;
					rotation.x = object["Rotation"]["x"].get<float>();
					rotation.y = object["Rotation"]["y"].get<float>();
					rotation.z = object["Rotation"]["z"].get<float>();

					SC_Vector3f scale;
					scale.x = object["Scale"]["x"].get<float>();
					scale.y = object["Scale"]["y"].get<float>();
					scale.z = object["Scale"]["z"].get<float>();

					SG_MeshCreateData meshCreateData;
					// TODO: GET PROPER CREATE DATA HERE

					SC_Ref<SG_MeshInstance> mesh = SG_MeshInstance::Create(meshCreateData);
					if (mesh)
					{
						SC_Matrix44 mat = mesh->GetTransform();
						mat.Scale(scale);
						mat.SetPosition(position);
						mesh->SetTransform(mat);
						myWorld->AddMesh(mesh);

						TEMP_Object* tempObj = new TEMP_Object(myCamera);
						tempObj->myModel = mesh;
						myObjects.Add(tempObj);
					}
				}
			}
			else
			{
				SC_ERROR_LOG("Level file doesn't containt 'Objects' specifier.");
				return false;
			}

			if (levelData.find("Lights") != levelData.end())
			{
				for (const SC_Json& light : levelData["Lights"])
				{
					std::string type;
					type = light["Type"].get<std::string>();

					if (type == "Pointlight")
					{
						SC_Ref<SG_Pointlight> pl = new SG_Pointlight();

						SC_Vector3f color;
						color.x = light["Color"]["r"].get<float>();
						color.y = light["Color"]["g"].get<float>();
						color.z = light["Color"]["b"].get<float>();
						pl->SetColor(color);

						pl->SetIntensity(light["Intensity"].get<float>());
						pl->SetRange(light["Range"].get<float>());

						SC_Vector3f position;
						position.x = light["Position"]["x"].get<float>();
						position.y = light["Position"]["y"].get<float>();
						position.z = light["Position"]["z"].get<float>();
						pl->SetPosition(position);

						myWorld->AddLight(pl);
						myLights.Add(pl);
					}
					else if (type == "Spotlight")
					{

					}
				}

			}
			else
			{
				SC_ERROR_LOG("Level file doesn't containt 'Lights' specifier.");
				return false;
			}


			SC_LOG("Finished loading level (%s) - elapsed time: %.3f ms", aLevelFile, timer.Stop());
			return true;
		}
		else
			SC_ERROR_LOG("Could not open level (%s)", aLevelFile);

		return false;
	}
}