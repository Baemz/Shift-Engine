#pragma once

//////////////////////////////////////////////////////////////////////////////////////
//																					//
//	CTexture acts as a wrapper around a certain SRV for an actual texture buffer.	//
//																					//
//////////////////////////////////////////////////////////////////////////////////////

#include "SR_GraphicsResource.h"
#include "SR_Buffer.h"
#include "SR_TextureBuffer.h"

namespace Shift
{
	struct _STextureDesc // New Format
	{
		SR_Format myFormat;

		uint16 myArraySize;
		uint16 myFirstArrayIndex;

		uint8 myMostDetailedMip;
		uint8 myMips;

		bool myIsWritable;

		_STextureDesc()
			: myMostDetailedMip(0)
			, myMips(0)
			, myFirstArrayIndex(0)
			, myArraySize(0)
			, myFormat(SR_Format_Unknown)
			, myIsWritable(false)
		{}
		_STextureDesc(const SR_Format& aFormat)
			: myMostDetailedMip(0)
			, myMips(0)
			, myFirstArrayIndex(0)
			, myArraySize(0)
			, myFormat(aFormat)
			, myIsWritable(false)
		{}
	};

	struct SR_TextureDesc : public SR_ImageStorageRange
	{
		SR_TextureDesc()
			: myFormat(SR_Format_Unknown)
			, myFlags(0)
			, myIsCube(false)
		{}

		SR_Format myFormat;
		uint myFlags;
		bool myIsCube : 1;
	};

	class SR_TextureBuffer;
	class SR_Texture : public SR_Resource//, public CNoncopyable
	{
		friend class SR_GraphicsDevice_DX12;

		friend class SR_SwapChain_DX12;
	public:
		SR_Texture();
		virtual ~SR_Texture();

		const SR_TextureBuffer* GetTextureBuffer() const	{ return myTextureBuffer; }
		SC_Ref<SR_TextureBuffer> GetTextureBuffer()			{ return myTextureBuffer; }

		const SR_TextureDesc& GetProperties() const		{ return myDescription; }

		SR_TextureDesc myDescription;
	protected:
		SC_Ref<SR_TextureBuffer> myTextureBuffer;
	};

}