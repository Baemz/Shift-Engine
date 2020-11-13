#include "STools_Precompiled.h"
#include "STools_GraphicsWorld.h"

#include "SG_World.h"
#include "SG_Terrain.h"

namespace Shift
{
	STools_GraphicsWorld::STools_GraphicsWorld(SG_World* aWorld)
		: myWorld(aWorld)
		, myEnableTerrain(false)
	{
	}

	STools_GraphicsWorld::~STools_GraphicsWorld()
	{
	}

	void STools_GraphicsWorld::Update()
	{
		STools_EditorSubModule::Update();

		if (myEnableTerrain && !myWorld->myTerrain)
		{
			myWorld->myTerrain = new SG_Terrain();

			SG_TerrainInitData terrainInitData;
			terrainInitData.myResolution = { 1024.f, 1024.f };

			myWorld->myTerrain->Init(terrainInitData);
		}
		else if (!myEnableTerrain && myWorld->myTerrain)
			myWorld->myTerrain.Reset();
	}

	void STools_GraphicsWorld::Render()
	{
		ImGui::Checkbox("Enable Terrain", &myEnableTerrain);
		ImGui::Checkbox("Draw AABBs", &myWorld->myDebugDrawAABBs);
		if (myWorld->myDebugDrawAABBs)
			ImGui::Checkbox("Draw All AABBs", &myWorld->myDebugDrawAllAABBs);

		ImGui::Checkbox("Draw Camera Frustum", &myWorld->myDrawCameraFrustum);
		ImGui::Checkbox("Lock Culling Camera", &myWorld->myLockCullingCamera);
		ImGui::DragFloat("Tessellation", &myWorld->myTessellationAmount, 1.0f, 1.0f, 25.f);
		ImGui::DragFloat("Displacement", &myWorld->myDisplacementAmount, 0.01f);
	}
}