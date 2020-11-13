#include "SGraphics_Precompiled.h"
#include "SG_Terrain.h"
#include "SG_CommonRenderStates.h"

namespace Shift
{
	SG_Terrain::SG_Terrain()
	{
	}

	SG_Terrain::~SG_Terrain()
	{
	}

	bool SG_Terrain::Init(const SG_TerrainInitData& aInitData)
	{
		// Create lowest LOD mesh.
		// Tesselate for detail
		// Use heightmap
		//

		SR_GraphicsDevice* device = SR_GraphicsDevice::GetDevice();
		SR_ShaderCompiler* shaderCompiler = device->GetShaderCompiler();

		myHeightMap = device->GetCreateTexture("ShiftEngine/Textures/HeightMapGen.png");
		myNormalMap = device->GetCreateTexture("ShiftEngine/Textures/HeightMapGenNorm.png");

		SR_ShaderStateDesc colorShaderDesc;
		colorShaderDesc.myMainPSO.myTopology = SR_Topology_TriangleList;
		colorShaderDesc.myMainPSO.myDepthStateDesc.myDepthComparisonFunc = SR_ComparisonFunc_Equal;
		colorShaderDesc.myMainPSO.myRenderTargetFormats.myNumColorFormats = 4;
		colorShaderDesc.myMainPSO.myRenderTargetFormats.myColorFormats[0] = SR_Format_RGBA8_Unorm; // Color
		colorShaderDesc.myMainPSO.myRenderTargetFormats.myColorFormats[1] = SR_Format_RG11B10_Float; // Normals
		colorShaderDesc.myMainPSO.myRenderTargetFormats.myColorFormats[2] = SR_Format_RGBA8_Unorm; // ARM
		colorShaderDesc.myMainPSO.myRenderTargetFormats.myColorFormats[3] = SR_Format_RGBA8_Unorm; // Emissive + Fog

		for (uint i = 0; i < 4; ++i)
		{
			colorShaderDesc.myMainPSO.myBlendStateDesc.myRenderTagetBlendDescs[i].mySrcBlend = SR_BlendMode_One;
			colorShaderDesc.myMainPSO.myBlendStateDesc.myRenderTagetBlendDescs[i].myDstBlend = SR_BlendMode_Zero;
			colorShaderDesc.myMainPSO.myBlendStateDesc.myRenderTagetBlendDescs[i].mySrcBlendAlpha = SR_BlendMode_One;
			colorShaderDesc.myMainPSO.myBlendStateDesc.myRenderTagetBlendDescs[i].myDstBlendAlpha = SR_BlendMode_Zero;
		}

		colorShaderDesc.myShaderByteCodes[SR_ShaderType_Vertex] = shaderCompiler->CompileShaderFromFile("ShiftEngine/Shaders/SGraphics/TEMP_SHADERS/ObjectTerrainVS.ssf", SR_ShaderType_Vertex);
		colorShaderDesc.myShaderByteCodes[SR_ShaderType_Pixel] = shaderCompiler->CompileShaderFromFile("ShiftEngine/Shaders/SGraphics/TEMP_SHADERS/ObjectTerrainPS.ssf", SR_ShaderType_Pixel);
		myShaders[Color] = device->CreateShaderState(colorShaderDesc);

		SR_ShaderStateDesc depthShaderDesc;
		depthShaderDesc.myMainPSO.myTopology = SR_Topology_TriangleList;
		colorShaderDesc.myMainPSO.myDepthStateDesc.myDepthComparisonFunc = SR_ComparisonFunc_GreaterEqual;
		colorShaderDesc.myMainPSO.myDepthStateDesc.myWriteDepth = true;

		SR_ShaderStateDesc::PSOStruct& shadowPSO = depthShaderDesc.myAdditionalPSOs.Add();
		shadowPSO = depthShaderDesc.myMainPSO;
		shadowPSO.myRasterizerStateDesc.myDepthBias = -100;
		shadowPSO.myRasterizerStateDesc.mySlopedScaleDepthBias = -8.0;
		shadowPSO.myRasterizerStateDesc.myEnableDepthClip = false;

		depthShaderDesc.myShaderByteCodes[SR_ShaderType_Vertex] = shaderCompiler->CompileShaderFromFile("ShiftEngine/Shaders/SGraphics/TEMP_SHADERS/ObjectTerrainVS.ssf", SR_ShaderType_Vertex);
		depthShaderDesc.myShaderByteCodes[SR_ShaderType_Pixel] = shaderCompiler->CompileShaderFromFile("ShiftEngine/Shaders/SGraphics/TEMP_SHADERS/ObjectDepthTestPS.ssf", SR_ShaderType_Pixel);
		myShaders[Depth] = device->CreateShaderState(depthShaderDesc);

		SC_GrowingArray<SVertex_Terrain> vertices;
		SC_GrowingArray<uint> indices; 

		SC_Vector2f uvIncrements = { 1.0f / aInitData.myResolution.x, 1.0f / aInitData.myResolution.y };

		int halfResY = (int)(aInitData.myResolution.y * 0.5f);
		int halfResX = (int)(aInitData.myResolution.x * 0.5f);

		for (uint y = 0; y < aInitData.myResolution.y; ++y)
		{
			for (uint x = 0; x < aInitData.myResolution.x; ++x)
			{
				SVertex_Terrain& vertex = vertices.Add();
				vertex.myPosition = {x * aInitData.mySectorSize - halfResX, 0.0f, y * aInitData.mySectorSize - halfResY, 1.0f };
				vertex.myUV = { x * uvIncrements.x, y * uvIncrements.y };
			}
		}
		
		for (uint y = 0; y < aInitData.myResolution.y - 1; ++y)
		{
			for (uint x = 0; x < aInitData.myResolution.x - 1; ++x)
			{
				indices.Add(uint(x + (y * aInitData.myResolution.x)));									//current vertex
				indices.Add(uint(x + (y * aInitData.myResolution.x) + aInitData.myResolution.x));			//current vertex + row
				indices.Add(uint(x + (y * aInitData.myResolution.x) + aInitData.myResolution.x + 1));		//current vertex + row + one right

				indices.Add(uint(x + (y * aInitData.myResolution.x)));									//current vertex
				indices.Add(uint(x + (y * aInitData.myResolution.x) + aInitData.myResolution.x + 1));		//current vertex + row + one right
				indices.Add(uint(x + (y * aInitData.myResolution.x) + 1));								//current vertex + one right
			}
		}

		SR_BufferDesc vBufferDesc;
		vBufferDesc.mySize = vertices.Count() *sizeof(SVertex_Terrain);
		vBufferDesc.myStructSize = sizeof(SVertex_Terrain);
		vBufferDesc.myBindFlag = SR_BindFlag_VertexBuffer;
		myVertexBuffer = device->CreateBuffer(vBufferDesc, vertices.GetBuffer(), "Terrain Vertex Buffer");

		SR_BufferDesc iBufferDesc;
		iBufferDesc.mySize = indices.Count() * sizeof(uint);
		iBufferDesc.myStructSize = sizeof(uint);
		iBufferDesc.myBindFlag = SR_BindFlag_IndexBuffer;
		myIndexBuffer = device->CreateBuffer(iBufferDesc, indices.GetBuffer(), "Terrain Index Buffer");

		return false;
	}
	void SG_Terrain::Render(const SG_RenderData&, bool aOnlyDepth)
	{
		SR_GraphicsContext* ctx = SR_GraphicsContext::GetCurrent();
		SG_CommonRenderStates* crs = SG_CommonRenderStates::GetInstance();

		SR_PROFILER_FUNCTION_TAGGED("Render Terrain");
		ctx->BindTexture(myHeightMap, 0);
		ctx->BindTexture(myNormalMap, 1);

		ctx->SetIndexBuffer(myIndexBuffer);
		ctx->SetVertexBuffer(0, myVertexBuffer);


		if (aOnlyDepth)
		{
			ctx->SetBlendState(crs->myDefaultBlendState, SC_Vector4f(0.0f));
			ctx->SetDepthState(crs->myDepthGreaterEqualWriteState, 0);
			ctx->SetShaderState(myShaders[Depth]);
		}
		else
		{
			ctx->SetBlendState(crs->myDeferredBlendState, SC_Vector4f(0.0f));
			ctx->SetDepthState(crs->myDepthEqualReadState, 0);
			ctx->SetShaderState(myShaders[Color]);
		}

		uint numIndices = myIndexBuffer->GetProperties().mySize / myIndexBuffer->GetProperties().myStructSize;
		ctx->DrawIndexed(numIndices, 0, 0);

	}
}