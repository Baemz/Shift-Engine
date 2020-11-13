#include "SRender_Precompiled.h"
#include "SR_TextureResource.h"

namespace Shift
{
	SR_TextureResource::SR_TextureResource()
		: myCreateRenderTarget(false)
		, myCreateTexture(false)
		, myCreateRWTexture(false)
		, myTextureBufferFormat(SR_Format_Unknown)
		, myTextureFormat(SR_Format_Unknown)
	{
	}
	SR_TextureResource::~SR_TextureResource()
	{
		Reset();
	}
	void SR_TextureResource::Reset()
	{
		myTexture.Reset();
		myTextureRW.Reset();
		myRenderTarget.Reset();
		myTextureBuffer.Reset();
	}

	bool SR_TextureResource::Update(const SC_Vector2f& aSize, const char* aName)
	{
		float x = (float)aSize.x;
		float y = (float)aSize.y;

		bool mismatchingSettings = !myTextureBuffer ||
			(myCreateTexture && !myTexture) ||
			(myCreateRWTexture && !myTextureRW) ||
			(myCreateRenderTarget && !myRenderTarget) ||
			(myTextureBuffer->GetProperties().myFormat != myTextureBufferFormat) ||
			(myTextureBuffer->GetProperties().myWidth != x) ||
			(myTextureBuffer->GetProperties().myHeight != y) ||
			(myTexture->GetProperties().myFormat != myTextureFormat) ||
			(myTextureRW->GetProperties().myFormat != myTextureFormat);

		if (mismatchingSettings)
		{
			SR_TextureBufferDesc tBufDesc;

			SR_Format bufferFormat = myTextureBufferFormat;
			if (bufferFormat == SR_Format_Unknown && myTextureFormat != SR_Format_Unknown)
				bufferFormat = myTextureFormat;

			bool isDepthRT = IsDepthFormat(bufferFormat);
			if (isDepthRT)
				tBufDesc.myFormat = GetTypelessFormat(bufferFormat);
			else
				tBufDesc.myFormat = bufferFormat;

			tBufDesc.myDimension = SR_Dimension_Texture2D;
			tBufDesc.myWidth = aSize.x;
			tBufDesc.myHeight = aSize.y;
			tBufDesc.myArraySize = 1;
			tBufDesc.myMips = 1;

			uint flags = 0;
			if (myCreateRenderTarget)
				flags |= (isDepthRT) ? SR_ResourceFlag_AllowDepthStencil : SR_ResourceFlag_AllowRenderTarget;
			if (myCreateRWTexture)
				flags |= SR_ResourceFlag_AllowWrites;

			tBufDesc.myFlags = flags;

			SR_GraphicsDevice* device = SR_GraphicsDevice::GetDevice();

			myTextureBuffer = device->CreateTextureBuffer(tBufDesc, aName);
			if (!myTextureBuffer)
				return false;

			if (myCreateTexture)
			{
				SR_TextureDesc texDesc;
				texDesc.myFormat = myTextureFormat;
				texDesc.myMipLevels = 1;
				myTexture = device->CreateTexture(texDesc, myTextureBuffer);
				if (!myTexture)
					return false;
			}

			if (myCreateRWTexture)
			{
				SR_TextureDesc texDesc;
				texDesc.myFormat = myTextureFormat;
				texDesc.myMipLevels = 1;
				myTextureRW = device->CreateRWTexture(texDesc, myTextureBuffer);
				if (!myTextureRW)
					return false;
			}

			if (myCreateRenderTarget)
			{
				SR_RenderTargetDesc rtDesc;
				rtDesc.myFormat = bufferFormat;

				if (IsDepthFormat(bufferFormat))
					myRenderTarget = device->CreateDepthStencil(rtDesc, myTextureBuffer);
				else
					myRenderTarget = device->CreateRenderTarget(rtDesc, myTextureBuffer);

				if (!myRenderTarget)
					return false;
			}
		}

		return true;
	}
}