#pragma once
#include "STools_EditorSubModule.h"

namespace ImGui
{
	class FileBrowser;
}

struct aiNode;
struct aiScene;

namespace Shift
{
	struct STools_MeshLod
	{
		SC_AABB myAABB;
		uint myVertexStrideSize;
		uint myNumVertices;
		uint myNumIndices;
		uint myTotalVertexBufferSize;
		uint myTotalIndexBufferSize;
		uint8* myVertices;
		uint* myIndices;
		bool myIsEnabled;
	};

	struct STools_MeshBuffer
	{
		SC_GrowingArray<STools_MeshLod> myLods;
		SC_GrowingArray<std::string> myTextures;
		std::string myName;
		std::string myMaterialPath;
		STools_MeshLod myMesh;

	};

	class STools_AssetImporter final : public STools_EditorSubModule
	{
		STOOLS_DECLARESUBMODULE(STools_AssetImporter);
	public:
		STools_AssetImporter();
		~STools_AssetImporter();

		void Render() override;

		const char* GetWindowName() const override { return "Asset Importer"; }
	private:
		void FindAndCreatePossibleLods(const char* aFile);
		void RenderImportedIntermediate();

		bool Import(const std::string& aFile);
		bool ImportSMF(const std::string& aFile);
		bool WriteMeshFile(const std::string& aFile, const std::string& aSubFolder, const STools_MeshBuffer& aMeshes);
		void ProcessAiNode(aiNode* aNode, const aiScene* aScene, const uint aLodIndex, SC_GrowingArray<STools_MeshBuffer>& aOutMeshes);
		void ExtractMesh(const aiScene* scene, uint aMeshIndex, STools_MeshLod& aMesh);

		std::string mySelectedFile;
		SC_UniquePtr<ImGui::FileBrowser> myBrowser;
		SC_GrowingArray<STools_MeshBuffer> myImportedMeshes;

		// Imported Intermediate
		std::string mySelectedMaterialFile;
		std::string myImportedModelName;
		int mySelectedMeshIdx;
		int mySelectedLodIdx;
		int mySelectedTextureIdx;
	};

}