#pragma once
#include "SR_Format.h"

#define SR_MAX_RENDER_TARGETS 6

namespace Shift
{
	enum SR_GraphicsAPI : uint8
	{
		DirectX12,
		Vulkan,
		PS4,
		//PS5,
		//Metal,
	};

	enum SR_ContextType : uint8
	{
		SR_ContextType_Render,
		SR_ContextType_Compute,
		SR_ContextType_CopyInit,
		SR_ContextType_CopyStream,
		SR_ContextType_CopyFast,

		SR_ContextType_COUNT,
	};

	enum SR_BindFlags : uint
	{
		SR_BindFlag_StagingBuffer		= 0,
		SR_BindFlag_VertexBuffer		= 0x02,
		SR_BindFlag_IndexBuffer			= 0x04,
		SR_BindFlag_ConstantBuffer		= 0x08,
		SR_BindFlag_Buffer				= 0x10,
		SR_BindFlag_IndirectBuffer		= 0x20,
		SR_BindFlag_RaytracingBuffer	= 0x40,
		SR_BindFlag_Count				= 6,
	};

	enum SR_AccessGPU
	{
		SR_AccessGPU_Read,
		SR_AccessGPU_Write,
		SR_AccessGPU_Staging,
	};

	enum SR_AccessCPU
	{
		SR_AccessCPU_None,
		SR_AccessCPU_Map_Read, // For use together with SR_MemoryAccessGPU_Staging
		SR_AccessCPU_Map_Write,
		SR_AccessCPU_Map_Read_Write,
	};

	enum SR_MemoryAccess
	{
		SR_MemoryAccess_None = 0,
		SR_MemoryAccess_Read = 0x1,
		SR_MemoryAccess_Write = 0x2,
		SR_MemoryAccess_Read_Write = SR_MemoryAccess_Read | SR_MemoryAccess_Write,
	};

	enum SR_Dimension : uint8
	{
		SR_Dimension_Unknown = 0,
		SR_Dimension_Buffer,
		SR_Dimension_Texture1D,
		SR_Dimension_Texture2D,
		SR_Dimension_Texture3D,
		SR_Dimension_TextureCube,
	};

	enum SR_ResourceFlag : uint
	{
		SR_ResourceFlag_None = 0,
		SR_ResourceFlag_AllowRenderTarget = 0x1,
		SR_ResourceFlag_AllowDepthStencil = 0x2,
		SR_ResourceFlag_AllowWrites = 0x4,
		SR_ResourceFlag_DenyShaderAccess = 0x8,
		SR_ResourceFlag_AllowCrossAdapter = 0x10,
		SR_ResourceFlag_AllowSimultaneousAccess = 0x20,
	};

	enum SR_ResourceState : uint
	{
		SR_ResourceState_Common = 0,
		SR_ResourceState_VertexConstantBuffer = 0x1,
		SR_ResourceState_IndexBuffer = 0x2,
		SR_ResourceState_RenderTarget = 0x4,
		SR_ResourceState_UnorderedAccess = 0x8,
		SR_ResourceState_DepthWrite = 0x10,
		SR_ResourceState_DepthRead = 0x20,
		SR_ResourceState_NonPixelSRV = 0x40,
		SR_ResourceState_PixelSRV = 0x80,
		SR_ResourceState_StreamOut = 0x100,
		SR_ResourceState_Indirect = 0x200,
		SR_ResourceState_CopyDest = 0x400,
		SR_ResourceState_CopySrc = 0x800,
		SR_ResourceState_ResolveDest = 0x1000,
		SR_ResourceState_ResolveSrc = 0x2000,
		SR_ResourceState_RaytracingAccelerationStruct = 0x400000,
		SR_ResourceState_GenericRead = (((((0x1 | 0x2) | 0x40) | 0x80) | 0x200) | 0x800),
		SR_ResourceState_Present = 0,
		SR_ResourceState_Predication = 0x200,
		SR_ResourceState_VideoDecodeRead = 0x10000,
		SR_ResourceState_VideoDecodeWrite = 0x20000,
		SR_ResourceState_VideoProcessRead = 0x40000,
		SR_ResourceState_VideoProcessWrite = 0x80000,
		SR_ResourceState_VideoEncodeRead = 0x200000,
		SR_ResourceState_VideoEncodeWrite = 0x800000
	};

	enum SR_Topology : uint8
	{
		SR_Topology_TriangleList,
		SR_Topology_PointList,
		SR_Topology_LineList,
		SR_Topology_Patch,
		SR_Topology_Unknown,
		SR_Topology_COUNT = SR_Topology_Unknown
	};

	enum SR_StencilOperator : uint8
	{
		SR_StencilOperator_Keep,
		SR_StencilOperator_Zero,
		SR_StencilOperator_Replace,
		SR_StencilOperator_Increment_Saturate,
		SR_StencilOperator_Decrement_Saturate,
		SR_StencilOperator_Invert,
		SR_StencilOperator_Increment,
		SR_StencilOperator_Decrement,
		SR_StencilOperator_COUNT,
	};

	enum SR_ComparisonFunc : uint8
	{
		SR_ComparisonFunc_Never,
		SR_ComparisonFunc_Less,
		SR_ComparisonFunc_Equal,
		SR_ComparisonFunc_LessEqual,
		SR_ComparisonFunc_Greater,
		SR_ComparisonFunc_NotEqual,
		SR_ComparisonFunc_GreaterEqual,
		SR_ComparisonFunc_Always,
		SR_ComparisonFunc_COUNT,
	};

	enum SR_RasterizerFaceCull : uint8
	{
		SR_RasterizerFaceCull_None,
		SR_RasterizerFaceCull_Front,
		SR_RasterizerFaceCull_Back,
		SR_RasterizerFaceCull_COUNT,
	};

	enum SR_BlendFunc : uint8
	{
		SR_BlendFunc_Add,
		SR_BlendFunc_Subtract,
		SR_BlendFunc_Reverse_Subtract,
		SR_BlendFunc_Min,
		SR_BlendFunc_Max,
		SR_BlendFunc_COUNT,
	};

	enum SR_BlendMode : uint8
	{
		SR_BlendMode_Zero,
		SR_BlendMode_One,
		SR_BlendMode_Src_Color,
		SR_BlendMode_One_Minus_Src_Color,
		SR_BlendMode_Src_Alpha,
		SR_BlendMode_One_Minus_Src_Alpha,
		SR_BlendMode_Dst_Color,
		SR_BlendMode_One_Minus_Dst_Color,
		SR_BlendMode_Dst_Alpha,
		SR_BlendMode_One_Minus_Dst_Alpha,
		SR_BlendMode_Src_Alpha_Saturate,
		SR_BlendMode_Blend_Factor,
		SR_BlendMode_One_Minus_Blend_Factor,
		SR_BlendMode_Src1_Color,
		SR_BlendMode_One_Minus_Src1_Color,
		SR_BlendMode_Src1_Alpha,
		SR_BlendMode_One_Minus_Src1_Alpha,
		SR_BlendMode_COUNT,
	};

	enum SR_ColorWriteMask : uint8
	{
		SR_ColorWriteMask_None = (0),
		SR_ColorWriteMask_Red = (1 << 0),
		SR_ColorWriteMask_Green = (1 << 1),
		SR_ColorWriteMask_Blue = (1 << 2),
		SR_ColorWriteMask_Alpha = (1 << 3),
		SR_ColorWriteMask_RGB = (SR_ColorWriteMask_Red | SR_ColorWriteMask_Green | SR_ColorWriteMask_Blue),
		SR_ColorWriteMask_RGBA = (SR_ColorWriteMask_Red | SR_ColorWriteMask_Green | SR_ColorWriteMask_Blue | SR_ColorWriteMask_Alpha),
	};

	enum SR_ShaderType : uint8
	{
		SR_ShaderType_Vertex,
		SR_ShaderType_Pixel,

#if SR_ENABLE_GEOMETRY_SHADERS
		SR_ShaderType_Geometry, 
#endif

#if SR_ENABLE_TESSELATION_SHADERS
		SR_ShaderType_Hull,
		SR_ShaderType_Domain,
#endif

		SR_ShaderType_Compute,

#if SR_ENABLE_RAYTRACING
		SR_ShaderType_Raytracing,
#endif

#if SR_ENABLE_MESH_SHADERS
		SR_ShaderType_Mesh,
#endif

		SR_ShaderType_COUNT,
	};

#if SR_ENABLE_RAYTRACING
	enum SR_RTShader_HitGroup : uint8
	{
		SR_RTShader_HitGroup_ClosestHit,
		SR_RTShader_HitGroup_AnyHit,
		SR_RTShader_HitGroup_Intersection,
		SR_RTShader_HitGroup_COUNT
	};
#endif
}