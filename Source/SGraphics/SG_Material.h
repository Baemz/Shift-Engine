#pragma once
#include "SG_RenderType.h"
#include "SR_Texture.h"

namespace Shift
{
	class SR_ShaderState;
	struct SG_MaterialDesc
	{
		static constexpr uint ourMaterialDescVersion = 0x2;

		SG_MaterialDesc()
			: myIsOpaque(true)
			, myNeedsGBuffer(true)
			, myNeedsForward(false)
			, myIsShadowCaster(true)
			, myAlphaTest(false)
			, myUseTessellation(false)
			, myIsTwoSided(false)
			, myAddToRaytracingScene(false)
		{}

		void FromFile(const char* aFilePath);
		bool ToFile(const char* aFilePath);

		SC_GrowingArray<std::string> myTextures;
		std::string myVertexShader;
		std::string myPixelShader;
		std::string myHullShader;
		std::string myDomainShader;
		std::string myLodPresetId;

		bool myIsOpaque;
		bool myNeedsForward;
		bool myNeedsGBuffer;
		bool myIsShadowCaster;
		bool myAlphaTest;
		bool myUseTessellation;
		bool myIsTwoSided;
		bool myAddToRaytracingScene;
	};

	inline uint SC_Hash(const SG_MaterialDesc& anItem)
	{
		uint hash = 0;
		hash += SC_Hash(anItem.myIsOpaque);
		hash += SC_Hash(anItem.myNeedsForward);
		hash += SC_Hash(anItem.myNeedsGBuffer);
		hash += SC_Hash(anItem.myIsShadowCaster);
		hash += SC_Hash(anItem.myAlphaTest);
		hash += SC_Hash(anItem.myUseTessellation);
		hash += SC_Hash(anItem.myIsTwoSided);
		hash += SC_Hash(anItem.myAddToRaytracingScene);
		hash += SC_Hash(anItem.myVertexShader.c_str());
		hash += SC_Hash(anItem.myPixelShader.c_str());
		hash += SC_Hash(anItem.myHullShader.c_str());
		hash += SC_Hash(anItem.myDomainShader.c_str());
		hash += SC_Hash(anItem.myLodPresetId.c_str());

		for (const std::string& tex : anItem.myTextures)
			hash += SC_Hash(tex.c_str());

		return hash;
	}

	class SG_Material : public SC_Resource
	{
		friend class SG_MaterialFactory;
	public:
		SG_Material();
		~SG_Material();

		const SR_Topology& GetTopology() const { return myTopology; }

		const char* GetLodPresetId() const { return myProperties.myLodPresetId.c_str(); }

		void BindTextures();
		void BindConstants();

		SC_Ref<SR_ShaderState> myShaders[SG_RenderType_COUNT];
		SC_GrowingArray<SC_Ref<SR_Texture>> myTextures;
		SG_MaterialDesc myProperties;
		SR_Topology myTopology;
	};
}
