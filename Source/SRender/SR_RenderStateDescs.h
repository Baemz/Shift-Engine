#pragma once
#include "SR_RenderEnums.h"

namespace Shift
{
	struct SR_StencilDesc
	{
		SR_StencilOperator myFail;
		SR_StencilOperator myDepthFail;
		SR_StencilOperator myPass;
		SR_ComparisonFunc myStencilComparisonFunc;

		SR_StencilDesc()
			: myFail(SR_StencilOperator_Keep)
			, myDepthFail(SR_StencilOperator_Keep)
			, myPass(SR_StencilOperator_Keep)
			, myStencilComparisonFunc(SR_ComparisonFunc_Always)
		{}
	};

	struct SR_DepthStateDesc
	{
		SR_DepthStateDesc()
			: myWriteDepth(false)
			, myDepthComparisonFunc(SR_ComparisonFunc_Always)
			, myEnableStencil(false)
			, myStencilReadMask(0xff)
			, myStencilWriteMask(0xff)
		{}

		bool myWriteDepth;
		SR_ComparisonFunc myDepthComparisonFunc;

		bool myEnableStencil;
		uint8 myStencilReadMask;
		uint8 myStencilWriteMask;
		SR_StencilDesc myFrontFace;
		SR_StencilDesc myBackFace;
	};

	inline uint SC_Hash(const SR_DepthStateDesc& anItem)
	{
		uint hash = anItem.myWriteDepth;
		hash += uint(anItem.myDepthComparisonFunc) << 1;
		hash += uint(anItem.myEnableStencil) << 4;
		hash += uint(anItem.myFrontFace.myFail) << 5;
		hash += uint(anItem.myFrontFace.myDepthFail) << 8;
		hash += uint(anItem.myFrontFace.myPass) << 11;
		hash += uint(anItem.myFrontFace.myStencilComparisonFunc) << 14;
		hash += uint(anItem.myBackFace.myFail) << 17;
		hash += uint(anItem.myBackFace.myDepthFail) << 20;
		hash += uint(anItem.myBackFace.myPass) << 23;
		hash += uint(anItem.myBackFace.myStencilComparisonFunc) << 26;
		hash ^= uint(anItem.myStencilReadMask) << 8;
		hash ^= uint(anItem.myStencilWriteMask) << 16;

		return hash;
	}

	struct SR_RasterizerStateDesc
	{
		SR_RasterizerStateDesc()
			: myDepthBias(0)
			, myDepthBiasClamp(0.0f)
			, mySlopedScaleDepthBias(0.0f)
			, myFaceCull(SR_RasterizerFaceCull_Back)
			, myEnableDepthClip(true)
			, myConservativeRaster(false)
			, myWireframe(false)
		{}

		int myDepthBias;
		float myDepthBiasClamp;
		float mySlopedScaleDepthBias;

		SR_RasterizerFaceCull myFaceCull;

		bool myEnableDepthClip;
		bool myConservativeRaster;
		bool myWireframe;
	}; 

	inline uint SC_Hash(const SR_RasterizerStateDesc& anItem)
	{
		uint hash = anItem.myWireframe;
		hash += uint(anItem.myFaceCull) << 1;
		hash += uint(anItem.myEnableDepthClip) << 3;
		hash += uint(anItem.myDepthBias) << 4;
		hash += *((uint*)&anItem.mySlopedScaleDepthBias) << 8;
		hash += *((uint*)&anItem.myDepthBiasClamp) << 16;

		return hash;
	}

	struct SR_RenderTargetBlendDesc
	{
		SR_RenderTargetBlendDesc()
			: myEnableBlend(false)
			, myWriteMask(SR_ColorWriteMask_RGBA) 
			, mySrcBlend(SR_BlendMode_Src_Alpha)
			, myDstBlend(SR_BlendMode_One_Minus_Src_Alpha)
			, myBlendFunc(SR_BlendFunc_Add)
			, mySrcBlendAlpha(SR_BlendMode_Zero)
			, myDstBlendAlpha(SR_BlendMode_One)
			, myBlendFuncAlpha(SR_BlendFunc_Add)
		{
		}

		uint GetHash() const
		{
			uint hash = myEnableBlend;
			hash += uint(mySrcBlend) << 1;
			hash += uint(myDstBlend) << 6;
			hash += uint(myBlendFunc) << 11;
			hash += uint(mySrcBlendAlpha) << 14;
			hash += uint(myDstBlendAlpha) << 19;
			hash += uint(myBlendFuncAlpha) << 24;
			hash += uint(myWriteMask) << 27;
			return hash;
		}

		bool myEnableBlend;
		uint8 myWriteMask;

		SR_BlendMode mySrcBlend;
		SR_BlendMode myDstBlend;
		SR_BlendFunc myBlendFunc;

		SR_BlendMode mySrcBlendAlpha;
		SR_BlendMode myDstBlendAlpha;
		SR_BlendFunc myBlendFuncAlpha;
	};

	struct SR_BlendStateDesc
	{
		SR_BlendStateDesc()
			: myNumRenderTargets(0)
			, myAlphaToCoverage(false)
		{}

		uint8 myNumRenderTargets;
		SR_RenderTargetBlendDesc myRenderTagetBlendDescs[SR_MAX_RENDER_TARGETS];

		bool myAlphaToCoverage;
	}; 
	
	inline uint SC_Hash(const SR_BlendStateDesc& anItem)
	{
		uint hash = anItem.myAlphaToCoverage;
		hash += uint(anItem.myNumRenderTargets) << 1;
		for (int i = 0, e = SC_Max(1U, uint(anItem.myNumRenderTargets)); i < e; ++i)
			hash ^= anItem.myRenderTagetBlendDescs[i].GetHash() << (i + 2);

		return hash;
	}

	struct SR_RenderTargetFormats
	{
		SR_RenderTargetFormats()
			: myDepthFormat(SR_Format_D32_Float)
			, myNumColorFormats(0)
		{
			for (uint i = 0; i < 6; ++i)
				myColorFormats[i] = SR_Format_Unknown;
		}

		uint8 myNumColorFormats;
		SR_Format myColorFormats[6];
		SR_Format myDepthFormat;
	};
	static_assert(sizeof(SR_RenderTargetFormats) == sizeof(uint64), "unexpected size");
	SC_ENABLE_RAW_HASHING(SR_RenderTargetFormats);
}