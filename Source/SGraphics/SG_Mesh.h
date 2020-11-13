#pragma once
#include "SC_Resource.h"
#include "SC_AABB.h"
#include "SG_RenderType.h"
#include "SG_MeshHeader.h"

namespace Shift
{
	using MeshHandle = unsigned int;
	class SR_Buffer;
	class SR_ShaderState;
	class SG_RaytracingMesh;

	struct SG_MeshLodData
	{
		SG_MeshHeader myHeader;
		char* myVertexData;
		char* myIndexData;
	};

	struct SG_MeshCreateData
	{
		SG_MeshLodData myBaseMesh;
		SC_GrowingArray<SG_MeshLodData> myLods;
		std::string myName;
		std::string myMaterialPath;
	};

	struct SG_MeshLod
	{
		SG_MeshLod()
			: myVertexStride(0)
			, myNumVertices(0)
			, myNumIndices(0)
		{}
		SG_MeshLod(const SG_MeshLod& aOther) 
			: myBoundingBox(aOther.myBoundingBox)
			, myVertexBuffer(aOther.myVertexBuffer)
			, myIndexBuffer(aOther.myIndexBuffer)
			, myVertexStride(aOther.myVertexStride)
			, myNumVertices(aOther.myNumVertices)
			, myNumIndices(aOther.myNumIndices)
		{
			const uint vbufSize = myNumVertices * myVertexStride;
			myVertexBufferData = new uint8[vbufSize];
			SC_Memcpy(myVertexBufferData, aOther.myVertexBufferData, vbufSize);


			const uint ibufSize = myNumIndices * sizeof(uint);
			myIndexBufferData = new uint[ibufSize];
			SC_Memcpy(myIndexBufferData, aOther.myIndexBufferData, ibufSize);
		}

		SC_AABB myBoundingBox;
		SC_Ref<SR_Buffer> myVertexBuffer;
		SC_Ref<SR_Buffer> myIndexBuffer;

		uint myVertexStride;
		uint myNumVertices;
		uint myNumIndices;
		SC_UniquePtr<uint8> myVertexBufferData;
		SC_UniquePtr<uint> myIndexBufferData;
	};

	class SG_Mesh : public SC_Resource
	{
	public:
		SG_Mesh();
		~SG_Mesh();

		static SC_Ref<SG_Mesh> Create(const SG_MeshHeader& aHeader, void* aVertexData, void* aIndexData, const char* aName = nullptr);
		static SC_Ref<SG_Mesh> Create(const SG_MeshCreateData& aCreateData);

		void Init();
		SG_RaytracingMesh* GetCreateRTMesh();

		const SC_AABB& GetAABB() const { return myBaseMesh.myBoundingBox; }

		const SG_MeshLod& GetLod(int aLodIndex) const 
		{ 
			int idx = SC_Clamp(aLodIndex, -1, int(myLods.Count() - 1));
			if (idx == -1)
				return myBaseMesh;
			
			return myLods[idx];
		}

		SC_Ref<SR_ShaderState> myShaders[SG_RenderType_COUNT];
	protected:
		std::string myName;
		SG_MeshLod myBaseMesh;
		SC_GrowingArray<SG_MeshLod> myLods;

		SC_Ref<SG_RaytracingMesh> myRTMesh;

	private:
		static SC_HashMap<uint, SC_Ref<SG_Mesh>> ourCache;
	};

}