#pragma once

namespace Shift
{
	enum SG_AmbientOcclusionType
	{
		SG_AmbientOcclusionType_SSAO,
		SG_AmbientOcclusionType_HBAO,
		SG_AmbientOcclusionType_RTAO
	};

	enum SG_AmbientOcclusionQuality
	{
		SG_AmbientOcclusionQuality_Low,
		SG_AmbientOcclusionQuality_Medium,
		SG_AmbientOcclusionQuality_High,
		SG_AmbientOcclusionQuality_VeryHigh,
		SG_AmbientOcclusionQuality_Ultra,
	};

	enum SG_AnisotropyQuality
	{
		SG_AnisotropyQuality_X1,
		SG_AnisotropyQuality_X2,
		SG_AnisotropyQuality_X4,
		SG_AnisotropyQuality_X8,
		SG_AnisotropyQuality_X16,
	};

	enum SG_AntiAliasingType
	{
		SG_AntiAliasingType_FXAA,
		SG_AntiAliasingType_SMAA,
		SG_AntiAliasingType_TAA,
	}; 
	
	enum SG_AntiAliasingQuality
	{
		SG_AntiAliasingQuality_Low,
		SG_AntiAliasingQuality_Medium,
		SG_AntiAliasingQuality_High,
		SG_AntiAliasingQuality_VeryHigh,
	};

	enum SG_FogQuality
	{
		SG_FogQuality_Low,
		SG_FogQuality_Medium,
		SG_FogQuality_High,
		SG_FogQuality_VeryHigh,
		SG_FogQuality_Ultra,
	};

	enum SG_ParticleQuality
	{
		SG_ParticleQuality_Low,
		SG_ParticleQuality_Medium,
		SG_ParticleQuality_High,
	};

	enum SG_ReflectionsType
	{
		SG_ReflectionsType_ScreenSpace,
		SG_ReflectionsType_Raytraced,
	};

	enum SG_ReflectionsQuality
	{
		SG_ReflectionsQuality_Low,
		SG_ReflectionsQuality_Medium,
		SG_ReflectionsQuality_High,
	};

	enum SG_ShadowQuality
	{
		SG_ShadowQuality_Low,
		SG_ShadowQuality_Medium,
		SG_ShadowQuality_High,
		SG_ShadowQuality_VeryHigh,
		SG_ShadowQuality_Ultra,
	};

#define SG_DEFINE_SETTING(aType, aId)					\
	private:											\
	aType my##aId;										\
	public:												\
	void Set##aId(aType aValue) { my##aId = aValue; }	\
	const aType& Get##aId() const { return my##aId; }

	class SG_GraphicsSettings : public SC_SimpleSingleton<SG_GraphicsSettings>
	{
	public:
		SG_GraphicsSettings() { memset(this, 0, sizeof(*this)); }
		
		SG_DEFINE_SETTING(SG_AmbientOcclusionType, AmbientOcclusionType);
		SG_DEFINE_SETTING(SG_AmbientOcclusionQuality, AmbientOcclusionQuality);
		SG_DEFINE_SETTING(SG_AnisotropyQuality, AnisotropyQuality);
		SG_DEFINE_SETTING(SG_AntiAliasingType, AntiAliasingType);
		SG_DEFINE_SETTING(SG_AntiAliasingQuality, AntiAliasingQuality);
		SG_DEFINE_SETTING(SG_FogQuality, FogQuality);
		SG_DEFINE_SETTING(SG_ParticleQuality, ParticleQuality);
		SG_DEFINE_SETTING(SG_ReflectionsType, ReflectionsType);
		SG_DEFINE_SETTING(SG_ReflectionsQuality, ReflectionsQuality);
		SG_DEFINE_SETTING(SG_ShadowQuality, ShadowQuality);
	};

#undef SG_DEFINE_SETTING
}

