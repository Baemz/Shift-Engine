#pragma once
#include "SR_TextureResource.h"

namespace Shift
{
	class SR_Texture;
	class SG_View;

	enum SG_TonemapType
	{
		Reinhard,
		Uncharted,
		BurgessDawson,
		FilmicACES,
	};

	struct SG_PostEffectsSettings
	{
		float myExposure;
		SG_TonemapType myTonemapType;
		bool myUseTonemap;

		float myBloomIntensity;
		float myBloomThreshold;
		bool myUseBloom;

		bool myUseGlare : 1;
		bool myUseFXAA : 1;
	};


	class SG_PostEffects
	{
	public:
		struct Shared
		{
		public:
			Shared();
			~Shared();

			void Load();

			SC_Ref<SR_ShaderState> myDownsampleShader;
			SC_Ref<SR_ShaderState> myBloomUpsampleShader;
			SC_Ref<SR_ShaderState> myInitialLumReductionShader;
			SC_Ref<SR_ShaderState> myLumReductionShader;
			SC_Ref<SR_ShaderState> myFinalLumReductionShader;
			SC_Ref<SR_ShaderState> myTonemapShader;

		private:
			bool myIsLoaded;
		};

		SG_PostEffects();
		~SG_PostEffects();

		void Init(SC_Vector2f aResolution);
		void Render(SG_View* aView);

		void CalculateLuminance(SG_View* aView, SR_Texture* aInputTexture);
		void Bloom(SG_View* aView);
		void Tonemap(SG_View* aView);

	private:
		//void ChromaticAbberation();
		void Tonemap(SR_Texture* aFramebufferTexture, SR_Texture* aBloomTexture);


		void RecursiveBloomDownsample(SR_TextureResource& aOutTexture, SR_Texture* aSourceMip, const uint aCurrentMip);
		void BloomUpsample(SR_Texture* aOutTexture, SR_Texture* aInTexture);
		void Downsample(SR_Texture* aOutTexture, SR_Texture* aSourceTexture);



	private:
		static Shared* ourSharedCache;
		SC_GrowingArray<SR_TextureResource> myLuminanceReductionTargets;
		SC_GrowingArray<SR_TextureResource> myBloomTargets;
		SC_GrowingArray<SR_TextureResource> myBloomUpTargets;

		SG_PostEffectsSettings mySettings;
	};
}

