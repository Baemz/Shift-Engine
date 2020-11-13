#include "SGraphics_Precompiled.h"
#include "SG_MeshInstance.h"
#include "SG_MeshHeader.h"
#include "SG_Mesh.h"
#include "SG_MaterialFactory.h"
#include "SG_Camera.h"
#include "SG_LodPresets.h"

namespace Shift
{
	SG_MeshInstance::SG_MeshInstance(SG_Mesh* aMesh)
		: myMeshTemplate(aMesh)
	{
		CalcBounds();
	}

	SG_MeshInstance::~SG_MeshInstance()
	{
	}

	SC_Ref<SG_MeshInstance> SG_MeshInstance::Create(const SG_MeshHeader& aHeader, void* aVertexData, void* aIndexData, const char* aMaterialPath, const char* aName)
	{
		SC_Ref<SG_Mesh> mesh = SG_Mesh::Create(aHeader, aVertexData, aIndexData, aName);
		SC_Ref<SG_MeshInstance> meshInstance = new SG_MeshInstance(mesh);
		meshInstance->SetState(SC_LoadState::Loading);

		if (aMaterialPath && aMaterialPath[0] != '\0')
			meshInstance->myMaterial = SG_MaterialFactory::GetCreateMaterial(aMaterialPath);
		else
			meshInstance->myMaterial = SG_MaterialFactory::GetCreateMaterial("Assets/Materials/DefaultDeferred.smtf");

		meshInstance->SetState(SC_LoadState::Loaded);
		return meshInstance;
	}

	SC_Ref<SG_MeshInstance> SG_MeshInstance::Create(const SG_MeshCreateData& aCreateData)
	{
		SC_Ref<SG_Mesh> mesh = SG_Mesh::Create(aCreateData);

		SC_Ref<SG_MeshInstance> meshInstance = new SG_MeshInstance(mesh);
		meshInstance->SetState(SC_LoadState::Loading);

		if (!aCreateData.myMaterialPath.empty())
			meshInstance->myMaterial = SG_MaterialFactory::GetCreateMaterial(aCreateData.myMaterialPath.c_str());
		else
			meshInstance->myMaterial = SG_MaterialFactory::GetCreateMaterial("Assets/Materials/DefaultDeferred.smtf");

		meshInstance->SetState(SC_LoadState::Loaded);

		return meshInstance;
	}

	void SG_MeshInstance::SetTransform(const SC_Matrix44& aTransform)
	{
		myTransform = aTransform;
		CalcBounds();
	}

	const SC_Matrix44& SG_MeshInstance::GetTransform() const
	{
		return myTransform;
	}

	const SC_AABB& SG_MeshInstance::GetBoundingBox() const
	{
		return myBoundingBox;
	}
	const SC_Sphere& SG_MeshInstance::GetBoundingSphere() const
	{
		return myBoundingSphere;
	}
	bool SG_MeshInstance::CullAndReturnLod(const SG_Camera& aCamera)
	{
		int unused = 0;
		return CullAndReturnLod(aCamera, unused);
	}

	bool SG_MeshInstance::CullAndReturnLod(const SG_Camera& aCamera, int& aLodOut)
	{
		const SG_Frustum& frustum = aCamera.GetFrustum();

		if (frustum.Intersects(myBoundingSphere))
		{
			if (frustum.Intersects(myBoundingBox))
			{
				const SG_LodPreset& lodPreset = SG_LodPresets::GetPreset(myMaterial->GetLodPresetId());

				SC_AABB aabb = myBoundingBox;
				SC_Vector3f corners[8];

				// Near face
				corners[0] = { aabb.myMin };
				corners[1] = { aabb.myMin.x, aabb.myMax.y, aabb.myMin.z };
				corners[2] = { aabb.myMax.x, aabb.myMax.y, aabb.myMin.z };
				corners[3] = { aabb.myMax.x, aabb.myMin.y, aabb.myMin.z };

				// Far face
				corners[4] = { aabb.myMin.x, aabb.myMin.y, aabb.myMax.z };
				corners[5] = { aabb.myMin.x, aabb.myMax.y, aabb.myMax.z };
				corners[6] = { aabb.myMax };
				corners[7] = { aabb.myMax.x, aabb.myMin.y, aabb.myMax.z };

				corners[0] = aCamera.Project(corners[0]);
				corners[1] = aCamera.Project(corners[1]);
				corners[2] = aCamera.Project(corners[2]);
				corners[3] = aCamera.Project(corners[3]);
				corners[4] = aCamera.Project(corners[4]);
				corners[5] = aCamera.Project(corners[5]);
				corners[6] = aCamera.Project(corners[6]);
				corners[7] = aCamera.Project(corners[7]);

				SC_Vector2f min(SC_FLT_MAX);
				SC_Vector2f max(SC_FLT_LOWEST);

				for (uint i = 0; i < 8; ++i)
				{
					min.x = SC_Min(corners[i].x, min.x);
					min.y = SC_Min(corners[i].y, min.y);
					max.x = SC_Max(corners[i].x, max.x);
					max.y = SC_Max(corners[i].y, max.y);
				}

				SC_Vector2f rectDimensions;
				rectDimensions.x = fabs(max.x - min.x);
				rectDimensions.y = fabs(max.y - min.y);

				float area = rectDimensions.x * rectDimensions.y;

				//float distanceToCamera = (aCamera.GetPosition() - myTransform.GetPosition()).Length();
				//area += distanceToCamera;

				for (uint i = 0; i < lodPreset.myPercentageThresholds.Count(); ++i)
				{
					if (i == lodPreset.myPercentageThresholds.Count() - 1)
						break;

					if (i == 0 && area > (lodPreset.myPercentageThresholds[i] * 0.01f))
					{
						aLodOut = -1;
						return true;
					}

					if (area <= (lodPreset.myPercentageThresholds[i] * 0.01f) &&
						area > (lodPreset.myPercentageThresholds[i + 1] * 0.01f))
					{
						aLodOut = (int)i;
						return true;
					}
				}

				aLodOut = (int)lodPreset.myPercentageThresholds.Count() - 1;
				return true;
			}
		}

		return false;
	}
	SG_Material* SG_MeshInstance::GetMaterial() const
	{
		return myMaterial;
	}
	void SG_MeshInstance::DebugDraw()
	{
		SC_AABB aabb = myBoundingBox;		
		SC_Vector3f corners[8];

		// Near face
		corners[0] = { aabb.myMin };
		corners[1] = { aabb.myMin.x, aabb.myMax.y, aabb.myMin.z };
		corners[2] = { aabb.myMax.x, aabb.myMax.y, aabb.myMin.z };
		corners[3] = { aabb.myMax.x, aabb.myMin.y, aabb.myMin.z };

		// Far face
		corners[4] = { aabb.myMin.x, aabb.myMin.y, aabb.myMax.z };
		corners[5] = { aabb.myMin.x, aabb.myMax.y, aabb.myMax.z };
		corners[6] = { aabb.myMax };
		corners[7] = { aabb.myMax.x, aabb.myMin.y, aabb.myMax.z };

		// Near Face
		SG_PRIMITIVE_DRAW_LINE3D_COLORED(corners[0], corners[1], SC_Color(1.0f, 1.0f, 0.0f, 1.0f));
		SG_PRIMITIVE_DRAW_LINE3D_COLORED(corners[1], corners[2], SC_Color(1.0f, 1.0f, 0.0f, 1.0f));
		SG_PRIMITIVE_DRAW_LINE3D_COLORED(corners[2], corners[3], SC_Color(1.0f, 1.0f, 0.0f, 1.0f));
		SG_PRIMITIVE_DRAW_LINE3D_COLORED(corners[3], corners[0], SC_Color(1.0f, 1.0f, 0.0f, 1.0f));

		// Far face
		SG_PRIMITIVE_DRAW_LINE3D_COLORED(corners[4], corners[5], SC_Color(1.0f, 1.0f, 0.0f, 1.0f));
		SG_PRIMITIVE_DRAW_LINE3D_COLORED(corners[5], corners[6], SC_Color(1.0f, 1.0f, 0.0f, 1.0f));
		SG_PRIMITIVE_DRAW_LINE3D_COLORED(corners[6], corners[7], SC_Color(1.0f, 1.0f, 0.0f, 1.0f));
		SG_PRIMITIVE_DRAW_LINE3D_COLORED(corners[7], corners[4], SC_Color(1.0f, 1.0f, 0.0f, 1.0f));

		// Connecting near and far
		SG_PRIMITIVE_DRAW_LINE3D_COLORED(corners[0], corners[4], SC_Color(1.0f, 1.0f, 0.0f, 1.0f));
		SG_PRIMITIVE_DRAW_LINE3D_COLORED(corners[1], corners[5], SC_Color(1.0f, 1.0f, 0.0f, 1.0f));
		SG_PRIMITIVE_DRAW_LINE3D_COLORED(corners[2], corners[6], SC_Color(1.0f, 1.0f, 0.0f, 1.0f));
		SG_PRIMITIVE_DRAW_LINE3D_COLORED(corners[3], corners[7], SC_Color(1.0f, 1.0f, 0.0f, 1.0f));

	}

	void SG_MeshInstance::CalcBounds()
	{
		const SC_AABB& aabb = myMeshTemplate->GetAABB();
		myBoundingBox.myMin = aabb.myMin * myTransform;
		myBoundingBox.myMax = aabb.myMax * myTransform;

		float minx = myBoundingBox.myMin.x;
		float miny = myBoundingBox.myMin.y;
		float minz = myBoundingBox.myMin.z;
		float maxx = myBoundingBox.myMax.x;
		float maxy = myBoundingBox.myMax.y;
		float maxz = myBoundingBox.myMax.z;
		SC_Vector3f corner[8] =
		{
			SC_Vector3f(minx, miny, minz),
			SC_Vector3f(maxx, miny, minz),
			SC_Vector3f(maxx, maxy, minz),
			SC_Vector3f(minx, maxy, minz),
			SC_Vector3f(minx, miny, maxz),
			SC_Vector3f(maxx, miny, maxz),
			SC_Vector3f(maxx, maxy, maxz),
			SC_Vector3f(minx, maxy, maxz)
		};

		myBoundingBox.FromPoints(corner, 8);

		myBoundingSphere = SC_Sphere::FromAABB(myBoundingBox);
	}
}