#pragma once
#include "SR_ResourceRefs.h"

namespace Shift
{
	using SR_LocalConstantsMask = uint8;
	using SR_LocalTexturesMask = uint16;
	using SR_LocalBuffersMask = uint16;
	using SR_LocalTexturesRWMask = uint8;
	using SR_LocalBuffersRWMask = uint8;
	using SR_GlobalConstantsMask = uint8;
	using SR_GlobalTexturesMask = uint32;
	using SR_GlobalBuffersMask = uint8;
	using SR_GlobalTexturesRWMask = uint8;
	using SR_GlobalBuffersRWMask = uint8;

	static_assert(SR_ConstantBufferRef_NumLocals <= sizeof(SR_LocalConstantsMask) * 8);
	static_assert(SR_TextureRef_NumLocals <= sizeof(SR_LocalTexturesMask) * 8);
	static_assert(SR_BufferRef_NumLocals <= sizeof(SR_LocalBuffersMask) * 8);
	static_assert(SR_TextureRWRef_NumLocals <= sizeof(SR_LocalTexturesRWMask) * 8);
	static_assert(SR_BufferRWRef_NumLocals <= sizeof(SR_LocalBuffersRWMask) * 8);
	static_assert(SR_ConstantBufferRef_NumGlobals <= sizeof(SR_GlobalConstantsMask) * 8);
	static_assert(SR_TextureRef_NumGlobals <= sizeof(SR_GlobalTexturesMask) * 8);
	static_assert(SR_BufferRef_NumGlobals <= sizeof(SR_GlobalBuffersMask) * 8);
	static_assert(SR_TextureRWRef_NumGlobals <= sizeof(SR_GlobalTexturesRWMask) * 8);
	static_assert(SR_BufferRWRef_NumGlobals <= sizeof(SR_GlobalBuffersRWMask) * 8);

	struct SR_ResourceBindings
	{
		SR_ResourceBindings() { memset(this, 0, sizeof(*this)); }

		SR_ResourceBindings& operator|=(const SR_ResourceBindings& anOther)
		{
			myLocalConstants |= anOther.myLocalConstants;
			myLocalTextures |= anOther.myLocalTextures;
			myLocalBuffers |= anOther.myLocalBuffers;
			myLocalTextureRWs |= anOther.myLocalTextureRWs;
			myLocalBufferRWs |= anOther.myLocalBufferRWs;
			myGlobalConstants |= anOther.myGlobalConstants;
			myGlobalTextures |= anOther.myGlobalTextures;
			myGlobalBuffers |= anOther.myGlobalBuffers;
			myGlobalTextureRWs |= anOther.myGlobalTextureRWs;
			myGlobalBufferRWs |= anOther.myGlobalBufferRWs;
			return *this;
		}

		void Clear()
		{
			memset(this, 0, sizeof(*this));
		}

		bool operator==(const SR_ResourceBindings& anOther) const
		{
			return myLocalConstants == anOther.myLocalConstants &&
				myLocalTextures == anOther.myLocalTextures &&
				myLocalBuffers == anOther.myLocalBuffers &&
				myLocalTextureRWs == anOther.myLocalTextureRWs &&
				myLocalBufferRWs == anOther.myLocalBufferRWs &&
				myGlobalConstants == anOther.myGlobalConstants &&
				myGlobalTextures == anOther.myGlobalTextures &&
				myGlobalBuffers == anOther.myGlobalBuffers &&
				myGlobalTextureRWs == anOther.myGlobalTextureRWs &&
				myGlobalBufferRWs == anOther.myGlobalBufferRWs;
		}

		SR_LocalConstantsMask myLocalConstants;
		SR_LocalTexturesMask myLocalTextures;
		SR_LocalBuffersMask myLocalBuffers;
		SR_LocalTexturesRWMask myLocalTextureRWs;
		SR_LocalBuffersRWMask myLocalBufferRWs;
		SR_GlobalConstantsMask myGlobalConstants;
		SR_GlobalTexturesMask myGlobalTextures;
		SR_GlobalBuffersMask myGlobalBuffers;
		SR_GlobalTexturesRWMask myGlobalTextureRWs;
		SR_GlobalBuffersRWMask myGlobalBufferRWs;
	};
}