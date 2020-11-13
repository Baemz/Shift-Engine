#include "SRender_Precompiled.h"
#include "SR_GBuffer.h"

namespace Shift
{
	SR_GBuffer::SR_GBuffer()
	{
	}

	SR_GBuffer::~SR_GBuffer()
	{
	}

	void SR_GBuffer::Update()
	{
		SR_GraphicsDevice* device = SR_GraphicsDevice::GetDevice();
		const SC_Vector2f& framebufferRes = device->GetResolution();

		myColors.myTextureBufferFormat = SR_Format_RGBA8_Unorm;
		myColors.myTextureFormat = SR_Format_RGBA8_Unorm;
		myColors.myCreateRenderTarget = true;
		myColors.myCreateTexture = true;
		myColors.Update(framebufferRes, "GBuffer/Color");

		myNormals.myTextureBufferFormat = SR_Format_RG11B10_Float;
		myNormals.myTextureFormat = SR_Format_RG11B10_Float;
		myNormals.myCreateRenderTarget = true;
		myNormals.myCreateTexture = true;
		myNormals.Update(framebufferRes, "GBuffer/Normal");

		myMaterials.myTextureBufferFormat = SR_Format_RGBA8_Unorm;
		myMaterials.myTextureFormat = SR_Format_RGBA8_Unorm;
		myMaterials.myCreateRenderTarget = true;
		myMaterials.myCreateTexture = true;
		myMaterials.Update(framebufferRes, "GBuffer/Material");

		myEmissive.myTextureBufferFormat = SR_Format_RGBA8_Unorm;
		myEmissive.myTextureFormat = SR_Format_RGBA8_Unorm;
		myEmissive.myCreateRenderTarget = true;
		myEmissive.myCreateTexture = true;
		myEmissive.Update(framebufferRes, "GBuffer/Optional_Emissive");

		myMotionVectors.myTextureBufferFormat = SR_Format_RGBA8_Unorm;
		myMotionVectors.myTextureFormat = SR_Format_RGBA8_Unorm;
		myMotionVectors.myCreateRenderTarget = true;
		myMotionVectors.myCreateTexture = true;
		myMotionVectors.Update(framebufferRes, "GBuffer/Optional_MotionVectors");
		
	}
}