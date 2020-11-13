#pragma once
#include "SC_Resource.h"
#include "SC_Sphere.h"

namespace Shift
{
	class SG_Mesh;
	class SG_Camera;
	class SG_Material;
	struct SG_MeshHeader;
	struct SG_MeshCreateData;

	class SG_MeshInstance : public SC_Resource
	{
	public:
		SG_MeshInstance(SG_Mesh* aMesh);
		~SG_MeshInstance();

		static SC_Ref<SG_MeshInstance> Create(const SG_MeshHeader& aHeader, void* aVertexData, void* aIndexData, const char* aMaterialPath = nullptr, const char* aName = nullptr);
		static SC_Ref<SG_MeshInstance> Create(const SG_MeshCreateData& aCreateData);

		void SetTransform(const SC_Matrix44& aTransform);
		const SC_Matrix44& GetTransform() const;
		const SC_AABB& GetBoundingBox() const;
		const SC_Sphere& GetBoundingSphere() const;

		bool CullAndReturnLod(const SG_Camera& aCamera);
		bool CullAndReturnLod(const SG_Camera& aCamera, int& aLodOut);
		SG_Material* GetMaterial() const;

		void DebugDraw();

		SC_Ref<SG_Mesh> myMeshTemplate;
	private:
		void CalcBounds();

		SC_Ref<SG_Material> myMaterial;
		SC_Matrix44 myTransform;
		SC_AABB myBoundingBox;
		SC_Sphere myBoundingSphere;

	};
}
