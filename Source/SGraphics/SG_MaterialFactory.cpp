#include "SGraphics_Precompiled.h"
#include "SG_MaterialFactory.h"

#include "SR_ShaderCompiler.h"

#include <sstream>
#include "SG_CommonRenderStates.h"

namespace Shift
{
	static SC_Mutex locLock;
	SC_HashMap<uint, SC_Ref<SG_Material>> SG_MaterialFactory::ourCache;

	SC_Ref<SG_Material> SG_MaterialFactory::GetCreateMaterial(const char* aPath)
	{
		SG_MaterialDesc desc;
		desc.FromFile(aPath);

		return GetCreateMaterial(desc);
	}

	SC_Ref<SG_Material> SG_MaterialFactory::GetCreateMaterial(const SG_MaterialDesc& aDesc)
	{
		uint materialHash = SC_Hash(aDesc);

		SC_MutexLock lock(locLock);
		SC_Ref<SG_Material> material;
		if (ourCache.Find(materialHash, material))
			return material;

		material = new SG_Material();
		ourCache.Insert(materialHash, material);
		lock.Unlock();

		SR_GraphicsDevice* device = SR_GraphicsDevice::GetDevice();
		SR_ShaderCompiler* shaderCompiler = device->GetShaderCompiler();
		SG_CommonRenderStates* crs = SG_CommonRenderStates::GetInstance();

		material->myTopology = (aDesc.myUseTessellation) ? SR_Topology_Patch : SR_Topology_TriangleList;

		SC_Ref<SR_ShaderByteCode> vertexShader = shaderCompiler->CompileShaderFromFile(aDesc.myVertexShader.c_str(), SR_ShaderType_Vertex);

		SR_RasterizerState* rasterState = nullptr;
		if (aDesc.myIsOpaque)
		{
			// Depth shader
			SR_ShaderStateDesc shaderStateDesc;
			shaderStateDesc.myMainPSO.myTopology = material->myTopology;
			shaderStateDesc.myMainPSO.myDepthStateDesc.myDepthComparisonFunc = SR_ComparisonFunc_GreaterEqual;
			shaderStateDesc.myMainPSO.myDepthStateDesc.myWriteDepth = true;
			shaderStateDesc.myMainPSO.myRenderTargetFormats.myNumColorFormats = 0;

			if (aDesc.myIsTwoSided)
				rasterState = crs->myTwoSidedRasterizerState;
			else
				rasterState = crs->myDefaultRasterizerState;

			shaderStateDesc.myMainPSO.myRasterizerStateDesc = rasterState->myProperties;

			if (aDesc.myIsShadowCaster)
			{

				SR_ShaderStateDesc::PSOStruct& shadowPSO = shaderStateDesc.myAdditionalPSOs.Add();
				shadowPSO = shaderStateDesc.myMainPSO;

				if (aDesc.myIsTwoSided)
					rasterState = crs->myCSMTwoSidedRasterizerState;
				else
					rasterState = crs->myCSMRasterizerState;

				shadowPSO.myRasterizerStateDesc = rasterState->myProperties;
			}

			shaderStateDesc.myShaderByteCodes[SR_ShaderType_Vertex] = vertexShader;

			if (aDesc.myUseTessellation)
			{
				shaderStateDesc.myShaderByteCodes[SR_ShaderType_Hull] = shaderCompiler->CompileShaderFromFile(aDesc.myHullShader.c_str(), SR_ShaderType_Hull);
				shaderStateDesc.myShaderByteCodes[SR_ShaderType_Domain] = shaderCompiler->CompileShaderFromFile(aDesc.myDomainShader.c_str(), SR_ShaderType_Domain);
			}

			if (aDesc.myAlphaTest)
				shaderStateDesc.myShaderByteCodes[SR_ShaderType_Pixel] = shaderCompiler->CompileShaderFromFile("ShiftEngine/Shaders/SGraphics/TEMP_SHADERS/ObjectAlphaTestedPS.ssf", SR_ShaderType_Pixel);
			else	
				shaderStateDesc.myShaderByteCodes[SR_ShaderType_Pixel] = shaderCompiler->CompileShaderFromFile("ShiftEngine/Shaders/SGraphics/TEMP_SHADERS/ObjectDepthTestPS.ssf", SR_ShaderType_Pixel);


			material->myShaders[SG_RenderType_Depth] = device->CreateShaderState(shaderStateDesc);
		}

		if (aDesc.myNeedsForward)
		{
			// Color shader
			SR_ShaderStateDesc shaderStateDesc;
			shaderStateDesc.myMainPSO.myTopology = material->myTopology;
			shaderStateDesc.myMainPSO.myDepthStateDesc.myDepthComparisonFunc = SR_ComparisonFunc_Equal;
			shaderStateDesc.myMainPSO.myRenderTargetFormats.myNumColorFormats = 1;
			shaderStateDesc.myMainPSO.myRenderTargetFormats.myColorFormats[0] = SR_Format_RG11B10_Float; // Color

			if (aDesc.myIsTwoSided)
				rasterState = crs->myTwoSidedRasterizerState;
			else
				rasterState = crs->myDefaultRasterizerState;

			shaderStateDesc.myMainPSO.myRasterizerStateDesc = rasterState->myProperties;

			shaderStateDesc.myShaderByteCodes[SR_ShaderType_Vertex] = vertexShader;
			shaderStateDesc.myShaderByteCodes[SR_ShaderType_Pixel] = shaderCompiler->CompileShaderFromFile(aDesc.myPixelShader.c_str(), SR_ShaderType_Pixel);
			material->myShaders[SG_RenderType_Color] = device->CreateShaderState(shaderStateDesc);
		}

		if (aDesc.myNeedsGBuffer)
		{
			// GBuffer shader
			SR_ShaderStateDesc shaderStateDesc;
			shaderStateDesc.myMainPSO.myTopology = material->myTopology;
			shaderStateDesc.myMainPSO.myDepthStateDesc.myDepthComparisonFunc = SR_ComparisonFunc_Equal;
			shaderStateDesc.myMainPSO.myRenderTargetFormats.myNumColorFormats = 4;
			shaderStateDesc.myMainPSO.myRenderTargetFormats.myColorFormats[0] = SR_Format_RGBA8_Unorm; // Color
			shaderStateDesc.myMainPSO.myRenderTargetFormats.myColorFormats[1] = SR_Format_RG11B10_Float; // Normals
			shaderStateDesc.myMainPSO.myRenderTargetFormats.myColorFormats[2] = SR_Format_RGBA8_Unorm; // ARM
			shaderStateDesc.myMainPSO.myRenderTargetFormats.myColorFormats[3] = SR_Format_RGBA8_Unorm; // Emissive + Fog

			if (aDesc.myIsTwoSided)
				rasterState = crs->myTwoSidedRasterizerState;
			else
				rasterState = crs->myDefaultRasterizerState;

			shaderStateDesc.myMainPSO.myRasterizerStateDesc = rasterState->myProperties;

			shaderStateDesc.myMainPSO.myBlendStateDesc.myNumRenderTargets = 4;
			for (uint i = 0; i < 4; ++i)
			{
				shaderStateDesc.myMainPSO.myBlendStateDesc.myRenderTagetBlendDescs[i].mySrcBlend = SR_BlendMode_One;
				shaderStateDesc.myMainPSO.myBlendStateDesc.myRenderTagetBlendDescs[i].myDstBlend = SR_BlendMode_Zero;
				shaderStateDesc.myMainPSO.myBlendStateDesc.myRenderTagetBlendDescs[i].mySrcBlendAlpha = SR_BlendMode_One;
				shaderStateDesc.myMainPSO.myBlendStateDesc.myRenderTagetBlendDescs[i].myDstBlendAlpha = SR_BlendMode_Zero;
			}

			shaderStateDesc.myShaderByteCodes[SR_ShaderType_Vertex] = vertexShader; 
			
			if (aDesc.myUseTessellation)
			{
				shaderStateDesc.myShaderByteCodes[SR_ShaderType_Hull] = shaderCompiler->CompileShaderFromFile(aDesc.myHullShader.c_str(), SR_ShaderType_Hull);
				shaderStateDesc.myShaderByteCodes[SR_ShaderType_Domain] = shaderCompiler->CompileShaderFromFile(aDesc.myDomainShader.c_str(), SR_ShaderType_Domain);
			}

			shaderStateDesc.myShaderByteCodes[SR_ShaderType_Pixel] = shaderCompiler->CompileShaderFromFile(aDesc.myPixelShader.c_str(), SR_ShaderType_Pixel);
			material->myShaders[SG_RenderType_GBuffer] = device->CreateShaderState(shaderStateDesc);
		}

		bool canVoxelize = true;
		if (canVoxelize)
		{
			SR_ShaderStateDesc shaderStateDesc;
			shaderStateDesc.myMainPSO.myTopology = SR_Topology_TriangleList;
			shaderStateDesc.myMainPSO.myRenderTargetFormats.myNumColorFormats = 0;

			shaderStateDesc.myShaderByteCodes[SR_ShaderType_Vertex] = shaderCompiler->CompileShaderFromFile("ShiftEngine/Shaders/SGraphics/TEMP_SHADERS/ObjectInstancedVS.ssf", SR_ShaderType_Vertex);
			shaderStateDesc.myShaderByteCodes[SR_ShaderType_Geometry] = shaderCompiler->CompileShaderFromFile("ShiftEngine/Shaders/SGraphics/TEMP_SHADERS/ObjectVoxelizedGS.ssf", SR_ShaderType_Geometry);
			shaderStateDesc.myShaderByteCodes[SR_ShaderType_Pixel] = shaderCompiler->CompileShaderFromFile("ShiftEngine/Shaders/SGraphics/TEMP_SHADERS/ObjectVoxelizedPS.ssf", SR_ShaderType_Pixel);
			material->myShaders[SG_RenderType_Voxelize] = device->CreateShaderState(shaderStateDesc);
		}

		// Textures
		material->myTextures.PreAllocate(aDesc.myTextures.Count());
		for (uint i = 0; i < aDesc.myTextures.Count(); ++i)
			material->myTextures.Add(device->GetCreateTexture(aDesc.myTextures[i].c_str()));

		material->myProperties = aDesc;
		material->SetState(SC_LoadState::Loaded);
		return material;
	}
}
