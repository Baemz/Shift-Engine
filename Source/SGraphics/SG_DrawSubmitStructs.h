#pragma once
#include "SC_Matrix.h"
#include "SG_MeshInstance.h"
namespace Shift
{
	class SG_Model;
	class SG_Material;
	class CIndexBuffer;
	class CVertexBuffer;
	class SR_Texture;

	struct SG_PointLightObject
	{
		SC_Vector3f PointLightPos;
		SC_Vector3f PointLightColor;
		float PointLightRange;
		float PointLightIntensity;
	};

	struct SG_RenderObject
	{
		SG_RenderObject()
			: myModel(nullptr)
			, myMaterial(nullptr)
		{}

		bool operator==(const SG_RenderObject& aOther)
		{
			if (myTransform == aOther.myTransform &&
				myModel == aOther.myModel &&
				myMaterial == aOther.myMaterial)
			{
				return true;
			}

			return false;
		}
		bool operator<(const SG_RenderObject& aOther);

		SC_Matrix44 myTransform;
		SC_Ref<SG_MeshInstance> myMeshInstance;
		SG_Model* myModel;
		SG_Material* myMaterial;
	};

	struct SG_PrimitiveRenderObject
	{
		SG_PrimitiveRenderObject()
			: myVertexBuffer(nullptr)
			, myIndexBuffer(nullptr)
			, myTexture(nullptr)
		{}
		bool operator==(const SG_PrimitiveRenderObject& aOther)
		{
			if (myTransform == aOther.myTransform &&
				myVertexBuffer == aOther.myVertexBuffer &&
				myIndexBuffer == aOther.myIndexBuffer &&
				myTexture == aOther.myTexture)
			{
				return true;
			}

			return false;
		}

		SC_Matrix44 myTransform;
		CVertexBuffer* myVertexBuffer;
		CIndexBuffer* myIndexBuffer;
		SR_Texture* myTexture;
		SR_Texture* myTexture2;
		SR_Texture* myTexture3;
	};
}