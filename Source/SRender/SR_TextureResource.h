#pragma once
#include "SR_Texture.h"
#include "SR_RenderTarget.h"

namespace Shift
{
	class SR_TextureResource
	{
	public:
		SR_TextureResource();
		~SR_TextureResource();

		void Reset();
		bool Update(const SC_Vector2f& aSize, const char* aName);

		SC_Ref<SR_TextureBuffer> myTextureBuffer;
		SC_Ref<SR_Texture> myTexture;
		SC_Ref<SR_Texture> myTextureRW;
		SC_Ref<SR_RenderTarget> myRenderTarget;

		SR_Format myTextureBufferFormat;
		SR_Format myTextureFormat;

		bool myCreateTexture : 1;
		bool myCreateRWTexture : 1;
		bool myCreateRenderTarget : 1;
		bool myUnusedBool : 1;

	private:

	};
}