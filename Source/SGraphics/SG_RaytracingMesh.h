#pragma once
#include "SC_Resource.h"

namespace Shift
{
	class SG_RaytracingMesh : public SC_Resource
	{
		friend class SG_MeshInstance;
		friend class SG_Mesh;
		friend class SG_Raytracer;
	public:
		SG_RaytracingMesh();
		~SG_RaytracingMesh();

	private:
		SC_Ref<SR_Buffer> myBLAS;
		uint myLastUpdateFrame;
	};
}

