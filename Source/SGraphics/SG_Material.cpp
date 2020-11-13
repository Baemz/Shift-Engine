#include "SGraphics_Precompiled.h"
#include "SG_Material.h"

#include <sstream>

namespace Shift
{
	void SG_MaterialDesc::FromFile(const char* aFilePath)
	{
		std::ifstream in(aFilePath);
		if (in.fail())
		{
			SC_ERROR_LOG("Could not open file [%s]", aFilePath);
			return;
		}
		SC_Json descFile;
		in >> descFile;
		in.close();

		uint versionNumber = descFile["VersionNumber"];
		myIsOpaque = descFile["IsOpaque"];
		myNeedsGBuffer = descFile["NeedsGBuffer"];
		myNeedsForward = descFile["NeedsForward"];
		myVertexShader = descFile["VertexShader"];
		myPixelShader = descFile["PixelShader"];
		myIsShadowCaster = descFile["IsShadowCaster"];
		myLodPresetId = descFile["LodPresetId"].get<std::string>();

		if (descFile.find("TwoSided") != descFile.end())
			myIsTwoSided = descFile["TwoSided"];

		uint numTextures = (uint)descFile["Textures"].size();
		myTextures.PreAllocate(numTextures);
		for (uint i = 0; i < numTextures; ++i)
		{
			myTextures.Add(descFile["Textures"][i].get<std::string>());
		}

		if (versionNumber > 0x1)
		{
			myAlphaTest = descFile["AlphaTest"];
			myUseTessellation = descFile["Tessellation"];

			if (myUseTessellation)
			{
				myHullShader = "ShiftEngine/Shaders/SGraphics/TEMP_SHADERS/ObjectStandardHS.ssf";
				myDomainShader = "ShiftEngine/Shaders/SGraphics/TEMP_SHADERS/ObjectStandardDS.ssf";
			}

			myAddToRaytracingScene = descFile["RaytracingMesh"];
		}
	}

	bool SG_MaterialDesc::ToFile(const char* aFilePath)
	{
		SC_Json descFile;
		descFile["VersionNumber"] = ourMaterialDescVersion;
		descFile["IsOpaque"] = myIsOpaque;
		descFile["NeedsGBuffer"] = myNeedsGBuffer;
		descFile["NeedsForward"] = myNeedsForward;
		descFile["VertexShader"] = myVertexShader;
		descFile["PixelShader"] = myPixelShader;
		descFile["IsShadowCaster"] = myIsShadowCaster;
		descFile["AlphaTest"] = myAlphaTest;
		descFile["TwoSided"] = myIsTwoSided;
		descFile["Tessellation"] = myUseTessellation;
		descFile["LodPresetId"] = myLodPresetId;
		
		for (uint i = 0, count = myTextures.Count(); i < count; ++i)
			descFile["Textures"][i] = myTextures[i];

		std::ofstream outFile(aFilePath);
		if (outFile.is_open())
		{
			outFile << descFile;
			return true;
		}
		else
			return false;
	}

	SG_Material::SG_Material()
		: myTopology(SR_Topology_TriangleList)
	{
	}

	SG_Material::~SG_Material()
	{
	}

	void SG_Material::BindTextures()
	{
		SR_GraphicsContext* ctx = SR_GraphicsContext::GetCurrent();

		for (uint i = 0, count = myTextures.Count(); i < count; ++i)
			ctx->BindTexture(myTextures[i], i);

	}
	void SG_Material::BindConstants()
	{
	}
}