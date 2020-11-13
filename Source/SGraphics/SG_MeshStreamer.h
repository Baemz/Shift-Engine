#pragma once
#include "SG_MeshCache.h"

namespace Shift
{
	class SG_Mesh;
	class SG_MeshStreamer
	{
	public:
		SG_MeshStreamer();
		~SG_MeshStreamer();

		SC_Ref<SG_Mesh> RequestMesh(const CMeshID& aID);
		SC_Ref<SG_Mesh> RequestMeshWithLod(const CMeshID& aID, uint aLod = 0);
	private:
		SG_MeshCache myMeshCache;
	};
}

