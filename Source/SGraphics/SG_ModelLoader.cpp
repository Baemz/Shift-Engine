#include "SGraphics_Precompiled.h"
#include "SG_ModelLoader.h"
#include "SG_Model.h"
#include "SG_MeshHeader.h"
#include "SG_MeshInstance.h"
#include "SG_Mesh.h"
#include "SG_Material.h"

#include <fstream>
#include <sstream>

namespace Shift
{
	SG_ModelLoader::SG_ModelLoader()
	{
	}

	SG_ModelLoader::~SG_ModelLoader()
	{
	}
	
	SC_Future<SC_Ref<SG_Model>> SG_ModelLoader::ImportThreaded(const char* aFilePath)
	{
		return SC_CreateLongTask(this, &SG_ModelLoader::Import2, aFilePath, true);
	}

	SC_Ref<SG_Model> SG_ModelLoader::Import2(const char* aFilePath, bool /*aThreaded*/)
	{
		SG_MeshCreateData meshCreateData;

		SC_HybridArray<SC_Future<SC_Ref<SG_MeshInstance>>, 16> meshLoadFutures;
		std::ifstream inputFile(aFilePath, std::ios::binary);
		if (inputFile.is_open())
		{
			SC_Stopwatch timer;
			SC_Ref<SG_Model> model = new SG_Model();
			model->SetState(SC_LoadState::Loading);

			SG_MeshFileHeader fileHeader;
			char* fileHeaderPtr = (char*)(&fileHeader);
			inputFile.read(fileHeaderPtr, sizeof(fileHeader));

			meshCreateData.myName.resize(fileHeader.myNameSize);
			inputFile.read(meshCreateData.myName.data(), fileHeader.myNameSize);

			meshCreateData.myMaterialPath.resize(fileHeader.myMaterialPathSize);
			inputFile.read(meshCreateData.myMaterialPath.data(), fileHeader.myMaterialPathSize);

			meshCreateData.myLods.PreAllocate(fileHeader.myNumLods);
			for (int i = -1; i < (int)fileHeader.myNumLods; ++i)
			{
				SG_MeshLodData& meshData = (i == -1) ? meshCreateData.myBaseMesh : meshCreateData.myLods.Add();

				char* meshHeaderPtr = (char*)(&meshData.myHeader);
				inputFile.read(meshHeaderPtr, sizeof(meshData.myHeader));

				// Allocate space for mesh data
				meshData.myVertexData = new char[meshData.myHeader.myTotalVertexBufferSize];
				meshData.myIndexData = new char[meshData.myHeader.myTotalIndexBufferSize];

				// Read mesh data
				inputFile.read(meshData.myVertexData, meshData.myHeader.myTotalVertexBufferSize);
				inputFile.read(meshData.myIndexData, meshData.myHeader.myTotalIndexBufferSize);
			}
			inputFile.close();

			{
				SG_MeshInstance* instance = model->myMeshes.Add(SG_MeshInstance::Create(meshCreateData));
				model->myBoundingBox.Merge(instance->GetBoundingBox());
			}

			//if (aThreaded)
			//{
			//	for (auto& future : meshLoadFutures)
			//	{
			//		SG_MeshInstance* instance = model->myMeshes.Add(future.GetResult());
			//		model->myBoundingBox.Merge(instance->GetBoundingBox());
			//	}
			//}

			model->SetState(SC_LoadState::Loaded);
			SC_LOG("Loaded model [%s]: %.5f ms", meshCreateData.myName.c_str(), timer.Stop());

			return model;
		}
		else
		{
			SC_ERROR_LOG("Could not locate model file: (%s)", aFilePath);
			return nullptr;
		}
	}

	bool SG_ModelLoader::ImportCollection(const char* aFilePath, SC_GrowingArray<SC_Ref<SG_Model>>& aOutModels)
	{
		SC_Json collectionFile;
		std::ifstream outFile(aFilePath);
		if (outFile.is_open())
			outFile >> collectionFile;

		uint numMeshes = (uint)collectionFile["Meshes"].size();
		aOutModels.PreAllocate(numMeshes);
		for (uint i = 0, count = numMeshes; i < count; ++i)
		{
			SC_Ref<SG_Model> model = Import2(collectionFile["Meshes"][i].get<std::string>().c_str(), false);
			if (model)
				aOutModels.Add(model);
			else
				return false;
		}

		return true;
	}

	bool SG_ModelLoader::ReadModelName(const char* aFile, std::string& aOutName)
	{
		if (GetFileExtension(aFile) != "smf")
			return false;

		std::ifstream inputFile(aFile, std::ios::binary);
		if (inputFile.good())
		{
			uint64 currFilePos = 0;
			FileHeader fileHeader;
			char* headerPtr = reinterpret_cast<char*>(&fileHeader);
			inputFile.read(headerPtr, sizeof(fileHeader));
			currFilePos += sizeof(fileHeader);

			const uint nameByteSize = sizeof(char) * fileHeader.myNameSize;
			char* name = new char[fileHeader.myNameSize + 1];
			inputFile.seekg(currFilePos);
			inputFile.read(name, nameByteSize);
			currFilePos += nameByteSize;

			name[fileHeader.myNameSize] = '\0';
			aOutName = name;
			return true;
		}

		return false;
	}
	bool SG_ModelLoader::LoadModelFromFile(SG_Model& aModelOut)
	{
		aModelOut;
		return false;
	}
}