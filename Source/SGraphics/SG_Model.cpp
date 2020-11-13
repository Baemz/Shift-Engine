#include "SGraphics_Precompiled.h"
#include "SG_Model.h"

#include "SG_Material.h"
#include "SG_ModelLoader.h"
#include "SG_Camera.h"
#include "SG_MeshInstance.h"


namespace Shift
{
	SG_Model::SG_Model()
		: myParent(nullptr)
	{
		myBoundingBox.myMin = SC_Vector3f(SC_FLT_MAX);
		myBoundingBox.myMax = SC_Vector3f(SC_FLT_LOWEST);
	}

	SG_Model::~SG_Model()
	{
	}

	SC_Ref<SG_Model> SG_Model::Create(const char* aFilePath)
	{
		static SG_ModelLoader locLoader;

		SC_Ref<SG_Model> model = locLoader.Import2(aFilePath);
		return model;
	}

	void SG_Model::Render()
	{
	}

	void SG_Model::DebugDrawAABB()
	{
	}

	bool SG_Model::Cull(const SG_Camera& /*aCamera*/)
	{
		bool isCompletelyInactive = true;

		return !isCompletelyInactive;
	}

	void SG_Model::SetTransform(const SC_Matrix44& aTransform)
	{
		myTransform = aTransform;
		for (auto& mesh : myMeshes)
		{
			mesh->SetTransform(myTransform);
		}
		myBoundingBox.myMin = myBoundingBox.myMin * myTransform;
		myBoundingBox.myMax = myBoundingBox.myMax * myTransform;
	}
	const SC_AABB& SG_Model::GetBoundingBox() const
	{
		return myBoundingBox;
	}
}