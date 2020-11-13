#pragma once
#include "SR_TrackedResource.h"
#include "SR_RenderEnums.h"

namespace Shift
{
	struct SR_ImageAccessRange
	{
		uint myMipLevel;
		uint myArrayIndex;

		SR_ImageAccessRange() : myMipLevel(0), myArrayIndex(0) {}
	};

	struct SR_ImageStorageRange
	{
		uint myMostDetailedMip;
		uint myMipLevels;
		uint myFirstArrayIndex;
		uint myArraySize;

		SR_ImageStorageRange() : myMostDetailedMip(), myMipLevels(0), myFirstArrayIndex(0), myArraySize(0) {}
	};

	struct _STextureBufferDesc // New Format
	{
		SC_Vector3i mySize;
		uint myArraySize;
		uint8 myMips;

		// Access CPU/GPU

		SR_Dimension myDimension;
		SR_Format myFormat;

		bool myIsTexture;
		bool myIsRenderTarget;
		bool _myUnusedBool;
	};
	struct SR_TextureBufferDesc
	{

		SR_TextureBufferDesc()
			: myWidth(0)
			, myHeight(0)
			, myDepth(1)
			, myArraySize(1)
			, myMips(0)
			, myFormat(SR_Format_Unknown)
			, myFlags(0)
			, myDimension(SR_Dimension_Unknown)
			, myIsCube(false)
		{}

		float myWidth;
		float myHeight;
		float myDepth;
		uint myFlags;
		uint16 myArraySize;
		uint16 myMips;
		SR_Format myFormat;
		SR_Dimension myDimension;

		bool myIsCube : 1;
	};


	class SR_TextureBuffer : public SR_TrackedResource
	{
		friend class SR_GraphicsDevice_DX12;
		friend class SR_SwapChain_DX12;
		friend class SR_GraphicsDevice_Vk;
		friend class SR_SwapChain_Vk;
	public:
		SR_TextureBuffer();
		virtual ~SR_TextureBuffer();

		const SR_TextureBufferDesc& GetProperties() const;

	protected:
		SR_TextureBufferDesc myProperties;

#if !IS_FINAL_BUILD
		std::string myName;
#endif
	};

}