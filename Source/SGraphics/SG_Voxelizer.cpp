#include "SGraphics_Precompiled.h"
#include "SG_Voxelizer.h"

#include "SG_CommonRenderStates.h"
#include "SG_View.h"

namespace Shift
{
	SG_Voxelizer::SG_Voxelizer()
	{
		SR_GraphicsDevice* device = SR_GraphicsDevice::GetDevice();
		SR_ShaderCompiler* shaderCompiler = device->GetShaderCompiler();
		SR_TextureBufferDesc textureBufferDesc;
		textureBufferDesc.myWidth = ourVoxelSceneResolution;
		textureBufferDesc.myHeight = ourVoxelSceneResolution;
		textureBufferDesc.myDepth = ourVoxelSceneResolution;
		textureBufferDesc.myDimension = SR_Dimension_Texture3D;
		textureBufferDesc.myFormat = SR_Format_RGBA8_Unorm;
		textureBufferDesc.myFlags |= SR_ResourceFlag_AllowWrites;
		textureBufferDesc.myMips = 1;

		SC_Ref<SR_TextureBuffer> colorBuffer = device->CreateTextureBuffer(textureBufferDesc, "Voxel Color Texture3D");
		SC_Ref<SR_TextureBuffer> normalBuffer = device->CreateTextureBuffer(textureBufferDesc, "Voxel Normal Texture3D");

		textureBufferDesc.myMips = (uint16)log2(ourVoxelSceneResolution);
		textureBufferDesc.myFormat = SR_Format_RGB10A2_Unorm;
		SC_Ref<SR_TextureBuffer> litBuffer = device->CreateTextureBuffer(textureBufferDesc, "Voxel Lit Texture3D");

		SR_TextureDesc textureDesc;
		textureDesc.myFormat = SR_Format_RGBA8_Unorm;
		myVoxelColors = device->CreateTexture(textureDesc, colorBuffer);
		myVoxelNormals = device->CreateTexture(textureDesc, normalBuffer);

		textureDesc.myMipLevels = 1;
		myVoxelColorsRW = device->CreateRWTexture(textureDesc, colorBuffer);
		myVoxelNormalsRW = device->CreateRWTexture(textureDesc, normalBuffer);

		SR_TextureDesc textureLitDesc;
		textureLitDesc.myFormat = SR_Format_RGB10A2_Unorm;
		textureLitDesc.myMipLevels = textureBufferDesc.myMips;
		myVoxelLit = device->CreateTexture(textureLitDesc, litBuffer);
		textureLitDesc.myMipLevels = 1;
		myVoxelLitRW = device->CreateRWTexture(textureLitDesc, litBuffer);

		SR_ShaderStateDesc desc;
		desc.myShaderByteCodes[SR_ShaderType_Compute] = shaderCompiler->CompileShaderFromFile("ShiftEngine/Shaders/SGraphics/VXGI/RelightVoxels.ssf", SR_ShaderType_Compute);
		myRelightShader = device->CreateShaderState(desc);

		desc.myShaderByteCodes[SR_ShaderType_Compute] = shaderCompiler->CompileShaderFromFile("ShiftEngine/Shaders/SGraphics/VXGI/GenerateMips.ssf", SR_ShaderType_Compute);
		myGenerateMipsShader = device->CreateShaderState(desc);
	}

	SG_Voxelizer::~SG_Voxelizer()
	{
	}

	void SG_Voxelizer::Voxelize(SG_View* aView)
	{
		SR_GraphicsContext* ctx = SR_GraphicsContext::GetCurrent();
		SG_CommonRenderStates* crs = SG_CommonRenderStates::GetInstance();

		SR_Viewport viewport;
		viewport.topLeftX = 0;
		viewport.topLeftY = 0;
		viewport.width = float(ourVoxelSceneResolution);
		viewport.height = float(ourVoxelSceneResolution);
		viewport.minDepth = 0.f;
		viewport.maxDepth = float(ourVoxelSceneResolution);

		SR_ScissorRect scissor;
		scissor.left = 0;
		scissor.top = 0;
		scissor.right = long(ourVoxelSceneResolution);
		scissor.bottom = long(ourVoxelSceneResolution);

		ctx->SetViewport(viewport);
		ctx->SetScissorRect(scissor);

		ctx->SetRenderTargets(nullptr, 0, nullptr, 0);
		ctx->SetDepthState(crs->myDefaultDepthState, 0);

		struct VoxelSettings
		{
			SC_Vector3f VoxelSceneMinPos;
			float VoxelSize;
			SC_Vector3f VoxelSceneCenter;
			float VoxelSizeInversed;
			float VoxelSceneTexelSize;
			float VoxelSceneWorldSize;
			float VoxelWorldScale;
			uint VoxelSceneResolution;
		} settingsConstants;

		SC_Vector3f sceneMin = settingsConstants.VoxelSceneCenter - (ourVoxelSceneResolution * ourVoxelSize * 0.5);

		settingsConstants.VoxelSceneMinPos = sceneMin;
		settingsConstants.VoxelSize = ourVoxelSize;
		settingsConstants.VoxelSceneResolution = ourVoxelSceneResolution;
		settingsConstants.VoxelSceneTexelSize = 1.0f / ourVoxelSceneResolution;
		settingsConstants.VoxelSizeInversed = 1.0f / ourVoxelSize;
		settingsConstants.VoxelSceneWorldSize = ourVoxelSceneResolution * ourVoxelSize;
		settingsConstants.VoxelWorldScale = 1.0f / settingsConstants.VoxelSceneWorldSize;

		ctx->BindConstantBuffer(&settingsConstants, sizeof(settingsConstants), 0);

		ctx->BindTextureRW(myVoxelColorsRW, 0);
		ctx->BindTextureRW(myVoxelNormalsRW, 1);

		aView->GetRenderData().myQueues.myVoxelize.Render("Voxelize Meshes");
	}

	void SG_Voxelizer::RelightVoxelScene(SG_View* aView)
	{
		aView;
		SR_GraphicsContext* ctx = SR_GraphicsContext::GetCurrent();
		SR_PROFILER_FUNCTION_TAGGED("Relight Voxels");

		struct VoxelSettings
		{
			SC_Vector3f VoxelSceneMinPos;
			float VoxelSize;
			SC_Vector3f VoxelSceneCenter;
			float VoxelSizeInversed;
			float VoxelSceneTexelSize;
			float VoxelSceneWorldSize;
			float VoxelWorldScale;
			uint VoxelSceneResolution;
		} settingsConstants;

		SC_Vector3f sceneMax = settingsConstants.VoxelSceneCenter + (ourVoxelSceneResolution * ourVoxelSize * 0.5);
		SC_Vector3f sceneMin = settingsConstants.VoxelSceneCenter - (ourVoxelSceneResolution * ourVoxelSize * 0.5);

		settingsConstants.VoxelSceneMinPos = sceneMin;
		settingsConstants.VoxelSize = ourVoxelSize;
		settingsConstants.VoxelSceneResolution = ourVoxelSceneResolution;
		settingsConstants.VoxelSceneTexelSize = 1.0f / ourVoxelSceneResolution;
		settingsConstants.VoxelSizeInversed = 1.0f / ourVoxelSize;
		settingsConstants.VoxelSceneWorldSize = (sceneMax - sceneMin).Length();
		settingsConstants.VoxelWorldScale = 1.0f / settingsConstants.VoxelSceneWorldSize;

		ctx->BindConstantBuffer(&settingsConstants, sizeof(settingsConstants), 0);
		ctx->BindTextureRW(myVoxelColorsRW, 0);
		ctx->BindTextureRW(myVoxelNormalsRW, 1);
		ctx->BindTextureRW(myVoxelLitRW, 2);

		ctx->SetShaderState(myRelightShader);

		uint threadGroupsToDispatch = ourVoxelSceneResolution / 4;
		ctx->Dispatch(threadGroupsToDispatch, threadGroupsToDispatch, threadGroupsToDispatch);

		GenerateMipChain();
	}

	void SG_Voxelizer::SetVoxelConstants()
	{
		struct VoxelSettings
		{
			SC_Vector3f VoxelSceneMinPos;
			float VoxelSize;
			SC_Vector3f VoxelSceneCenter;
			float VoxelSizeInversed;
			float VoxelSceneTexelSize;
			float VoxelSceneWorldSize;
			float VoxelWorldScale;
			uint VoxelSceneResolution;
		} settingsConstants;

		SC_Vector3f sceneMax = settingsConstants.VoxelSceneCenter + (ourVoxelSceneResolution * ourVoxelSize * 0.5);
		SC_Vector3f sceneMin = settingsConstants.VoxelSceneCenter - (ourVoxelSceneResolution * ourVoxelSize * 0.5);

		settingsConstants.VoxelSceneMinPos = sceneMin;
		settingsConstants.VoxelSize = ourVoxelSize;
		settingsConstants.VoxelSceneResolution = ourVoxelSceneResolution;
		settingsConstants.VoxelSceneTexelSize = 1.0f / ourVoxelSceneResolution;
		settingsConstants.VoxelSizeInversed = 1.0f / ourVoxelSize;
		settingsConstants.VoxelSceneWorldSize = (sceneMax - sceneMin).Length();
		settingsConstants.VoxelWorldScale = 1.0f / settingsConstants.VoxelSceneWorldSize;

		SR_GraphicsContext* ctx = SR_GraphicsContext::GetCurrent();
		ctx->BindConstantBufferRef(&settingsConstants, sizeof(settingsConstants), SR_ConstantBufferRef::SR_ConstantBufferRef_VoxelSettings);
	}

	void SG_Voxelizer::GenerateMipChain()
	{
		SR_GraphicsDevice* device = SR_GraphicsDevice::GetDevice();
		SR_GraphicsContext* ctx = SR_GraphicsContext::GetCurrent();
		const SR_TextureBufferDesc& properties = myVoxelLit->GetTextureBuffer()->GetProperties();
		const uint mipCount = properties.myMips;

		SR_TextureDesc textureDesc;
		textureDesc.myFormat = properties.myFormat;
		textureDesc.myMipLevels = 1;

		SR_TextureDesc textureRWDesc;
		textureRWDesc.myFormat = properties.myFormat;
		textureRWDesc.myMipLevels = 1;

		ctx->Transition(SR_ResourceState_UnorderedAccess, myVoxelLit->GetTextureBuffer());

		for (uint topMip = 0; topMip < mipCount - 1; topMip++)
		{
			uint dstWidth = SC_Max((uint)properties.myWidth >> (topMip + 1), 1);
			uint dstHeight = SC_Max((uint)properties.myHeight >> (topMip + 1), 1);
			uint dstDepth = SC_Max((uint)properties.myDepth >> (topMip + 1), 1);

			textureDesc.myFirstArrayIndex = 0;
			textureDesc.myArraySize = dstDepth * 2;
			textureDesc.myMostDetailedMip = topMip;
			SC_Ref<SR_Texture> tex = device->CreateTexture(textureDesc, myVoxelLit->GetTextureBuffer());

			textureRWDesc.myFirstArrayIndex = 0;
			textureRWDesc.myArraySize = dstDepth;
			textureRWDesc.myMostDetailedMip = topMip + 1;
			SC_Ref<SR_Texture> texRW = device->CreateRWTexture(textureRWDesc, myVoxelLit->GetTextureBuffer());

			ctx->BindTexture(tex, 0);
			ctx->BindTextureRW(texRW, 0);
			ctx->Dispatch(myGenerateMipsShader, SC_Vector3ui(dstWidth, dstHeight, dstDepth));
			ctx->BarrierUAV(myVoxelLit->GetTextureBuffer());
		}

		ctx->Transition(SR_ResourceState(SR_ResourceState_NonPixelSRV), myVoxelLit->GetTextureBuffer());
	}
}