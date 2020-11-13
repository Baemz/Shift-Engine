#include "SGraphics_Precompiled.h"
#include "SG_PostEffects.h"
#include "SG_View.h"
#include "SR_ShaderCompiler.h"

namespace Shift
{
	SG_PostEffects::Shared* SG_PostEffects::ourSharedCache = nullptr;
	static constexpr float locPixelMargin = 0.5031f;

	uint locDispatchSize(uint aSize, uint aNumElements)
	{
		uint dispatchSize = aNumElements / aSize;
		dispatchSize += aNumElements % aSize > 0 ? 1 : 0;
		return dispatchSize;
	}

	uint ReductionSize = 16;

	SG_PostEffects::SG_PostEffects()
	{
	}

	SG_PostEffects::~SG_PostEffects()
	{
	}

	void SG_PostEffects::Init(SC_Vector2f aResolution)
	{
		if (ourSharedCache == nullptr)
		{
			ourSharedCache = new SG_PostEffects::Shared();
			ourSharedCache->Load();
		}

		SC_Vector2ui fullRes = SC_Vector2ui((uint)aResolution.x, (uint)aResolution.y);
		SC_Vector2ui halfRes = SC_Vector2ui((fullRes.x + 1) / 2, (fullRes.y + 1) / 2);

		uint width = fullRes.x;
		uint height = fullRes.y;

		while (width > 1 || height > 1)
		{
			width = locDispatchSize(ReductionSize, width);
			height = locDispatchSize(ReductionSize, height);

			SR_TextureResource& resource = myLuminanceReductionTargets.Add();
			resource.myCreateTexture = true;
			resource.myCreateRWTexture = true;
			resource.myTextureFormat = SR_Format_R32_Float;
			resource.Update(SC_Vector2f((float)width, (float)height), "Luminance Reduction");
		}

		uint bloomWidth = fullRes.x;
		uint bloomHeight = fullRes.y;
		while (bloomWidth > 4 || bloomHeight > 4)
		{
			bloomWidth = (bloomWidth + 1) / 2;
			bloomHeight = (bloomHeight + 1) / 2;

			SR_TextureResource& resource = myBloomTargets.Add();
			resource.myCreateTexture = true;
			resource.myCreateRWTexture = true;
			resource.myTextureFormat = SR_Format_RG11B10_Float;
			resource.Update(SC_Vector2f((float)bloomWidth, (float)bloomHeight), "Bloom Target");
		}
	}
	void SG_PostEffects::Render(SG_View* aView)
	{
		SG_FrameResources& frameResources = aView->myFrameResources;

		Downsample(myBloomTargets[0].myTextureRW, frameResources.myFullscreenHDR.myTexture);
		Downsample(myBloomTargets[1].myTextureRW, myBloomTargets[0].myTexture); // Quarter

		Bloom(aView);

		CalculateLuminance(aView, frameResources.myFullscreenHDR.myTexture);
		Tonemap(aView);
	}

	void SG_PostEffects::CalculateLuminance(SG_View* aView, SR_Texture* aInputTexture)
	{
		SR_PROFILER_FUNCTION_TAGGED("Luminance");
		SR_GraphicsContext* ctx = SR_GraphicsContext::GetCurrent();

		ctx->BindTextureRW(myLuminanceReductionTargets[0].myTextureRW, 0);
		ctx->BindTexture(aInputTexture, 0);
		//ctx->BindComputePSO(*ourSharedCache->myInitialLumReductionShader);
		ctx->SetShaderState(ourSharedCache->myInitialLumReductionShader);

		uint dispatchX = (uint)myLuminanceReductionTargets[0].myTextureBuffer->GetProperties().myWidth;
		uint dispatchY = (uint)myLuminanceReductionTargets[0].myTextureBuffer->GetProperties().myHeight;
		ctx->Dispatch(dispatchX, dispatchY, 1);

		for (uint i = 1, count = myLuminanceReductionTargets.Count(); i < count; ++i)
		{
			dispatchX = (uint)myLuminanceReductionTargets[i].myTextureBuffer->GetProperties().myWidth;
			dispatchY = (uint)myLuminanceReductionTargets[i].myTextureBuffer->GetProperties().myHeight;
			if (i == myLuminanceReductionTargets.Count() - 1)
			{
				struct Constants
				{
					float TimeDelta;
					bool EnableAdaptation;
				} constants;
				constants.TimeDelta = SC_Timer::GetGlobalDeltaTime();
				constants.EnableAdaptation = true;

				ctx->BindConstantBuffer(&constants, sizeof(constants), 0);
				ctx->SetShaderState(ourSharedCache->myFinalLumReductionShader);
			}
			else
				ctx->SetShaderState(ourSharedCache->myLumReductionShader);

			ctx->BindTextureRW(myLuminanceReductionTargets[i].myTextureRW, 0);
			ctx->BindTexture(myLuminanceReductionTargets[i - 1].myTexture, 0);

			ctx->Dispatch(dispatchX, dispatchY, 1);

			if (i == myLuminanceReductionTargets.Count() - 1)
				aView->myFrameResources.myAvgLuminance = myLuminanceReductionTargets[i];
		}
	}

	void SG_PostEffects::Bloom(SG_View* aView)
	{
		SR_PROFILER_FUNCTION_TAGGED("Bloom");
		SR_TextureResource tempTexture;
		RecursiveBloomDownsample(tempTexture, myBloomTargets[1].myTexture, 2);
		BloomUpsample(myBloomTargets[0].myTextureRW, tempTexture.myTexture);

		aView->myFrameResources.myBloomTexture = myBloomTargets[0].myTexture;
	}

	void SG_PostEffects::Tonemap(SG_View* aView)
	{
		SR_PROFILER_FUNCTION_TAGGED("Tonemap");

		SR_GraphicsContext* ctx = SR_GraphicsContext::GetCurrent();
		SG_FrameResources& frameResources = aView->myFrameResources;

		const SC_Float2 fullResolution = SR_GraphicsDevice::GetDevice()->GetResolution();
		struct TonemappingConstants
		{
			float ExposureBias;
			float BloomIntensity;
		} tonemappingConstants;
		tonemappingConstants.ExposureBias = 1.0f;
		tonemappingConstants.BloomIntensity = 0.75f;

		ctx->Transition(SR_ResourceState_UnorderedAccess, frameResources.myFullscreen.myTextureBuffer);

		ctx->BindConstantBuffer(&tonemappingConstants, sizeof(tonemappingConstants), 0);

		ctx->BindTexture(frameResources.myFullscreenHDR.myTexture, 0);
		ctx->BindTexture(myBloomTargets[0].myTexture, 1);
		ctx->BindTexture(frameResources.myAvgLuminance.myTexture, 2);
		ctx->BindTextureRW(frameResources.myFullscreen.myTextureRW, 0);
		ctx->Dispatch(ourSharedCache->myTonemapShader, uint(fullResolution.x), uint(fullResolution.y));
		frameResources.myLastTextureWritten = frameResources.myFullscreen;

		ctx->Transition(SR_ResourceState(SR_ResourceState_NonPixelSRV | SR_ResourceState_PixelSRV), frameResources.myFullscreen.myTextureBuffer);
	}

	void SG_PostEffects::Tonemap(SR_Texture* /*aFramebufferTexture*/, SR_Texture* /*aBloomTexture*/)
	{
	}

	void SG_PostEffects::RecursiveBloomDownsample(SR_TextureResource& aOutTexture, SR_Texture* aSourceMip, const uint aCurrentMip)
	{
		Downsample(myBloomTargets[aCurrentMip].myTextureRW, aSourceMip);

		const SR_TextureBufferDesc& bufProps = myBloomTargets[aCurrentMip].myTextureBuffer->GetProperties();
		if (bufProps.myWidth > 4 || bufProps.myHeight > 4)
		{
			SR_TextureResource tempTexture;
			RecursiveBloomDownsample(tempTexture, myBloomTargets[aCurrentMip].myTexture, aCurrentMip + 1);
			BloomUpsample(myBloomTargets[aCurrentMip].myTextureRW, tempTexture.myTexture);
		}

		aOutTexture = myBloomTargets[aCurrentMip];
	}

	void SG_PostEffects::BloomUpsample(SR_Texture* aOutTexture, SR_Texture* aInTexture)
	{
		assert(aOutTexture->GetTextureBuffer() != aInTexture->GetTextureBuffer());

		SR_PROFILER_FUNCTION_TAGGED("Bloom Upsample");
		const SR_TextureBufferDesc& sourceProps = aInTexture->GetTextureBuffer()->GetProperties();
		const SR_TextureBufferDesc& targetProps = aOutTexture->GetTextureBuffer()->GetProperties();
		SC_Vector3f sourceSize = SC_Vector3f(sourceProps.myWidth, sourceProps.myHeight, 1.f);
		SC_Vector3f dstSize = SC_Vector3f(targetProps.myWidth, targetProps.myHeight, 1.f);

		SC_ALIGN(16) struct Constants
		{
			SC_Vector2f myMinUV;
			SC_Vector2f myMaxUV;
			SC_Vector2f myTexelSizeAndUVScale;
			SC_Vector2f myScaleFilter;
			SC_Vector2i myDestOffset;
			float myStrength;
		} constants;

		const SC_Vector2f size((sourceSize.x != 0) ? (1.f / (float)sourceSize.x) : 1.f, (sourceSize.y != 0) ? (1.f / (float)sourceSize.y) : 1.f);
		const SC_Vector4f srcOrigUV((0.f), (0.f), (sourceSize.x * size.x), (sourceSize.y * size.y));
		const float srcOrigUVWidth = srcOrigUV.z - srcOrigUV.x;
		const float srcOrigUVHeight = srcOrigUV.w - srcOrigUV.y;

		constants.myMinUV = SC_Vector2f(((0.f + locPixelMargin) * size.x), ((0.f + locPixelMargin) * size.y));
		constants.myMaxUV = SC_Vector2f(((sourceSize.x - locPixelMargin) * size.x), ((sourceSize.y - locPixelMargin) * size.y));
		constants.myTexelSizeAndUVScale = SC_Vector2f(srcOrigUVWidth / dstSize.x, srcOrigUVHeight / dstSize.y);
		constants.myScaleFilter = SC_Vector2f(1.75f, 1.25f); // Bloom scale filter setting
		constants.myDestOffset = SC_Vector2i(0, 0);
		constants.myStrength = 0.63f; // bloom strength setting

		SR_GraphicsContext* ctx = SR_GraphicsContext::GetCurrent();

		SC_Pair<SR_ResourceState, SR_TrackedResource*> pairs[2];
		pairs[0] = SC_Pair(SR_ResourceState_NonPixelSRV, aInTexture->GetTextureBuffer());
		pairs[1] = SC_Pair(SR_ResourceState_UnorderedAccess, aOutTexture->GetTextureBuffer());

		ctx->Transition(pairs, 2);
		ctx->BindConstantBuffer(&constants, sizeof(constants), 0);
		ctx->BindTexture(aInTexture, 0);
		ctx->BindTextureRW(aOutTexture, 0);

		ctx->Dispatch(ourSharedCache->myBloomUpsampleShader, dstSize);
	}

	void SG_PostEffects::Downsample(SR_Texture* aOutTexture, SR_Texture* aSourceTexture)
	{
		assert(aOutTexture->GetTextureBuffer() != aSourceTexture->GetTextureBuffer());

		SR_PROFILER_FUNCTION_TAGGED("Downsample");
		const SR_TextureBufferDesc& inProps = aSourceTexture->GetTextureBuffer()->GetProperties();
		const SR_TextureBufferDesc& outProps = aOutTexture->GetTextureBuffer()->GetProperties();
		SC_Vector3f sourceSize = SC_Vector3f(inProps.myWidth, inProps.myHeight, 1.f);
		SC_Vector3ui dstSize = SC_Vector3f(outProps.myWidth, outProps.myHeight, 1.f);

		SC_ALIGN(16) struct Constants
		{
			SC_Vector2f myMinUV;
			SC_Vector2f myMaxUV;
			SC_Vector2f myTexelSizeAndUVScale;
			SC_Vector2f myUVOffset;
			SC_Vector2f myDiv2TexelSizeAndUVScale;
		} constants;

		const SC_Vector2f size((sourceSize.x != 0) ? (1.f / (float)sourceSize.x) : 1.f, (sourceSize.y != 0) ? (1.f / (float)sourceSize.y) : 1.f);

		constants.myMinUV = SC_Vector2f(((0.f + locPixelMargin) * size.x), ((0.f + locPixelMargin) * size.y));
		constants.myMaxUV = SC_Vector2f(((sourceSize.x - locPixelMargin) * size.x), ((sourceSize.y - locPixelMargin) * size.y));
		constants.myTexelSizeAndUVScale = SC_Vector2f(1.f / sourceSize.x, 1.f / sourceSize.y);
		constants.myUVOffset = SC_Vector2f(0.f, 0.f);
		constants.myDiv2TexelSizeAndUVScale = SC_Vector2f(2.f, 2.f) / constants.myTexelSizeAndUVScale;
		
		SR_GraphicsContext* ctx = SR_GraphicsContext::GetCurrent();

		SC_Pair<SR_ResourceState, SR_TrackedResource*> pairs[2];
		pairs[0] = SC_Pair(SR_ResourceState_NonPixelSRV, aSourceTexture->GetTextureBuffer());
		pairs[1] = SC_Pair(SR_ResourceState_UnorderedAccess, aOutTexture->GetTextureBuffer());

		ctx->Transition(pairs, 2);

		ctx->BindConstantBuffer(&constants, sizeof(constants), 0);
		ctx->BindTexture(aSourceTexture, 0);
		ctx->BindTextureRW(aOutTexture, 0);

		ctx->Dispatch(ourSharedCache->myDownsampleShader, dstSize);
	}

	SG_PostEffects::Shared::Shared()
	{
	}

	SG_PostEffects::Shared::~Shared()
	{
	}

	void SG_PostEffects::Shared::Load()
	{
		SR_GraphicsDevice* device = SR_GraphicsDevice::GetDevice();
		SR_ShaderCompiler* shaderCompiler = device->GetShaderCompiler();

		{
			SR_ShaderStateDesc desc;
			desc.myShaderByteCodes[SR_ShaderType_Compute] = shaderCompiler->CompileShaderFromFile("ShiftEngine/Shaders/SGraphics/PostFx/Downsample.ssf", SR_ShaderType_Compute);
			myDownsampleShader = device->CreateShaderState(desc);
		}
		{
			SR_ShaderStateDesc desc;
			desc.myShaderByteCodes[SR_ShaderType_Compute] = shaderCompiler->CompileShaderFromFile("ShiftEngine/Shaders/SGraphics/PostFx/Bloom_Upsample.ssf", SR_ShaderType_Compute);
			myBloomUpsampleShader = device->CreateShaderState(desc);
		}
		{
			SR_ShaderStateDesc desc;
			desc.myShaderByteCodes[SR_ShaderType_Compute] = shaderCompiler->CompileShaderFromFile("ShiftEngine/Shaders/SGraphics/PostFx/Exposure/LuminanceReduction_Initial.ssf", SR_ShaderType_Compute);
			myInitialLumReductionShader = device->CreateShaderState(desc);
		}
		{
			SR_ShaderStateDesc desc;
			desc.myShaderByteCodes[SR_ShaderType_Compute] = shaderCompiler->CompileShaderFromFile("ShiftEngine/Shaders/SGraphics/PostFx/Exposure/LuminanceReduction.ssf", SR_ShaderType_Compute);
			myLumReductionShader = device->CreateShaderState(desc);
		}
		{
			SR_ShaderStateDesc desc;
			desc.myShaderByteCodes[SR_ShaderType_Compute] = shaderCompiler->CompileShaderFromFile("ShiftEngine/Shaders/SGraphics/PostFx/Exposure/LuminanceReduction_Final.ssf", SR_ShaderType_Compute);
			myFinalLumReductionShader = device->CreateShaderState(desc);
		}
		{
			SR_ShaderStateDesc sDesc;
			sDesc.myShaderByteCodes[SR_ShaderType_Compute] = shaderCompiler->CompileShaderFromFile("ShiftEngine/Shaders/SGraphics/PostFX/Tonemap/Tonemap.ssf", SR_ShaderType_Compute);
			myTonemapShader = device->CreateShaderState(sDesc);
		}
		//{
		//	myBloomLuminancePSO = new CComputePSO();
		//
		//	SComputePSODesc csDesc;
		//	csDesc.cs = device->CreateComputeShader("Shaders/Bloom_Luminance.hlsl", "main");
		//	device->CreateComputePSO(csDesc, myBloomLuminancePSO);
		//}
		//{
		//	myBloomBlurPSO = new CComputePSO();
		//
		//	SComputePSODesc csDesc;
		//	csDesc.cs = device->CreateComputeShader("Shaders/Bloom_Blur.hlsl", "main");
		//	device->CreateComputePSO(csDesc, myBloomBlurPSO);
		//}
		//{
		//	myCopyPSO = new CComputePSO();
		//
		//	SComputePSODesc csDesc;
		//	csDesc.cs = device->CreateComputeShader("Shaders/Copy.hlsl", "main");
		//	device->CreateComputePSO(csDesc, myCopyPSO);
		//}
		//{
		//	myAddPSO = new CComputePSO();
		//
		//	SComputePSODesc csDesc;
		//	csDesc.cs = device->CreateComputeShader("Shaders/Bloom_Add.hlsl", "main");
		//	device->CreateComputePSO(csDesc, myAddPSO);
		//}
	}
}