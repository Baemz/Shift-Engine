#include "SGraphics_Precompiled.h"
#include "SG_MeshCache.h"
#include "SG_Mesh.h"

namespace Shift
{
	SG_MeshCache::~SG_MeshCache()
	{
		Destroy();
	}
	void SG_MeshCache::Init()
	{
	}
	void SG_MeshCache::Destroy()
	{
	}
	void SG_MeshCache::GetCreateMesh(const CMeshID& aID, uint aLOD)
	{
		aID;
		aLOD;
	}
}