#include "SGraphics_Precompiled.h"
#include "SG_MeshStreamer.h"
#include "SG_Mesh.h"

namespace Shift
{
	SG_MeshStreamer::SG_MeshStreamer()
	{
	}
	SG_MeshStreamer::~SG_MeshStreamer()
	{
	}
	SC_Ref<SG_Mesh> SG_MeshStreamer::RequestMesh(const CMeshID& aID)
	{
		aID;
		return nullptr;
	}
	SC_Ref<SG_Mesh> SG_MeshStreamer::RequestMeshWithLod(const CMeshID& aID, uint aLod)
	{
		aID;
		aLod;
		return nullptr;
	}
}