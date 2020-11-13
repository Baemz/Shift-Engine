#pragma once
#include "SR_GBuffer.h"

namespace Shift
{
	struct SG_FrameResources
	{
		SR_GBuffer myGBuffer;

		SR_TextureResource myDepth;
		SR_TextureResource myDepthLinear;
		SR_TextureResource myAmbientOcclusion;

		SR_TextureResource myFullscreen;
		SR_TextureResource myFullscreen2;
		SR_TextureResource myFullscreenHDR;
		SR_TextureResource myQuarterScreen;
		SR_TextureResource myAvgLuminance;

		SC_Ref<SR_TextureBuffer> mySkyProbeTextureBuffer;
		SC_Ref<SR_Texture> mySkyProbeTexture;
		SC_Ref<SR_Texture> mySkyProbeTextureRW;

		SC_Ref<SR_TextureBuffer> mySkyProbeIrradianceTextureBuffer;
		SC_Ref<SR_Texture> mySkyProbeIrradianceTexture;
		SC_Ref<SR_Texture> mySkyProbeIrradianceTextureRW;

		SC_Ref<SR_Texture> mySkyBrdfTexture;
		SC_Ref<SR_Texture> myBloomTexture;

		SR_TextureResource myLastTextureWritten;
		SR_TextureResource myScreenHistory[4];
	};
}
