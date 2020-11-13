#pragma once
#include "SC_AABB.h"

namespace Shift
{
	struct SG_MeshHeader
	{
		SC_AABB myAABB;
		uint myVertexStrideSize;
		uint myNumVertices;
		uint myNumIndices;
		uint myTotalVertexBufferSize;
		uint myTotalIndexBufferSize;
	};

	struct SG_MeshFileHeader
	{
		uint myNumLods;
		uint myNameSize;
		uint myMaterialPathSize;
	};

	inline uint SC_Hash(const SG_MeshHeader& aHeader)
	{ 
		uint hash = 0;
		hash += SC_Hash(aHeader.myVertexStrideSize);
		hash += SC_Hash(aHeader.myNumVertices);
		hash += SC_Hash(aHeader.myNumIndices);
		hash += SC_Hash(aHeader.myTotalVertexBufferSize);
		hash += SC_Hash(aHeader.myTotalIndexBufferSize);

		hash += SC_Hash(aHeader.myAABB.myMax);
		hash += SC_Hash(aHeader.myAABB.myMin);

		return hash;
	}

}