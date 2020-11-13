#include "SGraphics_Precompiled.h"
#include "SG_CommonRenderStates.h"

namespace Shift
{
	SG_CommonRenderStates* SG_CommonRenderStates::ourInstance = nullptr;

	void SG_CommonRenderStates::Create()
	{
		if (ourInstance == nullptr)
			ourInstance = new SG_CommonRenderStates();
	}
	void SG_CommonRenderStates::Destroy()
	{
		SC_SAFE_DELETE(ourInstance);
	}

	SG_CommonRenderStates* SG_CommonRenderStates::GetInstance()
	{
		return ourInstance;
	}

	SG_CommonRenderStates::SG_CommonRenderStates()
	{
		SR_DepthStateDesc depthDesc;
		myDefaultDepthState = new SR_DepthState(depthDesc);

		depthDesc.myDepthComparisonFunc = SR_ComparisonFunc_GreaterEqual;
		depthDesc.myWriteDepth = true;
		myDepthGreaterEqualWriteState = new SR_DepthState(depthDesc);

		depthDesc.myWriteDepth = false;
		myDepthGreaterEqualReadState = new SR_DepthState(depthDesc);

		depthDesc.myDepthComparisonFunc = SR_ComparisonFunc_Equal;
		depthDesc.myWriteDepth = false;
		myDepthEqualReadState = new SR_DepthState(depthDesc);

		SR_RasterizerStateDesc rasterState;
		myDefaultRasterizerState = new SR_RasterizerState(rasterState);

		rasterState.myWireframe = true;
		myWireframeRasterizerState = new SR_RasterizerState(rasterState);

		rasterState = SR_RasterizerStateDesc();
		rasterState.myFaceCull = SR_RasterizerFaceCull_None;
		myTwoSidedRasterizerState = new SR_RasterizerState(rasterState);

		rasterState = SR_RasterizerStateDesc();
		rasterState.mySlopedScaleDepthBias = -2.0f;
		rasterState.myDepthBias = -100;
		rasterState.myEnableDepthClip = false;
		rasterState.myFaceCull = SR_RasterizerFaceCull_None;
		myCSMRasterizerState = new SR_RasterizerState(rasterState);

		rasterState.myFaceCull = SR_RasterizerFaceCull_None;
		myCSMTwoSidedRasterizerState = new SR_RasterizerState(rasterState);

		SR_RenderTargetFormats deferredRTFs;
		deferredRTFs.myNumColorFormats = 4;
		deferredRTFs.myColorFormats[0] = SR_Format_RGBA8_Unorm; // Color
		deferredRTFs.myColorFormats[1] = SR_Format_RG11B10_Float; // Normals
		deferredRTFs.myColorFormats[2] = SR_Format_RGBA8_Unorm; // ARM
		deferredRTFs.myColorFormats[3] = SR_Format_RGBA8_Unorm; // Emissive + Fog
		myDeferredRenderTargetFormats = new SR_RenderTargetFormatState(deferredRTFs);

		SR_BlendStateDesc noBlendState;
		myDefaultBlendState = new SR_BlendState(noBlendState);

		noBlendState.myNumRenderTargets = 1;
		noBlendState.myRenderTagetBlendDescs[0].mySrcBlend = SR_BlendMode_One;
		noBlendState.myRenderTagetBlendDescs[0].myDstBlend = SR_BlendMode_Zero;
		noBlendState.myRenderTagetBlendDescs[0].mySrcBlendAlpha = SR_BlendMode_One;
		noBlendState.myRenderTagetBlendDescs[0].myDstBlendAlpha = SR_BlendMode_Zero;
		myNoBlendState = new SR_BlendState(noBlendState);

		SR_BlendStateDesc deferredBlendState;
		deferredBlendState.myNumRenderTargets = 4;
		for (uint i = 0; i < deferredRTFs.myNumColorFormats; ++i)
		{
			deferredBlendState.myRenderTagetBlendDescs[i].mySrcBlend = SR_BlendMode_One;
			deferredBlendState.myRenderTagetBlendDescs[i].myDstBlend = SR_BlendMode_Zero;
			deferredBlendState.myRenderTagetBlendDescs[i].mySrcBlendAlpha = SR_BlendMode_One;
			deferredBlendState.myRenderTagetBlendDescs[i].myDstBlendAlpha = SR_BlendMode_Zero;
		}
		myDeferredBlendState = new SR_BlendState(deferredBlendState);
	}

	SG_CommonRenderStates::~SG_CommonRenderStates()
	{
	}
}