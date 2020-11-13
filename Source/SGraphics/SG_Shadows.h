#pragma once
#include "SG_Camera.h"
#include "SR_TextureResource.h"

namespace Shift
{
	class SR_TextureBuffer;
	class SR_Texture;
	class SR_RenderTarget;
	class SG_RenderData;
	class SG_View;
	struct SG_ShadowConstants;

	class SG_Shadows
	{
	public:
		static constexpr uint ourNumShadowCascades = 4;
		static constexpr uint ourCSMResolution = 2048;
		static constexpr uint ourMaxShadowRange = 100;
		static constexpr float ourLinearLogSplitBalance = 0.96f;
		static constexpr float ourCSMFrustumThresholdAngle = 0.1f;

		struct CSMArray
		{
			bool Init(uint aNumCascades, uint aResolution);

			SC_Ref<SR_TextureBuffer> myTextureBuffer;
			SC_Ref<SR_Texture> myTexture;
			SC_GrowingArray<SC_Ref<SR_RenderTarget>> myRenderTargets;
			SC_GrowingArray<SC_Ref<SR_Texture>> myTextureSlices;
		};

		struct CachedCascadeParams
		{
			CachedCascadeParams();
			SC_Vector3f myCameraToMBSCenter[ourNumShadowCascades];
			SC_Vector3f myOldPositions[ourNumShadowCascades];
			float myRadius[ourNumShadowCascades];
			float myCascadeSplitPoints[(ourNumShadowCascades + 1)];

			float myCachedCascadeSplitPoints[(ourNumShadowCascades + 1)];
			float myCachedAspectRatio;
			float myCachedFov;
			bool myNeedsUpdate;
		};

		struct CascadeOldValues
		{
			SC_Vector3f myOldCenters[ourNumShadowCascades];
			SC_Vector2f myOldOffsets[ourNumShadowCascades];
			SC_Vector3f myOldLightDirections[ourNumShadowCascades];
		};


	public:
		SG_Shadows();
		~SG_Shadows();

		void Init();

		void SetupCSM();

		void PrepareShadowConstants(SG_ShadowConstants& aConstants, const SG_RenderData& aRenderData) const;

		void PrepareView(SG_View* aView);
		void RenderShadows(SG_View* aView);

		SR_Texture* GetSunShadowMap() const;
		SR_Texture* GetSunShadowMapSlice(uint aMip) const;
		SR_Texture* GetShadowNoiseTexture() const;
		const SC_Matrix44 GetShadowProjection(const SG_Camera& aCascadeCamera, uint aCascadeIndex) const;
		const SC_Matrix44 GetShadowProjection(uint aCascadeIndex) const;
		const SC_Vector3f GetShadowPosition(uint aCascadeIndex) const;
		const SG_Camera& GetCSMCamera(uint aCascadeIndex) const;

		float GetCascadeSplitPoints(uint aCascadeIndex);
		float GetPCFFilterOffset() const;

		static bool lockCamera;
		static bool wasLocked;
		static float ourPCFFilterScaleOverride;
	private:
		void RenderCascade(uint aIndex, SG_View* aView);
		void CalculateCSMViewProjections(const SG_RenderData& aRenderData);
		void CalculateCascadeSplitPoints(float aViewNearPlane, float aFarPlane);
		bool CalculateMinBoundingSphere(const SG_Camera& aCamera, float& aRadiusOut, SC_Vector3f& aCenterOut, float aNear, float aFar);

		float myCascadeSplitPoints[(ourNumShadowCascades + 1)];

		float myShadowCastDistance;
		float myPCFFilterScale;

		CachedCascadeParams myCachedParams;
		CascadeOldValues myOldValues;
		SC_Vector3f myCachedLightDirection;

		CSMArray myCSMArray;
		SG_Camera myLockedViewCamera;
		SG_Camera myCSMCameras[ourNumShadowCascades];
		SC_Matrix44 myCSMViewProjs[ourNumShadowCascades];
		SC_Ref<SR_Texture> myRandomNoiseTexture;
	};
}

