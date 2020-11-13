#pragma once

namespace Shift
{
	class SR_RasterizerState;
	class SR_BlendState;
	class SR_DepthState;

	class SG_CommonRenderStates
	{
	public:
		static void Create();
		static void Destroy();
		static SG_CommonRenderStates* GetInstance();

		SC_Ref<SR_RasterizerState> myDefaultRasterizerState;
		SC_Ref<SR_RasterizerState> myCSMRasterizerState;
		SC_Ref<SR_RasterizerState> myCSMTwoSidedRasterizerState;
		SC_Ref<SR_RasterizerState> myWireframeRasterizerState;
		SC_Ref<SR_RasterizerState> myTwoSidedRasterizerState;

		SC_Ref<SR_BlendState> myDefaultBlendState;
		SC_Ref<SR_BlendState> myNoBlendState;

		SC_Ref<SR_DepthState> myDefaultDepthState;
		SC_Ref<SR_DepthState> myDepthGreaterEqualWriteState;
		SC_Ref<SR_DepthState> myDepthGreaterEqualReadState;
		SC_Ref<SR_DepthState> myDepthEqualReadState;

		SC_Ref<SR_RenderTargetFormatState> myDeferredRenderTargetFormats;
		SC_Ref<SR_BlendState> myDeferredBlendState;

	private:
		SG_CommonRenderStates();
		~SG_CommonRenderStates();

		static SG_CommonRenderStates* ourInstance;
	};
}


