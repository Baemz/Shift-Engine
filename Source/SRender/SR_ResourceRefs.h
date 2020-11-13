#pragma once
#include "SC_Enum.h"

namespace Shift
{
	SC_ENUM(SR_ConstantBufferRef,
		SR_ConstantBufferRef_Local0 = 0,
		SR_ConstantBufferRef_Local1,
		SR_ConstantBufferRef_Local2,
		SR_ConstantBufferRef_Local3,
		SR_ConstantBufferRef_ViewConstants,
		SR_ConstantBufferRef_ShadowConstants,
		SR_ConstantBufferRef_EnvironmentConstants,
		SR_ConstantBufferRef_VoxelSettings
	);
	static constexpr uint SR_ConstantBufferRef_LocalStart = SR_ConstantBufferRef::SR_ConstantBufferRef_Local0;
	static constexpr uint SR_ConstantBufferRef_LocalEnd = SR_ConstantBufferRef::SR_ConstantBufferRef_Local3;
	static constexpr uint SR_ConstantBufferRef_NumLocals = SR_ConstantBufferRef_LocalEnd - SR_ConstantBufferRef_LocalStart + 1;
	static constexpr uint SR_ConstantBufferRef_GlobalStart = SR_ConstantBufferRef::SR_ConstantBufferRef_ViewConstants;
	static constexpr uint SR_ConstantBufferRef_GlobalEnd = SR_ConstantBufferRef::SR_ConstantBufferRef_VoxelSettings;
	static constexpr uint SR_ConstantBufferRef_NumGlobals = SR_ConstantBufferRef_GlobalEnd - SR_ConstantBufferRef_GlobalStart + 1;

	SC_ENUM(SR_SamplerRef,
		SR_SamplerRef_Linear_Clamp = 0,
		SR_SamplerRef_Linear_Wrap,
		SR_SamplerRef_Linear_Mirror,
		SR_SamplerRef_Point_Clamp,
		SR_SamplerRef_Point_Wrap,
		SR_SamplerRef_Point_Mirror,
		SR_SamplerRef_Aniso_Clamp,
		SR_SamplerRef_Aniso_Wrap,
		SR_SamplerRef_Aniso_Mirror,

		/* Comparison states*/
		SR_SamplerRef_CmpGreater_Linear_Clamp,
		SR_SamplerRef_CmpGreater_Point_Clamp,
		SR_SamplerRef_CmpGreater_Aniso_Clamp
	);

	SC_ENUM(SR_TextureRef,
		SR_TextureRef_Local0 = 0,
		SR_TextureRef_Local1,
		SR_TextureRef_Local2,
		SR_TextureRef_Local3,
		SR_TextureRef_Local4,
		SR_TextureRef_Local5,
		SR_TextureRef_Local6,
		SR_TextureRef_Local7,
		SR_TextureRef_Local8,
		SR_TextureRef_Local9,
		SR_TextureRef_Local10,
		SR_TextureRef_Local11,
		SR_TextureRef_Local12,
		SR_TextureRef_Local13,
		SR_TextureRef_Local14,
		SR_TextureRef_Local15,
		SR_TextureRef_Depth,
		SR_TextureRef_LinearDepth,
		SR_TextureRef_Stencil,
		SR_TextureRef_GBuffer_Color,
		SR_TextureRef_GBuffer_Normal,
		SR_TextureRef_GBuffer_ARM,
		SR_TextureRef_GBuffer_Optional_Emission,
		SR_TextureRef_GBuffer_Optional_MotionVector,
		SR_TextureRef_ShadowMapCSM,
		SR_TextureRef_ShadowMapNoise,
		SR_TextureRef_AmbientOcclusion,
		SR_TextureRef_VoxelScene,
		SR_TextureRef_SkyProbe,
		SR_TextureRef_SkyDiffuse,
		SR_TextureRef_SkySpecular,
		SR_TextureRef_SkyBrdf
	);
	static constexpr uint SR_TextureRef_LocalStart = SR_TextureRef::SR_TextureRef_Local0;
	static constexpr uint SR_TextureRef_LocalEnd = SR_TextureRef::SR_TextureRef_Local15;
	static constexpr uint SR_TextureRef_NumLocals = SR_TextureRef_LocalEnd - SR_TextureRef_LocalStart + 1;
	static constexpr uint SR_TextureRef_GlobalStart = SR_TextureRef::SR_TextureRef_Depth;
	static constexpr uint SR_TextureRef_GlobalEnd = SR_TextureRef::SR_TextureRef_SkyBrdf;
	static constexpr uint SR_TextureRef_NumGlobals = SR_TextureRef_GlobalEnd - SR_TextureRef_GlobalStart + 1;


	SC_ENUM(SR_TextureRWRef,
		SR_TextureRWRef_Local0 = 0,
		SR_TextureRWRef_Local1,
		SR_TextureRWRef_Local2,
		SR_TextureRWRef_Local3,
		SR_TextureRWRef_TESTDATA
		);
	static constexpr uint SR_TextureRWRef_LocalStart = SR_TextureRWRef::SR_TextureRWRef_Local0;
	static constexpr uint SR_TextureRWRef_LocalEnd = SR_TextureRWRef::SR_TextureRWRef_Local3;
	static constexpr uint SR_TextureRWRef_NumLocals = SR_TextureRWRef_LocalEnd - SR_TextureRWRef_LocalStart + 1;
	static constexpr uint SR_TextureRWRef_GlobalStart = SR_TextureRWRef::SR_TextureRWRef_TESTDATA;
	static constexpr uint SR_TextureRWRef_GlobalEnd = SR_TextureRWRef::SR_TextureRWRef_TESTDATA;
	static constexpr uint SR_TextureRWRef_NumGlobals = SR_TextureRWRef_GlobalEnd - SR_TextureRWRef_GlobalStart + 1;

	SC_ENUM(SR_BufferRef,
		SR_BufferRef_Local0 = 0,
		SR_BufferRef_Local1,
		SR_BufferRef_Local2,
		SR_BufferRef_Local3,
		SR_BufferRef_LightTileData,
		SR_BufferRef_RaytracingScene
		);
	static constexpr uint SR_BufferRef_LocalStart = SR_BufferRef::SR_BufferRef_Local0;
	static constexpr uint SR_BufferRef_LocalEnd = SR_BufferRef::SR_BufferRef_Local3;
	static constexpr uint SR_BufferRef_NumLocals = SR_BufferRef_LocalEnd - SR_BufferRef_LocalStart + 1;
	static constexpr uint SR_BufferRef_GlobalStart = SR_BufferRef::SR_BufferRef_LightTileData;
	static constexpr uint SR_BufferRef_GlobalEnd = SR_BufferRef::SR_BufferRef_RaytracingScene;
	static constexpr uint SR_BufferRef_NumGlobals = SR_BufferRef_GlobalEnd - SR_BufferRef_GlobalStart + 1;

	SC_ENUM(SR_BufferRWRef,
		SR_BufferRWRef_Local0 = 0,
		SR_BufferRWRef_Local1,
		SR_BufferRWRef_Local2,
		SR_BufferRWRef_Local3,
		SR_BufferRWRef_TESTDATA
		);
	static constexpr uint SR_BufferRWRef_LocalStart = SR_BufferRWRef::SR_BufferRWRef_Local0;
	static constexpr uint SR_BufferRWRef_LocalEnd = SR_BufferRWRef::SR_BufferRWRef_Local3;
	static constexpr uint SR_BufferRWRef_NumLocals = SR_BufferRWRef_LocalEnd - SR_BufferRWRef_LocalStart + 1;
	static constexpr uint SR_BufferRWRef_GlobalStart = SR_BufferRWRef::SR_BufferRWRef_TESTDATA;
	static constexpr uint SR_BufferRWRef_GlobalEnd = SR_BufferRWRef::SR_BufferRWRef_TESTDATA;
	static constexpr uint SR_BufferRWRef_NumGlobals = SR_BufferRWRef_GlobalEnd - SR_BufferRWRef_GlobalStart + 1;
	
}