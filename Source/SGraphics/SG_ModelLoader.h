#pragma once

namespace Shift
{
	class SG_Model;
	class SG_ModelLoader
	{
	public:
		SG_ModelLoader();
		~SG_ModelLoader();

		SC_Future<SC_Ref<SG_Model>> ImportThreaded(const char* aFilePath);
		SC_Ref<SG_Model> Import2(const char* aFilePath, bool aThreaded = true);
		bool ImportCollection(const char* aFilePath, SC_GrowingArray<SC_Ref<SG_Model>>& aOutModels);

		bool ReadModelName(const char* aFile, std::string& aOutString);

	private:
		bool LoadModelFromFile(SG_Model& aModelOut);

		static constexpr const char* ourModelExtension = ".smf";

		struct MeshHeader
		{
			SC_AABB myAABB;
			uint myVertexStructSize;
			uint myVertexStartOffset;
			uint myNumVertices;
			uint myIndexStartOffset;
			uint myNumIndices;
			uint myMeshByteSize;
			uint myMaterialHash;
		};
		struct FileHeader
		{
			uint myNumMeshes;
			uint myNameSize;
		};

		void WriteMesh();
		void WriteMaterial();
	};

}
