#pragma once
#include <map>

namespace Shift
{
	using CMeshID = const char*;

	class SG_Mesh;
	class SG_MeshCache
	{
	public:
		friend class SG_MeshStreamer;
		~SG_MeshCache();

		void Init();
		void Destroy();

		void GetCreateMesh(const CMeshID& aID, uint aLOD);

	private:
		SG_MeshCache();

		std::unordered_map<CMeshID, SC_Ref<SG_Mesh>[6]> ourCache; // Sorted on ID, array contains LOD 0 -> MAXLOD
	};
}

