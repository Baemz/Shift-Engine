#pragma once
#include "SC_RefCounted.h"

#include "SG_Model.h"
#include "SG_DirectionalLight.h"
#include "SG_DynamicSky.h"
#include "SG_Camera.h"

#include "SC_QuadTree.h"

namespace Shift
{
	class SG_View;
	class SG_RenderData;
	class SG_Terrain;
	class SG_Raytracer;

	class SG_World : public SC_RefCounted
	{
	public:
		bool myDebugDrawAABBs;
		bool myDebugDrawAllAABBs;
		bool myLockCullingCamera;
		bool myDrawCameraFrustum;
		static float myTessellationAmount; // TODO: REMOVE FROM SG_WORLD
		static float myDisplacementAmount; // TODO: REMOVE FROM SG_WORLD

		SG_World();
		~SG_World();

		bool Init();
		void Update();

		// Adds a new view to this world and returns a pointer.
		SC_Ref<SG_View> CreateView();
		bool PrepareView(SG_View* aView);

		void AddModel(SG_Model* aModel);
		void AddMeshes(SG_Model* aModel);
		void AddMesh(SG_MeshInstance* aMesh);
		void RemoveModel(SG_Model* aModel);
		void AddLight(SG_Light* aLight);
		void AddEffect();
		const SC_GrowingArray<SC_Ref<SG_MeshInstance>>& GetMeshes() const;


		SC_Ref<SG_DynamicSky> mySky;
		SC_UniquePtr<SG_Terrain> myTerrain;
		SC_UniquePtr<SG_Raytracer> myRaytracer;

	private:
		bool PrepareMeshes_Depth(SG_View* aView);
		bool PrepareMeshes_Gbuffer(SG_View* aView);
		bool PrepareMeshes_Forward(SG_View* aView);
		bool PrepareMeshes_Shadow(SG_View* aView);
		bool PrepareLights(SG_View* aView);

		// Quadtree here for spacial data.
		// Terrain lives here
		// Weather-controller lies here (sky, clouds, fog, daylight, timeofday etc.)
		// Particles controlled from here.

		SC_QuadTree<SC_Ref<SG_Model>> myQuadTree;

		SC_GrowingArray<SC_Ref<SG_View>> myViews;
		SC_GrowingArray<SG_Model*> myModels;

		SC_Mutex myMeshMutex;
		SC_Mutex myLightsMutex;
		SC_GrowingArray<SC_Ref<SG_MeshInstance>> myPendingMeshes;
		SC_GrowingArray<SC_Ref<SG_Light>> myPendingLights;
		SC_GrowingArray<SC_Ref<SG_MeshInstance>> myMeshes;
		SC_GrowingArray<SC_Ref<SG_Light>> myLights;

		SC_Ref<SR_Texture> _mySkyDiffuseTexture;
		SC_Ref<SR_Texture> _mySkySpecularTexture;
		SC_Ref<SR_Texture> _mySkyBRDFTexture;
		SG_Model mySphere;
		SG_DirectionalLight mySun;

		SG_Camera myLockedCullingCamera;

		// Tools
		SC_Vector3f myLightDirection;
		bool mySkyWindowOpen;
		bool myWasCameraLocked;
	};

}