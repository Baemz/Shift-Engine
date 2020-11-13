#pragma once
#include "SR_Format.h"

namespace Shift
{
	enum EShaderTypes : unsigned char
	{
		VS,
		HS,
		DS,
		GS,
		PS,
		CS,
		COUNT
	};

	enum class ERasterizerState : unsigned char
	{
		Fill = 0,
		Fill_FrontFaceCull,
		Fill_NoFaceCull,
		Wireframe,
		Wireframe_FrontFaceCull,
		Wireframe_NoFaceCull,
		ERS_Begin = Fill,
		ERS_End = Wireframe_NoFaceCull,
		ERS_Count = ERS_End + 1,
	};

	enum class EBlendState : unsigned char
	{
		Disabled = 0,
		Blend,
		Add,
		Mul,
		Custom,
		EBS_Begin = Disabled,
		EBS_End = Mul,
		EBS_Count = EBS_End + 1
	};

	enum EDefaultGraphicsPSO : unsigned char
	{
		GBuffer = 0,
		Lighting,
		Transparent,
		Terrain,
		Wireframe,
		FullscreenImage,
		EDGPSO_Begin = GBuffer,
		EDGPSO_End = FullscreenImage,
		EDGPSO_Count = EDGPSO_End + 1,
	};

	enum EBindFlags : unsigned int
	{
		SBindFlag_Unknown = 0,
		SBindFlag_VertexBuffer = 0x02,
		SBindFlag_IndexBuffer = 0x04,
		SBindFlag_ConstantBuffer = 0x08,
		SBindFlag_Buffer = 0x10,
		SBindFlag_StructuredBuffer = 0x20,
		SBindFlag_ByteBuffer = 0x40,
		SBindFlag_Count = 7,
	};

	enum EMemoryAccess : unsigned char
	{
		SMemoryAccessRW_CPU,
		SR_MemoryAccess_CPU,
		SMemoryAccessRW_GPU,
		SR_MemoryAccess_GPU,
	};

	enum EMiscFlags : unsigned char
	{
	};

	enum EDepthState : unsigned char
	{
		SDepthState_Disabled,
		SDepthState_Read,
		SDepthState_Write
	};

	enum EDepthFunc : unsigned char
	{
		SDepthFunc_Greater,
		SDepthFunc_GreaterEqual,
		SDepthFunc_Less,
		SDepthFunc_LessEqual
	};

	enum EQueryType : unsigned char
	{
		SQueryType_Occlusion,
		SQueryType_BinaryOcclusion,
		SQueryType_Timestamp
	};
}