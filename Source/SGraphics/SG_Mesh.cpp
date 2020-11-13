#include "SGraphics_Precompiled.h"
#include "SG_Mesh.h"
#include "SG_MeshHeader.h"
#include "SG_RaytracingMesh.h"

namespace Shift
{
	SC_HashMap<uint, SC_Ref<SG_Mesh>> SG_Mesh::ourCache;

	SG_Mesh::SG_Mesh()
	{
	}

	SG_Mesh::~SG_Mesh()
	{
	}

	SC_Ref<SG_Mesh> SG_Mesh::Create(const SG_MeshHeader& /*aHeader*/, void* /*aVertexData*/, void* /*aIndexData*/, const char* /*aName*/)
	{
		//uint hash = 0;
		//hash += SC_Hash(aHeader);
		//if (aName)
		//	hash += SC_Hash(aName);
		//
		//SC_Ref<SG_Mesh> mesh;
		//if (ourCache.Find(hash, mesh))
		//	return mesh;
		//
		//
		//mesh->myVertexBufferData = new uint8[aHeader.myTotalVertexBufferSize];
		//SC_Memcpy(mesh->myVertexBufferData, aVertexData, aHeader.myTotalVertexBufferSize);
		//mesh->myNumVertices = aHeader.myNumVertices;
		//mesh->myVertexStride = aHeader.myVertexStrideSize;
		//
		//mesh->myIndexBufferData = new uint[aHeader.myTotalIndexBufferSize];
		//SC_Memcpy(mesh->myIndexBufferData, aIndexData, aHeader.myTotalIndexBufferSize);
		//mesh->myNumIndices = aHeader.myNumIndices;
		//
		//mesh->myBoundingBox = aHeader.myAABB;
		//mesh->myName = aName;
		//
		//mesh->Init();
		//
		//ourCache.Insert(hash, mesh);
		SC_Ref<SG_Mesh> mesh = new SG_Mesh();
		return mesh;
	}

	SC_Ref<SG_Mesh> SG_Mesh::Create(const SG_MeshCreateData& aCreateData)
	{
		const SG_MeshHeader& header = aCreateData.myBaseMesh.myHeader;

		uint hash = 0;
		hash += SC_Hash(header);
		for (uint i = 0; i < aCreateData.myLods.Count(); ++i)
			hash += SC_Hash(aCreateData.myLods[i].myHeader);

		if (!aCreateData.myName.empty())
			hash += SC_Hash(aCreateData.myName.c_str());

		SC_Ref<SG_Mesh> mesh;
		if (ourCache.Find(hash, mesh))
			return mesh;

		mesh = new SG_Mesh();
		SG_MeshLod& baseMesh = mesh->myBaseMesh;

		baseMesh.myVertexBufferData = new uint8[header.myTotalVertexBufferSize];
		SC_Memcpy(baseMesh.myVertexBufferData, aCreateData.myBaseMesh.myVertexData, header.myTotalVertexBufferSize);
		baseMesh.myNumVertices = header.myNumVertices;
		baseMesh.myVertexStride = header.myVertexStrideSize;

		baseMesh.myIndexBufferData = new uint[header.myTotalIndexBufferSize];
		SC_Memcpy(baseMesh.myIndexBufferData, aCreateData.myBaseMesh.myIndexData, header.myTotalIndexBufferSize);
		baseMesh.myNumIndices = header.myNumIndices;

		baseMesh.myBoundingBox = header.myAABB;
		mesh->myName = aCreateData.myName;

		for (uint i = 0; i < aCreateData.myLods.Count(); ++i)
		{
			const SG_MeshHeader& lodHeader = aCreateData.myLods[i].myHeader;
			SG_MeshLod& lod = mesh->myLods.Add();

			lod.myVertexBufferData = new uint8[lodHeader.myTotalVertexBufferSize];
			SC_Memcpy(lod.myVertexBufferData, aCreateData.myLods[i].myVertexData, lodHeader.myTotalVertexBufferSize);
			lod.myNumVertices = lodHeader.myNumVertices;
			lod.myVertexStride = lodHeader.myVertexStrideSize;

			lod.myIndexBufferData = new uint[lodHeader.myTotalIndexBufferSize];
			SC_Memcpy(lod.myIndexBufferData, aCreateData.myLods[i].myIndexData, lodHeader.myTotalIndexBufferSize);
			lod.myNumIndices = lodHeader.myNumIndices;

			lod.myBoundingBox = lodHeader.myAABB;
		}

		mesh->Init();

		ourCache.Insert(hash, mesh);
		return mesh;
	}

	void SG_Mesh::Init()
	{
		SR_GraphicsDevice* device = SR_GraphicsDevice::GetDevice();

		SR_BufferDesc vertexBufferDesc;
		vertexBufferDesc.myBindFlag = SBindFlag_VertexBuffer;
		vertexBufferDesc.mySize = myBaseMesh.myVertexStride * myBaseMesh.myNumVertices;
		vertexBufferDesc.myStructSize = myBaseMesh.myVertexStride;
		myBaseMesh.myVertexBuffer = device->CreateBuffer(vertexBufferDesc, myBaseMesh.myVertexBufferData, "Mesh VertexBuffer");

		SR_BufferDesc indexBufferDesc;
		indexBufferDesc.myBindFlag = SBindFlag_IndexBuffer;
		indexBufferDesc.mySize = sizeof(uint) * myBaseMesh.myNumIndices;
		myBaseMesh.myIndexBuffer = device->CreateBuffer(indexBufferDesc, myBaseMesh.myIndexBufferData, "Mesh IndexBuffer");

		for (SG_MeshLod& lod : myLods)
		{
			vertexBufferDesc.mySize = lod.myVertexStride * lod.myNumVertices;
			vertexBufferDesc.myStructSize = lod.myVertexStride;
			lod.myVertexBuffer = device->CreateBuffer(vertexBufferDesc, lod.myVertexBufferData, "Mesh VertexBuffer");

			indexBufferDesc.mySize = sizeof(uint) * lod.myNumIndices;
			lod.myIndexBuffer = device->CreateBuffer(indexBufferDesc, lod.myIndexBufferData, "Mesh IndexBuffer");
		}
	}

	SG_RaytracingMesh* SG_Mesh::GetCreateRTMesh()
	{
#if SR_ENABLE_RAYTRACING
		if (!myRTMesh)
		{
			myRTMesh = new SG_RaytracingMesh();

			myRTMesh->myBLAS = SR_GraphicsContext::GetCurrent()->CreateRTMesh(myRTMesh->myBLAS ? nullptr : myRTMesh->myBLAS, false, myBaseMesh.myVertexBuffer,
				myBaseMesh.myVertexStride, myBaseMesh.myNumVertices, myBaseMesh.myIndexBuffer, myBaseMesh.myNumIndices, nullptr);
		}
#endif

		return myRTMesh;
	}
}