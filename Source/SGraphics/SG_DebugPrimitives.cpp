#include "SGraphics_Precompiled.h"
#include "SG_DebugPrimitives.h"

#if ENABLE_DEBUG_PRIMITIVES
#include "SG_View.h"
#include "SG_CommonRenderStates.h"

namespace Shift
{
	SG_DebugPrimitives* SG_DebugPrimitives::ourInstance = nullptr;

	static const SC_Vector4f locCubeVertices[] = {
		SC_Vector4f(-0.5f,  0.5f, -0.5f, 1.0f),
		SC_Vector4f( 0.5f,  0.5f, -0.5f, 1.0f),
		SC_Vector4f(-0.5f, -0.5f, -0.5f, 1.0f),
		SC_Vector4f( 0.5f, -0.5f, -0.5f, 1.0f),
		SC_Vector4f(-0.5f,  0.5f,  0.5f, 1.0f),
		SC_Vector4f( 0.5f,  0.5f,  0.5f, 1.0f),
		SC_Vector4f(-0.5f, -0.5f,  0.5f, 1.0f),
		SC_Vector4f( 0.5f, -0.5f,  0.5f, 1.0f)
	};

	static const uint locCubeIndices[] =
	{
		0, 1, 2,   // side 1
		2, 1, 3,
		4, 0, 6,   // side 2
		6, 0, 2,
		7, 5, 6,   // side 3
		6, 5, 4,
		3, 1, 7,   // side 4
		7, 1, 5,
		4, 5, 0,   // side 5
		0, 5, 1,
		3, 7, 2,   // side 6
		2, 7, 6,
	};

	SC_Vector4f GetTransformedCubeVertex(const SC_Matrix44& aTransform, uint aIndex)
	{
		return locCubeVertices[aIndex] * aTransform;
	}

	void SG_DebugPrimitives::Create()
	{
		if (!ourInstance)
			ourInstance = new SG_DebugPrimitives();
	}

	void SG_DebugPrimitives::Destroy()
	{
		delete ourInstance;
	}

	void SG_DebugPrimitives::Clear()
	{
		{
			SC_MutexLock lock(myLinesMutex);
			myLineVertices3D.RemoveAll();
		}
		{
			SC_MutexLock lock(myTriangleMutex);
			myTriangleVertices3D.RemoveAll();
		}
	}

	void SG_DebugPrimitives::DrawLine3D(const SC_Vector3f& aStartPos, const SC_Vector3f& aEndPos)
	{
		DrawLine3D(aStartPos, aEndPos, { 1.0f, 1.0f, 1.0f, 1.0f });
	}

	void SG_DebugPrimitives::DrawLine3D(const SC_Vector3f& aStartPos, const SC_Vector3f& aEndPos, const SC_Vector4f& aColor)
	{
		DrawLine3D(aStartPos, aEndPos, aColor, aColor);
	}

	void SG_DebugPrimitives::DrawLine3D(const SC_Vector3f& aStartPos, const SC_Vector3f& aEndPos, const SC_Vector4f& aStartColor, const SC_Vector4f& aEndColor)
	{
		SG_PrimitiveVertex startVert;
		startVert.myPosition = aStartPos;
		startVert.myColor = aStartColor;

		SG_PrimitiveVertex endVert;
		endVert.myPosition = aEndPos;
		endVert.myColor = aEndColor;

		SC_MutexLock lock(myLinesMutex);
		myLineVertices3D.Add(startVert);
		myLineVertices3D.Add(endVert);
	}

	void SG_DebugPrimitives::DrawTriangle3D(const SC_Vector3f& aCorner1, const SC_Vector3f& aCorner2, const SC_Vector3f& aCorner3, const SC_Vector4f& aColor)
	{
		SG_PrimitiveVertex firstCorner;
		firstCorner.myPosition = aCorner1;
		firstCorner.myColor = aColor;

		SG_PrimitiveVertex secondCorner;
		secondCorner.myPosition = aCorner2;
		secondCorner.myColor = aColor;

		SG_PrimitiveVertex thirdCorner;
		thirdCorner.myPosition = aCorner3;
		thirdCorner.myColor = aColor;

		SC_MutexLock lock(myTriangleMutex);
		myTriangleVertices3D.Add(firstCorner);
		myTriangleVertices3D.Add(secondCorner);
		myTriangleVertices3D.Add(thirdCorner);
	}

	void SG_DebugPrimitives::DrawBox3D(const SC_Matrix44& aTransform, const SC_Vector4f& aColor)
	{
		SG_PrimitiveVertex vertices[36];
		for (uint i = 0; i < 36; ++i)
		{
			vertices[i].myPosition = GetTransformedCubeVertex(aTransform, locCubeIndices[i]);
			vertices[i].myColor = aColor;
		}

		SC_MutexLock lock(myTriangleMutex);
		myTriangleVertices3D.AddN(vertices, 36);
	}

	void SG_DebugPrimitives::RenderPrimitives(SG_View* aView)
	{
		RenderLines(aView);
		RenderTriangles(aView);
	}

	SG_DebugPrimitives::SG_DebugPrimitives()
	{
		SR_GraphicsDevice* device = SR_GraphicsDevice::GetDevice();
		SR_ShaderCompiler* shaderCompiler = device->GetShaderCompiler();

		SR_BufferDesc vBufferDesc;
		vBufferDesc.mySize = sizeof(SG_PrimitiveVertex);
		vBufferDesc.myStructSize = sizeof(SG_PrimitiveVertex);
		vBufferDesc.myBindFlag = SR_BindFlag_VertexBuffer;
		myTriangleVertexBuffer = device->CreateBuffer(vBufferDesc);
		myLineVertexBuffer = device->CreateBuffer(vBufferDesc);

		SR_ShaderStateDesc shaderStateDesc;
		shaderStateDesc.myMainPSO.myTopology = SR_Topology_LineList;
		shaderStateDesc.myMainPSO.myDepthStateDesc.myDepthComparisonFunc = SR_ComparisonFunc_GreaterEqual;
		shaderStateDesc.myMainPSO.myDepthStateDesc.myWriteDepth = false;
		shaderStateDesc.myMainPSO.myRasterizerStateDesc.myFaceCull = SR_RasterizerFaceCull_None;
		shaderStateDesc.myMainPSO.myRenderTargetFormats.myNumColorFormats = 1;
		shaderStateDesc.myMainPSO.myRenderTargetFormats.myColorFormats[0] = SR_Format_RGBA8_Unorm;
		shaderStateDesc.myMainPSO.myBlendStateDesc.myNumRenderTargets = 1;
		shaderStateDesc.myMainPSO.myBlendStateDesc.myRenderTagetBlendDescs[0].mySrcBlend = SR_BlendMode_One;
		shaderStateDesc.myMainPSO.myBlendStateDesc.myRenderTagetBlendDescs[0].myDstBlend = SR_BlendMode_Zero;
		shaderStateDesc.myMainPSO.myBlendStateDesc.myRenderTagetBlendDescs[0].mySrcBlendAlpha = SR_BlendMode_One;
		shaderStateDesc.myMainPSO.myBlendStateDesc.myRenderTagetBlendDescs[0].myDstBlendAlpha = SR_BlendMode_Zero;
		shaderStateDesc.myShaderByteCodes[SR_ShaderType_Vertex] = shaderCompiler->CompileShaderFromFile("ShiftEngine/Shaders/SGraphics/Debug/Primitive_SimpleLine3DVS.ssf", SR_ShaderType_Vertex);
		shaderStateDesc.myShaderByteCodes[SR_ShaderType_Pixel] = shaderCompiler->CompileShaderFromFile("ShiftEngine/Shaders/SGraphics/Debug/Primitive_SimpleLine3DPS.ssf", SR_ShaderType_Pixel);
		myLineShader = device->CreateShaderState(shaderStateDesc);

		shaderStateDesc.myMainPSO.myTopology = SR_Topology_TriangleList;
		myTriangleShader = device->CreateShaderState(shaderStateDesc);
	}

	SG_DebugPrimitives::~SG_DebugPrimitives()
	{
	}

	void SG_DebugPrimitives::RenderLines(SG_View* aView)
	{
		SR_GraphicsContext* ctx = SR_GraphicsContext::GetCurrent();
		SG_CommonRenderStates* crs = SG_CommonRenderStates::GetInstance();
		uint lineVertexCount = 0;
		{
			SC_MutexLock lock(myLinesMutex);
			if (myLineVertices3D.Empty())
				return;

			lineVertexCount = myLineVertices3D.Count();
			ctx->UpdateBufferData(myLineVertices3D.GetByteSize(), myLineVertices3D.GetBuffer(), myLineVertexBuffer);
		}

		ctx->SetTopology(SR_Topology_LineList);
		ctx->SetRasterizerState(crs->myTwoSidedRasterizerState);
		ctx->SetDepthState(crs->myDepthGreaterEqualReadState, 0);
		ctx->SetBlendState(crs->myNoBlendState, SC_Vector4f(0.0f));

		ctx->SetVertexBuffer(0, myLineVertexBuffer);

		struct ViewConstants
		{
			SC_Matrix44 myViewProjection;
		} viewConstants;

		const SG_Camera& camera = aView->GetRenderData().myMainCamera;
		SC_Matrix44 view = camera.GetView();
		SC_Matrix44 projection = camera.GetProjection();

		viewConstants.myViewProjection = view * projection;
		ctx->BindConstantBuffer(&viewConstants, sizeof(viewConstants), 0);

		ctx->SetShaderState(myLineShader);
		ctx->Draw(lineVertexCount, 0);
	}

	void SG_DebugPrimitives::RenderTriangles(SG_View* aView)
	{
		SR_GraphicsContext* ctx = SR_GraphicsContext::GetCurrent();
		SG_CommonRenderStates* crs = SG_CommonRenderStates::GetInstance();
		uint triangleVertexCount = 0;
		{
			SC_MutexLock lock(myTriangleMutex);
			if (myTriangleVertices3D.Empty())
				return;

			triangleVertexCount = myTriangleVertices3D.Count();
			ctx->UpdateBufferData(myTriangleVertices3D.GetByteSize(), myTriangleVertices3D.GetBuffer(), myTriangleVertexBuffer);
		}

		ctx->SetTopology(SR_Topology_TriangleList);
		ctx->SetRasterizerState(crs->myTwoSidedRasterizerState);
		ctx->SetDepthState(crs->myDepthGreaterEqualReadState, 0);
		ctx->SetBlendState(crs->myNoBlendState, SC_Vector4f(0.0f));

		ctx->SetVertexBuffer(0, myTriangleVertexBuffer);

		struct ViewConstants
		{
			SC_Matrix44 myViewProjection;
		} viewConstants;

		const SG_Camera& camera = aView->GetRenderData().myMainCamera;
		SC_Matrix44 view = camera.GetView();
		SC_Matrix44 projection = camera.GetProjection();

		viewConstants.myViewProjection = view * projection;
		ctx->BindConstantBuffer(&viewConstants, sizeof(viewConstants), 0);

		ctx->SetShaderState(myTriangleShader);
		ctx->Draw(triangleVertexCount, 0);
	}
}
#endif