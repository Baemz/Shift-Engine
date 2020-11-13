#pragma once

namespace Shift
{
	class SR_Texture;
	class SR_TextureBuffer;
	struct SR_TextureDesc;
	struct SR_TextureBufferDesc;

	class SR_TempResourcePool
	{
	public:
		SR_TempResourcePool();
		virtual ~SR_TempResourcePool();

		virtual SC_Ref<SR_TextureBuffer> CreateTextureBuffer(const SR_TextureBufferDesc& aDesc, const char* aID = nullptr) = 0;
		virtual SC_Ref<SR_Texture> CreateTexture(const SR_TextureBuffer* aTextureBuffer, const SR_TextureDesc& aDesc) = 0;

	protected:

		// Properties
	};

}