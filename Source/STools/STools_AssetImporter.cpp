#include "STools_Precompiled.h"
#include "STools_AssetImporter.h"

#include "SG_MeshHeader.h"
#include "SG_Material.h"

#include "imguifilebrowser.h"

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#include <sstream>

namespace Shift
{
	STools_AssetImporter::STools_AssetImporter()
		: mySelectedMeshIdx(0)
	{
		myBrowser = new ImGui::FileBrowser();
		myBrowser->SetTitle("Asset Browser");

		std::filesystem::create_directory("Assets");
		std::filesystem::create_directory("Assets/Imported");
		std::filesystem::create_directory("Assets/Imported/Models");
	}

	STools_AssetImporter::~STools_AssetImporter()
	{
	}

	void STools_AssetImporter::Render()
	{
		ImGui::Text("%s", mySelectedFile.c_str());

		if (ImGui::Button("..."))
		{
			mySelectedFile.clear();
			myBrowser->SetTypeFilters({ ".fbx", ".obj", ".3ds", ".smf" });
			myBrowser->Open();
		}

		myBrowser->Display();

		if (mySelectedFile.empty() && myBrowser->HasSelected())
		{
			mySelectedFile = myBrowser->GetSelected().string();
			myBrowser->ClearSelected();

			SC_Stopwatch time;
			if (Import(mySelectedFile))
			{
				SC_SUCCESS_LOG("Asset Imported [%s]: import duration: %.3f ms", GetFileWithExtension(mySelectedFile).c_str(), time.Stop());
			}
			else
			{
				SC_ERROR_LOG("Failed to import asset [%s]", GetFileWithExtension(mySelectedFile).c_str());
			}
		}
		
		if (!myImportedMeshes.Empty())
		{
			RenderImportedIntermediate();
		}
	}

	void STools_AssetImporter::FindAndCreatePossibleLods(const char* aFile)
	{
		const std::string dir = SC_GetFileDirectory(aFile);
		std::string filenameWithoutLod = GetFilename(aFile);
		filenameWithoutLod = filenameWithoutLod.substr(0, filenameWithoutLod.rfind('_'));

		SC_GrowingArray<std::string> meshLodPaths;
		for (const auto& file : std::filesystem::directory_iterator(dir))
		{
			std::string fileName = file.path().filename().string();
			if (fileName.find(filenameWithoutLod) != std::string::npos &&
				GetFileExtension(fileName).find("fbx") != std::string::npos)
				meshLodPaths.Add(file.path().string());
		}

		Assimp::Importer importer;
		unsigned int flags = aiProcess_CalcTangentSpace | aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_SortByPType | aiProcess_ConvertToLeftHanded;
		const aiScene* scene = importer.ReadFile(meshLodPaths.GetFirst(), flags);

		if (scene->HasMeshes())
		{
			uint numMeshes = scene->mNumMeshes;
			for (uint meshIdx = 0; meshIdx < numMeshes; ++meshIdx)
			{
				STools_MeshBuffer& mesh = myImportedMeshes.Add();

				ExtractMesh(scene, meshIdx, mesh.myMesh);
				mesh.myMesh.myIsEnabled = true;
				mesh.myName = scene->mMeshes[meshIdx]->mName.C_Str();
			}
			for (uint i = 1; i < meshLodPaths.Count(); ++i)
			{
				const aiScene* lodScene = importer.ReadFile(meshLodPaths[i], flags);
				uint numLodMeshes = lodScene->mNumMeshes;

				for (uint meshIdx = 0; meshIdx < numLodMeshes; ++meshIdx)
				{
					for (STools_MeshBuffer& mesh : myImportedMeshes)
					{
						if (mesh.myName.find(lodScene->mMeshes[meshIdx]->mName.C_Str()) != std::string::npos)
							ExtractMesh(lodScene, meshIdx, mesh.myLods.Add());
					}
				}
			}


			myImportedModelName = GetFilename(filenameWithoutLod);
		}
	}

	void STools_AssetImporter::RenderImportedIntermediate()
	{
		uint numMeshes = myImportedMeshes.Count();

		SC_HybridArray<const char*, 10> meshList;
		for (uint i = 0; i < numMeshes; ++i)
			meshList.Add(myImportedMeshes[i].myName.c_str());

		ImGui::ListBox(" :Meshes", &mySelectedMeshIdx, meshList.GetBuffer(), numMeshes, SC_Min(meshList.Count(), 6));

		if (mySelectedMeshIdx >= 0)
		{
			STools_MeshBuffer& mesh = myImportedMeshes[mySelectedMeshIdx];
			uint numLods = mesh.myLods.Count();
			SC_HybridArray<std::string, 6> tempstr;
			SC_HybridArray<const char*, 6> lodList;
			for (uint i = 1; i <= numLods; ++i)
			{
				tempstr.Add(GetFormattedString("LOD %i", i));
				lodList.Add(tempstr.GetLast().c_str());
			}

			ImGui::NewLine();
			ImGui::Separator();
			ImGui::NewLine();
			ImGui::Text("Model: %s", myImportedModelName.c_str());
			ImGui::Text("Num Lods in file: %i", numLods);

			{
				STools_MeshLod& meshLod = mesh.myMesh;
				ImGui::Text("Name: %s", mesh.myName.c_str());
				ImGui::Text("AABB: { min: x:%.3f y:%.3f z:%.3f }\n\t\t{ max: x:%.3f y:%.3f z:%.3f }",
					meshLod.myAABB.myMin.x, meshLod.myAABB.myMin.y, meshLod.myAABB.myMin.z,
					meshLod.myAABB.myMax.x, meshLod.myAABB.myMax.y, meshLod.myAABB.myMax.z);
				ImGui::Text("Num Vertices: %i", meshLod.myNumVertices);
				ImGui::Text("Vertex Size: %i", meshLod.myVertexStrideSize);
				ImGui::Text("Num Indices: %i", meshLod.myNumIndices);
			}
			if (numLods > 0)
			{
				ImGui::ListBox(" :Lods", &mySelectedLodIdx, lodList.GetBuffer(), numLods, SC_Min(lodList.Count(), 6));
				ImGui::NewLine();

				ImGui::Separator();
				ImGui::NewLine();

				if (mySelectedLodIdx >= 0)
				{
					STools_MeshLod& meshLod = mesh.myLods[mySelectedLodIdx];
					ImGui::Checkbox("Enable", &meshLod.myIsEnabled);
					ImGui::Text("LOD %i", mySelectedLodIdx);
					ImGui::Text("AABB: { min: x:%.3f y:%.3f z:%.3f }\n\t\t{ max: x:%.3f y:%.3f z:%.3f }",
						meshLod.myAABB.myMin.x, meshLod.myAABB.myMin.y, meshLod.myAABB.myMin.z,
						meshLod.myAABB.myMax.x, meshLod.myAABB.myMax.y, meshLod.myAABB.myMax.z);
					ImGui::Text("Num Vertices: %i", meshLod.myNumVertices);
					ImGui::Text("Vertex Size: %i", meshLod.myVertexStrideSize);
					ImGui::Text("Num Indices: %i", meshLod.myNumIndices);
					ImGui::Separator();
					ImGui::NewLine();
				}
			}
			ImGui::NewLine();
			mesh.myMaterialPath.reserve(256);
			ImGui::Text("Material: %s", mesh.myMaterialPath.c_str());
			ImGui::SameLine();
			if (ImGui::Button("Material..."))
			{
				mesh.myMaterialPath.clear();
				myBrowser->SetTypeFilters({ ".smtf" });
				myBrowser->Open();
			}

			if (myBrowser->HasSelected() && mesh.myMaterialPath.empty())
			{
				mesh.myMaterialPath = myBrowser->GetSelected().string();
				std::replace(mesh.myMaterialPath.begin(), mesh.myMaterialPath.end(), '\\', '/');

				std::string currentDir = SC_EnginePaths::GetWorkingDirectoryAbsolute();
				size_t pos = mesh.myMaterialPath.find(currentDir);
				if (pos != mesh.myMaterialPath.npos)
				{
					mesh.myMaterialPath.erase(pos, currentDir.length());
					mesh.myMaterialPath = mesh.myMaterialPath.erase(mesh.myMaterialPath.find('/'), 1);
				}

				myBrowser->ClearSelected();
			}

		}

		if (ImGui::Button("Save"))
		{
			bool writeCollection = myImportedMeshes.Count() > 1;
			SC_HybridArray<std::string, 64> collectionPaths;
			collectionPaths.PreAllocate(myImportedMeshes.Count());

			std::string subFolder;
			if (writeCollection)
				subFolder = myImportedModelName;

			for (STools_MeshBuffer& mesh : myImportedMeshes)
			{
				//if (mesh.myName.rfind("_column_b") != std::string::npos)
				//	mesh.myMaterialPath = "Assets/Materials/Sponza/Sponza_Column_B.smtf";
				//else if (mesh.myName.rfind("_fabric_c") != std::string::npos)
				//	mesh.myMaterialPath = "Assets/Materials/Sponza/Sponza_Curtain_Red.smtf";
				//else if (mesh.myName.rfind("_fabric_g") != std::string::npos)
				//	mesh.myMaterialPath = "Assets/Materials/Sponza/Sponza_Curtain_Blue.smtf";
				//else if (mesh.myName.rfind("_fabric_f") != std::string::npos)
				//	mesh.myMaterialPath = "Assets/Materials/Sponza/Sponza_Curtain_Green.smtf";
				//else if (mesh.myName.rfind("_flagpole") != std::string::npos)
				//	mesh.myMaterialPath = "Assets/Materials/Sponza/Sponza_FlagPole.smtf";
				//else if (mesh.myName.rfind("_column_c") != std::string::npos)
				//	mesh.myMaterialPath = "Assets/Materials/Sponza/Sponza_Column_C.smtf";
				//else if (mesh.myName.rfind("_column_a") != std::string::npos)
				//	mesh.myMaterialPath = "Assets/Materials/Sponza/Sponza_Column_A.smtf";
				//else if (mesh.myName.rfind("_fabric_a") != std::string::npos)
				//	mesh.myMaterialPath = "Assets/Materials/Sponza/Sponza_Fabric_Red.smtf";
				//else if (mesh.myName.rfind("_floor") != std::string::npos)
				//	mesh.myMaterialPath = "Assets/Materials/Sponza/Sponza_Floor.smtf";
				//else if (mesh.myName.rfind("_roof") != std::string::npos)
				//	mesh.myMaterialPath = "Assets/Materials/Sponza/Sponza_Roof.smtf";
				//else if (mesh.myName.rfind("_bricks") != std::string::npos)
				//	mesh.myMaterialPath = "Assets/Materials/Sponza/Sponza_Bricks_A.smtf";
				//else if (mesh.myName.rfind("_details") != std::string::npos)
				//	mesh.myMaterialPath = "Assets/Materials/Sponza/Sponza_Details.smtf";
				//else if (mesh.myName.rfind("_vase_hanging") != std::string::npos)
				//	mesh.myMaterialPath = "Assets/Materials/Sponza/Sponza_VaseHanging.smtf";
				//else if (mesh.myName.rfind("_chain") != std::string::npos)
				//	mesh.myMaterialPath = "Assets/Materials/Sponza/Sponza_Chain.smtf";
				//else if (mesh.myName.rfind("_arch") != std::string::npos)
				//	mesh.myMaterialPath = "Assets/Materials/Sponza/Sponza_Arch.smtf";
				//else if (mesh.myName.rfind("_ceiling") != std::string::npos)
				//	mesh.myMaterialPath = "Assets/Materials/Sponza/Sponza_Ceiling.smtf";
				//else if (mesh.myName.rfind("_fabric_e") != std::string::npos)
				//	mesh.myMaterialPath = "Assets/Materials/Sponza/Sponza_Fabric_Green.smtf";
				//else if (mesh.myName.rfind("_fabric_d") != std::string::npos)
				//	mesh.myMaterialPath = "Assets/Materials/Sponza/Sponza_Fabric_Blue.smtf";
				//else if (mesh.myName.rfind("_leaf") != std::string::npos)
				//	mesh.myMaterialPath = "Assets/Materials/Sponza/Sponza_Leaf.smtf";
				//else if (mesh.myName.rfind("_Material__57") != std::string::npos)
				//	mesh.myMaterialPath = "Assets/Materials/Sponza/Sponza_VasePlant.smtf";
				//else if (mesh.myName.rfind("_Material__298") != std::string::npos)
				//	mesh.myMaterialPath = "Assets/Materials/Sponza/Sponza_Background.smtf";
				//else if (mesh.myName.rfind("_Material__25") != std::string::npos)
				//	mesh.myMaterialPath = "Assets/Materials/Sponza/Sponza_Lion.smtf";
				//else if (mesh.myName.rfind("_vase") != std::string::npos)
				//	mesh.myMaterialPath = "Assets/Materials/Sponza/Sponza_Vase.smtf";
				//else if (mesh.myName.rfind("_vase_round") != std::string::npos)
				//	mesh.myMaterialPath = "Assets/Materials/Sponza/Sponza_VaseRound.smtf";

				for (int i = mesh.myLods.Count() - 1; i >= 0; --i)
				{
					STools_MeshLod& lod = mesh.myLods[i];
					if (!lod.myIsEnabled)
						mesh.myLods.RemoveByIndex(i);
				}

				if (!WriteMeshFile(mesh.myName, subFolder, mesh))
				{
					SC_ERROR_LOG("Failed to save asset [%s]", GetFileWithExtension(mesh.myName).c_str());
				}
				else if (writeCollection)
				{
					std::stringstream path;
					path << "Assets/Imported/Models/" << subFolder << "/" << mesh.myName << ".smf";
					collectionPaths.Add(path.str());
				}
			}

			if (writeCollection)
			{
				std::stringstream path;
				path << "Assets/Imported/Models/" << myImportedModelName << ".smcf";

				SC_Json collectionFile;
				for (uint i = 0, count = collectionPaths.Count(); i < count; ++i)
					collectionFile["Meshes"][i] = collectionPaths[i];

				std::ofstream outFile(path.str());
				if (outFile.is_open())
					outFile << collectionFile;
			}

			myImportedMeshes.RemoveAll();
		}
	}

	bool STools_AssetImporter::Import(const std::string& aFile)
	{
		if (GetFileExtension(aFile) == "smf")
			return ImportSMF(aFile);

		SC_GrowingArray<const aiScene*> meshImports;
		FindAndCreatePossibleLods(aFile.c_str());

		return true;
	}

	bool STools_AssetImporter::ImportSMF(const std::string& aFile)
	{
		std::ifstream inputFile(aFile, std::ios::binary);
		if (!inputFile.is_open())
		{
			SC_ERROR_LOG("Cannot open .smf file [%s]", aFile.c_str());
			return false;
		}

		SG_MeshFileHeader fileHeader;
		char* fileHeaderPtr = (char*)(&fileHeader);
		inputFile.read(fileHeaderPtr, sizeof(fileHeader));

		std::string modelName;
		modelName.resize(fileHeader.myNameSize);
		inputFile.read(modelName.data(), fileHeader.myNameSize);


		myImportedMeshes.PreAllocate(fileHeader.myNumLods);
		for (uint i = 0; i < fileHeader.myNumLods; ++i)
		{
			SG_MeshHeader meshHeader;
			char* meshHeaderPtr = (char*)(&meshHeader);
			inputFile.read(meshHeaderPtr, sizeof(meshHeader));

			//char* meshNameBuffer = new char[meshHeader.myNameSize + 1];
			//meshNameBuffer[meshHeader.myNameSize] = '\0';
			//char* materialPathBuffer = new char[meshHeader.myMaterialPathSize + 1];
			//materialPathBuffer[meshHeader.myMaterialPathSize] = '\0';

			// Allocate space for mesh data
			char* vertexBufferData = new char[meshHeader.myTotalVertexBufferSize];
			char* indexBufferData = new char[meshHeader.myTotalIndexBufferSize];

			// Read mesh name
			//inputFile.read(meshNameBuffer, meshHeader.myNameSize);
			//inputFile.read(materialPathBuffer, meshHeader.myMaterialPathSize);

			// Read mesh data
			inputFile.read(vertexBufferData, meshHeader.myTotalVertexBufferSize);
			inputFile.read(indexBufferData, meshHeader.myTotalIndexBufferSize);

			STools_MeshBuffer meshBuffer;
			meshBuffer.myMesh.myAABB = meshHeader.myAABB;
			meshBuffer.myMesh.myNumVertices = meshHeader.myNumVertices;
			meshBuffer.myMesh.myNumIndices = meshHeader.myNumIndices;
			//meshBuffer.myMaterialPath = materialPathBuffer;
			//meshBuffer.myName = meshNameBuffer;
			meshBuffer.myMesh.myTotalVertexBufferSize = meshHeader.myTotalVertexBufferSize;
			meshBuffer.myMesh.myTotalIndexBufferSize = meshHeader.myTotalIndexBufferSize;
			meshBuffer.myMesh.myVertexStrideSize = meshHeader.myVertexStrideSize;
			meshBuffer.myMesh.myVertices = (uint8*)vertexBufferData;
			meshBuffer.myMesh.myIndices = (uint*)indexBufferData;
			myImportedMeshes.Add(meshBuffer);
			//delete[] meshNameBuffer;
			//delete[] materialPathBuffer;
		}

		return true;
	}

	bool STools_AssetImporter::WriteMeshFile(const std::string& aFile, const std::string& aSubFolder, const STools_MeshBuffer& aMesh)
	{
		std::stringstream outputFileName;
		std::string modelName = GetFilename(aFile);

		outputFileName << "Assets/Imported/Models/";
		
		if (!aSubFolder.empty())
		{
			outputFileName << aSubFolder << "/";
			std::filesystem::create_directory(outputFileName.str());
		}

		outputFileName << modelName << ".smf";

		std::ofstream outputFile(outputFileName.str(), std::ios::binary);
		if (outputFile.good())
		{
			const STools_MeshBuffer& mesh = aMesh;

			SG_MeshFileHeader fileHeader;
			fileHeader.myNumLods = mesh.myLods.Count();
			fileHeader.myNameSize = (uint)modelName.size();
			fileHeader.myMaterialPathSize = (uint)mesh.myMaterialPath.size();
			outputFile.write((char*)&fileHeader, sizeof(fileHeader));
			outputFile.write(modelName.c_str(), fileHeader.myNameSize);
			outputFile.write(mesh.myMaterialPath.c_str(), fileHeader.myMaterialPathSize);

			uint largestAlloc = 0;
			char* data = nullptr;

			for (int i = -1, count = fileHeader.myNumLods; i < count; ++i)
			{
				const STools_MeshLod& lod = (i == -1) ? mesh.myMesh : mesh.myLods[i];
				if (!lod.myIsEnabled)
					continue;

				SG_MeshHeader header;
				header.myAABB = lod.myAABB;
				header.myVertexStrideSize = lod.myVertexStrideSize;
				header.myNumVertices = lod.myNumVertices;
				header.myNumIndices = lod.myNumIndices;
				header.myTotalVertexBufferSize = lod.myTotalVertexBufferSize;
				header.myTotalIndexBufferSize = lod.myTotalIndexBufferSize;

				uint meshDataSize = 0;
				meshDataSize += sizeof(SG_MeshHeader);
				meshDataSize += header.myTotalVertexBufferSize + header.myTotalIndexBufferSize;

				if (meshDataSize > largestAlloc)
				{
					delete[] data;
					data = new char[meshDataSize];
					largestAlloc = meshDataSize;
				}

				uint currPos = 0;
				SC_Memcpy(&data[currPos], &header, sizeof(header));
				currPos += sizeof(header);

				SC_Memcpy(&data[currPos], lod.myVertices, header.myTotalVertexBufferSize);
				currPos += header.myTotalVertexBufferSize;

				SC_Memcpy(&data[currPos], lod.myIndices, header.myTotalIndexBufferSize);
				currPos += header.myTotalIndexBufferSize;

				outputFile.write(data, meshDataSize);
				delete[] lod.myVertices;
				delete[] lod.myIndices;
			}
			delete[] data;
		}
		else
			return false;

		outputFile.close();

		SC_SUCCESS_LOG("Saved mesh [%s] with material [%s].", modelName.c_str(), aMesh.myMaterialPath.c_str());

		return true;
	}

	void STools_AssetImporter::ProcessAiNode(aiNode* aNode, const aiScene* aScene, const uint /*aLodIndex*/, SC_GrowingArray<STools_MeshBuffer>& aOutMeshes)
	{
		uint numMeshes = aNode->mNumMeshes;
		aOutMeshes.PreAllocate(numMeshes);
		for (unsigned int meshIdx = 0; meshIdx < numMeshes; ++meshIdx)
		{
			const aiMesh* mesh = aScene->mMeshes[aNode->mMeshes[meshIdx]];

			uint vertexStrideSize = sizeof(SC_Vector4f);

			bool hasUVs = mesh->HasTextureCoords(0);
			bool hasNormals = mesh->HasNormals();
			bool hasTangentsAndBitangents = mesh->HasTangentsAndBitangents();
			bool hasVertexColors = mesh->HasVertexColors(0);

			vertexStrideSize += sizeof(SC_Vector4f);
			vertexStrideSize += sizeof(SC_Vector3f);
			vertexStrideSize += sizeof(SC_Vector3f) * 2;
			vertexStrideSize += sizeof(SC_Vector2f);

			uint vertexDataArraySize = mesh->mNumVertices * vertexStrideSize;
			uint8* vertexDataArray = new uint8[vertexDataArraySize];
			uint currentPos = 0;

			SC_AABB meshAABB;
			meshAABB.myMax = SC_Vector3f(SC_FLT_LOWEST);
			meshAABB.myMin = SC_Vector3f(SC_FLT_MAX);
			for (uint i = 0; i < mesh->mNumVertices; ++i)
			{
				SC_Vector4f position = SC_Vector4f(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z, 1.0f);
				SC_Memcpy(&vertexDataArray[currentPos], &position, sizeof(SC_Vector4f));
				currentPos += sizeof(SC_Vector4f);

				if (position.x < meshAABB.myMin.x)
					meshAABB.myMin.x = position.x;
				else if (position.x > meshAABB.myMax.x)
					meshAABB.myMax.x = position.x;

				if (position.y < meshAABB.myMin.y)
					meshAABB.myMin.y = position.y;
				else if (position.y > meshAABB.myMax.y)
					meshAABB.myMax.y = position.y;

				if (position.z < meshAABB.myMin.z)
					meshAABB.myMin.z = position.z;
				else if (position.z > meshAABB.myMax.z)
					meshAABB.myMax.z = position.z;

				SC_Vector4f color(1.0f);
				if (hasVertexColors)
					color = SC_Vector4f(mesh->mColors[0][i].r, mesh->mColors[0][i].g, mesh->mColors[0][i].b, mesh->mColors[0][i].a);
				SC_Memcpy(&vertexDataArray[currentPos], &color, sizeof(SC_Vector4f));
				currentPos += sizeof(SC_Vector4f);

				SC_Vector3f normal;
				if (hasNormals)
					normal = SC_Vector3f(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
				SC_Memcpy(&vertexDataArray[currentPos], &normal, sizeof(SC_Vector3f));
				currentPos += sizeof(SC_Vector3f);

				SC_Vector3f data[2];
				if (hasTangentsAndBitangents)
				{
					data[0] = SC_Vector3f(mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z);
					data[1] = SC_Vector3f(mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z);
				}
				SC_Memcpy(&vertexDataArray[currentPos], data, sizeof(SC_Vector3f) * 2);
				currentPos += (sizeof(SC_Vector3f) * 2);

				SC_Vector2f uv;
				if (hasUVs)
					uv = SC_Vector2f(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
				SC_Memcpy(&vertexDataArray[currentPos], &uv, sizeof(SC_Vector2f));
				currentPos += sizeof(SC_Vector2f);
			}

			const uint numIndices = mesh->mNumFaces * 3; // Assume 3 indices per face.
			uint* indices = new uint[numIndices];
			uint indexNum = 0;
			for (uint i = 0; i < mesh->mNumFaces; ++i)
			{
				const aiFace& face = mesh->mFaces[i];
				assert(face.mNumIndices == 3 && "Mesh isn't triangulated.");

				for (uint index = 0; index < face.mNumIndices; ++index)
					indices[indexNum++] = face.mIndices[index];
			}


			SC_GrowingArray<std::string> textures;
			const aiMaterial* material = aScene->mMaterials[mesh->mMaterialIndex];
			aiString path;
			for (uint i = 0; i < AI_TEXTURE_TYPE_MAX; ++i)
			{
				aiReturn texFound = material->GetTexture((aiTextureType)i, 0, &path);
				if (texFound != AI_SUCCESS)
					continue;

				std::string texturePath = GetFileWithExtension(std::string(path.data));

				textures.Add(texturePath);
			}

			STools_MeshBuffer& outMesh = aOutMeshes.Add();
			outMesh.myMesh.myAABB = meshAABB;
			outMesh.myMesh.myVertices = vertexDataArray;
			outMesh.myMesh.myNumVertices = mesh->mNumVertices;
			outMesh.myMesh.myVertexStrideSize = vertexStrideSize;
			outMesh.myMesh.myIndices = indices;
			outMesh.myMesh.myNumIndices = numIndices;
			outMesh.myMesh.myTotalVertexBufferSize = vertexDataArraySize;
			outMesh.myMesh.myTotalIndexBufferSize = (sizeof(uint) * outMesh.myMesh.myNumIndices);
			outMesh.myName = mesh->mName.C_Str();

			if (outMesh.myName.rfind("_column_b") != std::string::npos)
				outMesh.myMaterialPath = "Assets/Materials/Sponza/Sponza_Column_B.smtf";
			else if (outMesh.myName.rfind("_fabric_c") != std::string::npos)
				outMesh.myMaterialPath = "Assets/Materials/Sponza/Sponza_Curtain_Red.smtf";
			else if (outMesh.myName.rfind("_fabric_g") != std::string::npos)
				outMesh.myMaterialPath = "Assets/Materials/Sponza/Sponza_Curtain_Blue.smtf";
			else if (outMesh.myName.rfind("_fabric_f") != std::string::npos)
				outMesh.myMaterialPath = "Assets/Materials/Sponza/Sponza_Curtain_Green.smtf";
			else if (outMesh.myName.rfind("_flagpole") != std::string::npos)
				outMesh.myMaterialPath = "Assets/Materials/Sponza/Sponza_FlagPole.smtf";
			else if (outMesh.myName.rfind("_column_c") != std::string::npos)
				outMesh.myMaterialPath = "Assets/Materials/Sponza/Sponza_Column_C.smtf";
			else if (outMesh.myName.rfind("_column_a") != std::string::npos)
				outMesh.myMaterialPath = "Assets/Materials/Sponza/Sponza_Column_A.smtf";
			else if (outMesh.myName.rfind("_fabric_a") != std::string::npos)
				outMesh.myMaterialPath = "Assets/Materials/Sponza/Sponza_Fabric_Red.smtf";
			else if (outMesh.myName.rfind("_floor") != std::string::npos)
				outMesh.myMaterialPath = "Assets/Materials/Sponza/Sponza_Floor.smtf";
			else if (outMesh.myName.rfind("_roof") != std::string::npos)
				outMesh.myMaterialPath = "Assets/Materials/Sponza/Sponza_Roof.smtf";
			else if (outMesh.myName.rfind("_bricks") != std::string::npos)
				outMesh.myMaterialPath = "Assets/Materials/Sponza/Sponza_Bricks_A.smtf";
			else if (outMesh.myName.rfind("_details") != std::string::npos)
				outMesh.myMaterialPath = "Assets/Materials/Sponza/Sponza_Details.smtf";
			else if (outMesh.myName.rfind("_vase_hanging") != std::string::npos)
				outMesh.myMaterialPath = "Assets/Materials/Sponza/Sponza_VaseHanging.smtf";
			else if (outMesh.myName.rfind("_chain") != std::string::npos)
				outMesh.myMaterialPath = "Assets/Materials/Sponza/Sponza_Chain.smtf";
			else if (outMesh.myName.rfind("_arch") != std::string::npos)
				outMesh.myMaterialPath = "Assets/Materials/Sponza/Sponza_Arch.smtf";
			else if (outMesh.myName.rfind("_ceiling") != std::string::npos)
				outMesh.myMaterialPath = "Assets/Materials/Sponza/Sponza_Ceiling.smtf";
			else if (outMesh.myName.rfind("_fabric_e") != std::string::npos)
				outMesh.myMaterialPath = "Assets/Materials/Sponza/Sponza_Fabric_Green.smtf";
			else if (outMesh.myName.rfind("_fabric_d") != std::string::npos)
				outMesh.myMaterialPath = "Assets/Materials/Sponza/Sponza_Fabric_Blue.smtf";
			else if (outMesh.myName.rfind("_leaf") != std::string::npos)
				outMesh.myMaterialPath = "Assets/Materials/Sponza/Sponza_Leaf.smtf";
			else if (outMesh.myName.rfind("_Material__57") != std::string::npos)
				outMesh.myMaterialPath = "Assets/Materials/Sponza/Sponza_VasePlant.smtf";
			else if (outMesh.myName.rfind("_Material__298") != std::string::npos)
				outMesh.myMaterialPath = "Assets/Materials/Sponza/Sponza_Background.smtf";
			else if (outMesh.myName.rfind("_Material__25") != std::string::npos)
				outMesh.myMaterialPath = "Assets/Materials/Sponza/Sponza_Lion.smtf";
			else if (outMesh.myName.rfind("_vase") != std::string::npos)
				outMesh.myMaterialPath = "Assets/Materials/Sponza/Sponza_Vase.smtf";
			else if (outMesh.myName.rfind("_vase_round") != std::string::npos)
				outMesh.myMaterialPath = "Assets/Materials/Sponza/Sponza_VaseRound.smtf";

			outMesh.myTextures = SC_Move(textures);
		}
		
		for (unsigned int n = 0; n < aNode->mNumChildren; ++n)
		{
			//ProcessAiNode(aNode->mChildren[n], aScene, aOutMeshes);
		}
	}

	void STools_AssetImporter::ExtractMesh(const aiScene* aScene, uint aMeshIndex, STools_MeshLod& aMesh)
	{
		if (!aScene->HasMeshes())
		{
			SC_ERROR_LOG("Model-file contains no meshes");
			return;
		}

		const aiMesh* mesh = aScene->mMeshes[aMeshIndex];
		uint vertexStrideSize = sizeof(SC_Vector4f);

		bool hasUVs = mesh->HasTextureCoords(0);
		bool hasNormals = mesh->HasNormals();
		bool hasTangentsAndBitangents = mesh->HasTangentsAndBitangents();
		bool hasVertexColors = mesh->HasVertexColors(0);

		vertexStrideSize += sizeof(SC_Vector4f);
		vertexStrideSize += sizeof(SC_Vector3f);
		vertexStrideSize += sizeof(SC_Vector3f) * 2;
		vertexStrideSize += sizeof(SC_Vector2f);

		uint vertexDataArraySize = mesh->mNumVertices * vertexStrideSize;
		uint8* vertexDataArray = new uint8[vertexDataArraySize];
		uint currentPos = 0;

		SC_AABB meshAABB;
		meshAABB.myMax = SC_Vector3f(SC_FLT_LOWEST);
		meshAABB.myMin = SC_Vector3f(SC_FLT_MAX);
		for (uint i = 0; i < mesh->mNumVertices; ++i)
		{
			SC_Vector4f position = SC_Vector4f(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z, 1.0f);
			SC_Memcpy(&vertexDataArray[currentPos], &position, sizeof(SC_Vector4f));
			currentPos += sizeof(SC_Vector4f);

			if (position.x < meshAABB.myMin.x)
				meshAABB.myMin.x = position.x;
			else if (position.x > meshAABB.myMax.x)
				meshAABB.myMax.x = position.x;

			if (position.y < meshAABB.myMin.y)
				meshAABB.myMin.y = position.y;
			else if (position.y > meshAABB.myMax.y)
				meshAABB.myMax.y = position.y;

			if (position.z < meshAABB.myMin.z)
				meshAABB.myMin.z = position.z;
			else if (position.z > meshAABB.myMax.z)
				meshAABB.myMax.z = position.z;

			SC_Vector4f color(1.0f);
			if (hasVertexColors)
				color = SC_Vector4f(mesh->mColors[0][i].r, mesh->mColors[0][i].g, mesh->mColors[0][i].b, mesh->mColors[0][i].a);
			SC_Memcpy(&vertexDataArray[currentPos], &color, sizeof(SC_Vector4f));
			currentPos += sizeof(SC_Vector4f);

			SC_Vector3f normal;
			if (hasNormals)
				normal = SC_Vector3f(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
			SC_Memcpy(&vertexDataArray[currentPos], &normal, sizeof(SC_Vector3f));
			currentPos += sizeof(SC_Vector3f);

			SC_Vector3f data[2];
			if (hasTangentsAndBitangents)
			{
				data[0] = SC_Vector3f(mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z);
				data[1] = SC_Vector3f(mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z);
			}
			SC_Memcpy(&vertexDataArray[currentPos], data, sizeof(SC_Vector3f) * 2);
			currentPos += (sizeof(SC_Vector3f) * 2);

			SC_Vector2f uv;
			if (hasUVs)
				uv = SC_Vector2f(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
			SC_Memcpy(&vertexDataArray[currentPos], &uv, sizeof(SC_Vector2f));
			currentPos += sizeof(SC_Vector2f);
		}

		const uint numIndices = mesh->mNumFaces * 3; // Assume 3 indices per face.
		uint* indices = new uint[numIndices];
		uint indexNum = 0;
		for (uint i = 0; i < mesh->mNumFaces; ++i)
		{
			const aiFace& face = mesh->mFaces[i];
			assert(face.mNumIndices == 3 && "Mesh isn't triangulated.");

			for (uint index = 0; index < face.mNumIndices; ++index)
				indices[indexNum++] = face.mIndices[index];
		}

		STools_MeshLod& outMesh = aMesh;
		outMesh.myAABB = meshAABB;
		outMesh.myVertices = vertexDataArray;
		outMesh.myNumVertices = mesh->mNumVertices;
		outMesh.myVertexStrideSize = vertexStrideSize;
		outMesh.myIndices = indices;
		outMesh.myNumIndices = numIndices;
		outMesh.myTotalVertexBufferSize = vertexDataArraySize;
		outMesh.myTotalIndexBufferSize = (sizeof(uint) * outMesh.myNumIndices);
		outMesh.myIsEnabled = true;
	}
}