#pragma once
#include "SC_Resource.h"

struct aiNode;
struct aiScene;
struct aiMaterial;
namespace Shift
{
	class SG_Material;
	class SG_MeshInstance;
	class SG_Camera;
	class SG_Model : public SC_Resource
	{
		friend class SG_ModelLoader;
		friend class SG_World;
	public:
		SG_Model();
		~SG_Model();

		static SC_Ref<SG_Model> Create(const char* aFilePath);

		void Render();
		void DebugDrawAABB();
		bool Cull(const SG_Camera& aCamera);
		void SetTransform(const SC_Matrix44& aTransform);

		const SC_AABB& GetBoundingBox() const;

		SC_Matrix44 myTransform;
	private:
		SC_GrowingArray<SC_Ref<SG_MeshInstance>> myMeshes;
		std::string myName;
		SC_AABB myBoundingBox;
		SG_Model* myParent;
	};
}