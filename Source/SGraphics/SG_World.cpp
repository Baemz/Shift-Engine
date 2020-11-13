#include "SGraphics_Precompiled.h"
#include "SG_World.h"
#include "SG_View.h"
#include "SG_SceneGraph.h"
#include "SG_ViewManager.h"
#include "SG_Terrain.h"
#include "SG_MeshInstance.h"
#include "SG_Mesh.h"
#include "SG_Material.h"
#include "SG_CommonRenderStates.h"
#include "SG_Raytracer.h"

#include "SR_GraphicsDevice.h"

namespace Shift
{

	float SG_World::myTessellationAmount = 8.f;
	float SG_World::myDisplacementAmount = 0.f;

	SG_World::SG_World()
		: mySkyWindowOpen(true)
		, myQuadTree({ 2048.f, 2048.f}, 32)
		, myDebugDrawAABBs(false)
		, myDebugDrawAllAABBs(false)
		, myLockCullingCamera(false)
		, myWasCameraLocked(false)
		, myDrawCameraFrustum(false)
	{
	}


	SG_World::~SG_World()
	{
		for (uint i = 0; i < myViews.Count(); ++i)
		{
			if (SG_ViewManager* vm = SC_EngineInterface::GetViewManager())
				vm->RemoveView(myViews[i]);
		}
	}

	bool SG_World::Init()
	{
		SR_GraphicsDevice* device = SR_GraphicsDevice::GetDevice();

		_mySkyDiffuseTexture = device->GetCreateTexture("skyDiffuseHDR.dds");
		_mySkySpecularTexture = device->GetCreateTexture("skySpecularHDR.dds");
		_mySkyBRDFTexture = device->GetCreateTexture("skyBrdf.dds");


		//if (!Init_Objects())
		//	return false;

		mySun.SetDirection({0.1f, -1.f, 0.1f});
		mySky = new SG_DynamicSky();
		if (mySky)
			mySky->Init();

		//myTerrain = new SG_Terrain();

		//SG_TerrainInitData terrainInitData;
		//terrainInitData.myResolution = { 1024.f, 1024.f };
		//
		//myTerrain->Init(terrainInitData);

		if (SR_GraphicsDevice::GetDevice()->GetFeatureSupport().myEnableRaytracing)
		{
			myRaytracer = new SG_Raytracer(this);
			myRaytracer->Init();
		}

		return true;
	}

	SC_Ref<SG_View> SG_World::CreateView()
	{
		SC_Ref<SG_View> newView = myViews.Add(new SG_View(this));
		SC_EngineInterface::GetViewManager()->AddView(newView);
		return newView;
	}

	bool SG_World::PrepareView(SG_View* aView)
	{
		if (!aView)
			return false;

		SG_RenderData& prepareData = aView->GetPrepareData();

		if (myLockCullingCamera)
		{
			if (!myWasCameraLocked)
			{
				myWasCameraLocked = true;
				myLockedCullingCamera = prepareData.myMainCamera;
			}
		}
		else
			myWasCameraLocked = false;

		const SG_Camera& camera = (myLockCullingCamera) ? myLockedCullingCamera : prepareData.myMainCamera;

		if (myDrawCameraFrustum)
			camera.DebugDrawFrustum();

		if (myPendingMeshes.Count())
		{
			SC_MutexLock lock(myMeshMutex);
			myMeshes.AddN(myPendingMeshes);
			myPendingMeshes.RemoveAll();
		}
		if (myPendingLights.Count())
		{
			SC_MutexLock lock(myLightsMutex);
			myLights.AddN(myPendingLights);
			myPendingLights.RemoveAll();
		}

		prepareData.myViewWorldPrepareEvent_MeshesDepth = SC_CreateFrameTask(this, &SG_World::PrepareMeshes_Depth, aView);
		prepareData.myViewWorldPrepareEvent_MeshesGbuffer = SC_CreateFrameTask(this, &SG_World::PrepareMeshes_Gbuffer, aView);
		prepareData.myViewWorldPrepareEvent_MeshesForward = SC_CreateFrameTask(this, &SG_World::PrepareMeshes_Forward, aView);
		prepareData.myViewWorldPrepareEvent_MeshesShadow = SC_CreateFrameTask(this, &SG_World::PrepareMeshes_Shadow, aView);
		prepareData.myViewWorldPrepareEvent_Lights = SC_CreateFrameTask(this, &SG_World::PrepareLights, aView);

		if (myRaytracer)
			prepareData.myViewWorldPrepareEvent_UpdateRTScene = SC_CreateFrameTask(myRaytracer.Get(), &SG_Raytracer::PrepareScene, aView);

		return true;
	}

	void SG_World::AddModel(SG_Model* aModel)
	{
		myModels.Add(aModel);
	}

	void SG_World::AddMeshes(SG_Model* aModel)
	{
		if (aModel)
		{
			SC_MutexLock lock(myMeshMutex);
			myPendingMeshes.AddN(aModel->myMeshes);
		}
	}

	void SG_World::AddMesh(SG_MeshInstance* aMesh)
	{
		SC_MutexLock lock(myMeshMutex);
		myPendingMeshes.Add(aMesh);
	}

	void SG_World::RemoveModel(SG_Model* aModel)
	{
		myModels.RemoveCyclic(aModel);
	}

	void SG_World::AddLight(SG_Light* aLight)
	{
		SC_MutexLock lock(myLightsMutex);
		myPendingLights.Add(aLight);
	}

	const SC_GrowingArray<SC_Ref<SG_MeshInstance>>& SG_World::GetMeshes() const
	{
		return myMeshes;
	}

	bool SG_World::PrepareMeshes_Depth(SG_View* aView)
	{
		if (!aView)
			return false;

		SG_RenderData& prepareData = aView->GetPrepareData();
		SG_CommonRenderStates* crs = SG_CommonRenderStates::GetInstance();
		const SG_ViewSettings& viewSettings = aView->GetViewSettings();

		const SG_Camera& camera = (myLockCullingCamera) ? myLockedCullingCamera : prepareData.myMainCamera;

		int lodIndex = -1;
		for (SG_MeshInstance* mesh : myMeshes)
		{
			if (!mesh->IsLoaded())
				continue;

			if (mesh->CullAndReturnLod(camera, lodIndex))
			{
				SG_Material* material = mesh->GetMaterial();

				if (myDebugDrawAABBs && !myDebugDrawAllAABBs)
					mesh->DebugDraw();

				if (material->myProperties.myIsOpaque)
				{
					const SG_MeshLod& lod = mesh->myMeshTemplate->GetLod(lodIndex);

					SG_RenderQueueItem renderItem;
					renderItem.myLodIndex = lodIndex;
					renderItem.myVertexBuffer = lod.myVertexBuffer;
					renderItem.myIndexBuffer = lod.myIndexBuffer;
					renderItem.myNumIndices = lod.myNumIndices;

					const SC_Matrix44& meshTransform = mesh->GetTransform();
					const SC_Matrix44& cameraTransform = camera.GetTransform();

					float distanceToCamera = (meshTransform.GetPosition() - cameraTransform.GetPosition()).Length();
					renderItem.mySortDistance = distanceToCamera;
					renderItem.myTransform = meshTransform;

					if (material->myProperties.myIsTwoSided)
						renderItem.myRasterizerState = crs->myTwoSidedRasterizerState;
					else
						renderItem.myRasterizerState = crs->myDefaultRasterizerState;

					if (material->myProperties.myAlphaTest || material->myProperties.myUseTessellation)
						renderItem.myMaterial = material;
					else
						renderItem.myMaterial = nullptr;

					renderItem.myBlendState = crs->myDefaultBlendState;
					renderItem.myShader = material->myShaders[SG_RenderType_Depth];
					prepareData.myQueues.myDepth.myQueueItems.Add(renderItem);
				}
			}

			if (viewSettings.myUseVoxels)
			{
				if (mesh->GetMaterial()->myShaders[SG_RenderType_Voxelize]) // Also check if mesh is located inside current voxel scene AABB
				{
					const SG_MeshLod& lod = mesh->myMeshTemplate->GetLod(lodIndex);

					SG_RenderQueueItem renderItem;
					renderItem.myVertexBuffer = lod.myVertexBuffer;
					renderItem.myIndexBuffer = lod.myIndexBuffer;
					renderItem.myNumIndices = lod.myNumIndices;

					renderItem.myShader = mesh->GetMaterial()->myShaders[SG_RenderType_Voxelize];
					const SC_Matrix44& meshTransform = mesh->GetTransform();
					const SC_Matrix44& cameraTransform = camera.GetTransform();

					float distanceToCamera = (meshTransform.GetPosition() - cameraTransform.GetPosition()).Length();
					renderItem.mySortDistance = distanceToCamera;
					renderItem.myTransform = meshTransform;
					renderItem.myBlendState = crs->myDefaultBlendState;
					renderItem.myRasterizerState = crs->myDefaultRasterizerState;
					renderItem.myMaterial = mesh->GetMaterial();
					prepareData.myQueues.myVoxelize.myQueueItems.Add(renderItem);
				}
			}
		}

		prepareData.myQueues.myDepth.Prepare();

		if (viewSettings.myUseVoxels)
			prepareData.myQueues.myVoxelize.Prepare();

		return true;
	}

	bool SG_World::PrepareMeshes_Gbuffer(SG_View* aView)
	{
		if (!aView)
			return false;

		SG_RenderData& prepareData = aView->GetPrepareData();
		SG_CommonRenderStates* crs = SG_CommonRenderStates::GetInstance();

		const SG_Camera& camera = (myLockCullingCamera) ? myLockedCullingCamera : prepareData.myMainCamera;

		int lodIndex = -1;
		for (SG_MeshInstance* mesh : myMeshes)
		{
			if (!mesh->IsLoaded())
				continue;

			SG_Material* material = mesh->GetMaterial();
			if (!material->myProperties.myNeedsGBuffer)
				continue;

			if (mesh->CullAndReturnLod(camera, lodIndex))
			{
				const SG_MeshLod& lod = mesh->myMeshTemplate->GetLod(lodIndex);

				SG_RenderQueueItem renderItem;
				renderItem.myLodIndex = lodIndex;
				renderItem.myVertexBuffer = lod.myVertexBuffer;
				renderItem.myIndexBuffer = lod.myIndexBuffer;
				renderItem.myNumIndices = lod.myNumIndices;

				const SC_Matrix44& meshTransform = mesh->GetTransform();
				const SC_Matrix44& cameraTransform = camera.GetTransform();

				float distanceToCamera = (meshTransform.GetPosition() - cameraTransform.GetPosition()).Length();
				renderItem.mySortDistance = distanceToCamera;
				renderItem.myTransform = meshTransform;
				renderItem.myShader = material->myShaders[SG_RenderType_GBuffer];
				renderItem.myBlendState = crs->myDeferredBlendState;

				if (material->myProperties.myIsTwoSided)
					renderItem.myRasterizerState = crs->myTwoSidedRasterizerState;
				else
					renderItem.myRasterizerState = crs->myDefaultRasterizerState;

				renderItem.myMaterial = material;
				prepareData.myQueues.myGBuffers[0].myQueueItems.Add(renderItem);
			}
		}

		prepareData.myQueues.myGBuffers[0].Prepare();

		return true;
	}

	bool SG_World::PrepareMeshes_Forward(SG_View* aView)
	{
		if (!aView)
			return false;

		SG_RenderData& prepareData = aView->GetPrepareData();
		SG_CommonRenderStates* crs = SG_CommonRenderStates::GetInstance();

		const SG_Camera& camera = (myLockCullingCamera) ? myLockedCullingCamera : prepareData.myMainCamera;

		int lodIndex = -1;
		for (SG_MeshInstance* mesh : myMeshes)
		{
			if (!mesh->IsLoaded())
				continue;

			SG_Material* material = mesh->GetMaterial();
			if (!material->myProperties.myNeedsForward)
				continue;

			if (mesh->CullAndReturnLod(camera, lodIndex))
			{
				const SG_MeshLod& lod = mesh->myMeshTemplate->GetLod(lodIndex);

				SG_RenderQueueItem renderItem;
				renderItem.myLodIndex = lodIndex;
				renderItem.myVertexBuffer = lod.myVertexBuffer;
				renderItem.myIndexBuffer = lod.myIndexBuffer;
				renderItem.myNumIndices = lod.myNumIndices;

				const SC_Matrix44& meshTransform = mesh->GetTransform();
				const SC_Matrix44& cameraTransform = camera.GetTransform();

				float distanceToCamera = (meshTransform.GetPosition() - cameraTransform.GetPosition()).Length();
				renderItem.mySortDistance = distanceToCamera;
				renderItem.myTransform = meshTransform;
				renderItem.myShader = material->myShaders[SG_RenderType_Color];
				renderItem.myBlendState = crs->myDefaultBlendState;

				if (material->myProperties.myIsTwoSided)
					renderItem.myRasterizerState = crs->myTwoSidedRasterizerState;
				else
					renderItem.myRasterizerState = crs->myDefaultRasterizerState;

				renderItem.myMaterial = material;
				prepareData.myQueues.myForwardOpaque.myQueueItems.Add(renderItem);
			}
		}

		prepareData.myQueues.myForwardOpaque.Prepare();

		return true;
	}

	bool SG_World::PrepareMeshes_Shadow(SG_View* aView)
	{
		if (!aView)
			return false;

		SG_RenderData& prepareData = aView->GetPrepareData();
		SG_CommonRenderStates* crs = SG_CommonRenderStates::GetInstance();
		prepareData.myPrepareShadowProjections.Wait();

		for (SG_MeshInstance* mesh : myMeshes)
		{
			if (!mesh->IsLoaded())
				continue;

			SG_Material* material = mesh->GetMaterial();
			if (material->myProperties.myIsShadowCaster)
			{
				int lodIndex = -1;
				for (uint cascade = 0; cascade < SG_Shadows::ourNumShadowCascades; ++cascade)
				{
					const SG_Camera& camera = prepareData.myShadowLightCameras[cascade];
					if (mesh->CullAndReturnLod(camera, lodIndex))
					{
						const SG_MeshLod& lod = mesh->myMeshTemplate->GetLod(lodIndex);

						SG_RenderQueueItem& renderItem = prepareData.myCSMQueues[cascade].myDepth.myQueueItems.Add();
						renderItem.myLodIndex = lodIndex;
						renderItem.myVertexBuffer = lod.myVertexBuffer;
						renderItem.myIndexBuffer = lod.myIndexBuffer;
						renderItem.myNumIndices = lod.myNumIndices;

						const SC_Matrix44& meshTransform = mesh->GetTransform();
						const SC_Matrix44& cameraTransform = camera.GetTransform();

						float distanceToCamera = (meshTransform.GetPosition() - cameraTransform.GetPosition()).Length();
						renderItem.mySortDistance = distanceToCamera;
						renderItem.myTransform = meshTransform;
						renderItem.myShader = material->myShaders[SG_RenderType_Depth];
						renderItem.myBlendState = crs->myDefaultBlendState;

						if (material->myProperties.myIsTwoSided)
							renderItem.myRasterizerState = crs->myCSMTwoSidedRasterizerState;
						else
							renderItem.myRasterizerState = crs->myCSMRasterizerState;

						if (material->myProperties.myAlphaTest || material->myProperties.myUseTessellation)
							renderItem.myMaterial = material;
						else
							renderItem.myMaterial = nullptr;
					}
				}
			}
		}

		for (uint cascade = 0; cascade < SG_Shadows::ourNumShadowCascades; ++cascade)
			prepareData.myCSMQueues[cascade].myDepth.Prepare();

		return true;
	}

	bool SG_World::PrepareLights(SG_View* aView)
	{
		if (!aView)
			return false;

		SG_RenderData& prepareData = aView->GetPrepareData();
		for (SG_Light* light : myLights)
		{
			if (!light->IsLoaded())
				continue;

			// Do initial culling here
			if (light->GetType() != SG_LightType_Directional && light->GetType() != SG_LightType_Unknown)
			{
				SG_Light::GPUData gpuData = light->GetGPUData();
				prepareData.myLights.Add(gpuData);
				//SG_PRIMITIVE_DRAW_LINE3D_COLORED(gpuData.myPositionAndRange.XYZ(), gpuData.myPositionAndRange.XYZ() + (SC_Vector3f(0.f, 1.f, 0.f) * gpuData.myPositionAndRange.w), SC_Vector4f(gpuData.myColorAndIntensity.XYZ(), 1.0f));
				//SG_PRIMITIVE_DRAW_LINE3D_COLORED(gpuData.myPositionAndRange.XYZ(), gpuData.myPositionAndRange.XYZ() + (SC_Vector3f(0.f, -1.f, 0.f) * gpuData.myPositionAndRange.w), SC_Vector4f(gpuData.myColorAndIntensity.XYZ(), 1.0f));
				//SG_PRIMITIVE_DRAW_LINE3D_COLORED(gpuData.myPositionAndRange.XYZ(), gpuData.myPositionAndRange.XYZ() + (SC_Vector3f(1.f, 0.f, 0.f) * gpuData.myPositionAndRange.w), SC_Vector4f(gpuData.myColorAndIntensity.XYZ(), 1.0f));
				//SG_PRIMITIVE_DRAW_LINE3D_COLORED(gpuData.myPositionAndRange.XYZ(), gpuData.myPositionAndRange.XYZ() + (SC_Vector3f(-1.f, 0.f, 0.f) * gpuData.myPositionAndRange.w), SC_Vector4f(gpuData.myColorAndIntensity.XYZ(), 1.0f));
				//SG_PRIMITIVE_DRAW_LINE3D_COLORED(gpuData.myPositionAndRange.XYZ(), gpuData.myPositionAndRange.XYZ() + (SC_Vector3f(0.f, 0.f, 1.f) * gpuData.myPositionAndRange.w), SC_Vector4f(gpuData.myColorAndIntensity.XYZ(), 1.0f));
				//SG_PRIMITIVE_DRAW_LINE3D_COLORED(gpuData.myPositionAndRange.XYZ(), gpuData.myPositionAndRange.XYZ() + (SC_Vector3f(0.f, 0.f, -1.f) * gpuData.myPositionAndRange.w), SC_Vector4f(gpuData.myColorAndIntensity.XYZ(), 1.0f));
			}
		}

		return true;
	}

	void SG_World::Update()
	{
		mySky->Update(0.f);
		// Update player
		// Stream GameObjects
		// Update Terrain

		mySun.SetDirection(-(mySky->GetToSunDirection()));

		if (myDebugDrawAABBs && myDebugDrawAllAABBs)
		{
			for (SG_MeshInstance* mesh : myMeshes)
			{
				if (!mesh->IsLoaded())
					continue;

				mesh->DebugDraw();
			}
		}
	}
}