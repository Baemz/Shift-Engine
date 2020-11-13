#pragma once
#include "SR_RenderStateDescs.h"
#include "SR_PSOKey.h"

namespace Shift
{
	struct SR_ShaderStateDesc
	{
		struct PSOStruct
		{
			PSOStruct() : myTopology(SR_Topology_Unknown) {}

			SR_Topology myTopology;
			SR_DepthStateDesc myDepthStateDesc;
			SR_RasterizerStateDesc myRasterizerStateDesc;
			SR_BlendStateDesc myBlendStateDesc;
			SR_RenderTargetFormats myRenderTargetFormats;

			SR_PSOKey GetKey()
			{
				SR_PSOKey out;

				out.myTopologyKey = SC_Hash(myTopology);
				out.myDepthStateKey = SC_Hash(myDepthStateDesc);
				out.myRasterStateKey = SC_Hash(myRasterizerStateDesc);
				out.myBlendStateKey = SC_Hash(myBlendStateDesc);
				out.myRenderTargetsKey = SC_Hash(myRenderTargetFormats);

				return out;
			}
		};

		SR_ShaderStateDesc()
		{
		}

#if SR_ENABLE_RAYTRACING

		struct RTHitGroup
		{
			RTHitGroup() : myHasClosestHit(false), myHasAnyHit(false), myHasIntersection(false) {}

			bool myHasClosestHit : 1;
			bool myHasAnyHit : 1;
			bool myHasIntersection : 1;
		};

		SC_GrowingArray<RTHitGroup> myHitGroups;
#endif


		SC_Ref<SR_ShaderByteCode> myShaderByteCodes[SR_ShaderType_COUNT];

		PSOStruct myMainPSO;
		SC_GrowingArray<PSOStruct> myAdditionalPSOs;
	};

	class SR_ShaderState : public SR_Resource
	{
	public:
		SR_ShaderState();
		virtual ~SR_ShaderState();

		virtual uint GetHash() const = 0;

	public:
		struct MetaData
		{
			SC_Vector3ui myNumThreads;
		};

		MetaData myMetaData;

		bool IsCompute() const { return myIsCompute; }
		bool IsRaytracing() const { return myIsRaytracing; }

	protected:
		SC_HashMap<uint, SR_ShaderStateDesc> myProperties;
		bool myIsCompute : 1;
		bool myIsRaytracing : 1;
	};
}
