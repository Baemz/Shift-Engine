/*

	MATCH BINDINGS WITH "Data/Shaders/GeneralShaderInterop.h"

*/

#pragma once
#ifndef _GENERAL_SHADER_INTEROP_
#define _GENERAL_SHADER_INTEROP_

#define USE_REVERSE_Z 1

// Static Texture Slot Mapping
#define TEXSLOT_SCENE_DEPTH						10
#define TEXSLOT_SKY_DIFFUSE						11
#define TEXSLOT_SKY_SPECULAR					12
#define TEXSLOT_SKY_BRDF						13
#define TEXSLOT_SSAO_RAND						14

#define TEXSLOT_GBUFFER_COLOR					15
#define TEXSLOT_GBUFFER_NORMALS					16
#define TEXSLOT_GBUFFER_ARM						17
#define TEXSLOT_GBUFFER_EMISSION				18
#define TEXSLOT_VOXEL_SCENE						19

#define TEXSLOT_SUN_SHADOWMAP_CSM				20
#define TEXSLOT_SUN_SHADOWMAP_NOISE				21

// SamplerState Slot Mapping
#define SAMPLERSLOT_LINEAR_CLAMP				0
#define SAMPLERSLOT_LINEAR_WRAP					1
#define SAMPLERSLOT_LINEAR_MIRROR				2

#define SAMPLERSLOT_POINT_CLAMP					3
#define SAMPLERSLOT_POINT_WRAP					4
#define SAMPLERSLOT_POINT_MIRROR				5

#define SAMPLERSLOT_ANISO16_CLAMP				6
#define SAMPLERSLOT_ANISO16_WRAP				7
#define SAMPLERSLOT_ANISO16_MIRROR				8

#define SAMPLERSLOT_CMP_LINEAR_CLAMP			9
#define SAMPLERSLOT_CMP_POINT_CLAMP				10
#define SAMPLERSLOT_CMP_ANISO16_CLAMP			11

#define SAMPLERSLOT_COUNT						12

// ConstantBuffer Slot Mapping
#define CBSLOT_RENDERER_WORLD					0
#define CBSLOT_RENDERER_FRAME					1
#define CBSLOT_RENDERER_MAINCAMERA				2
#define CBSLOT_SHADOW_CONSTANTS					4

#ifdef __cplusplus


#else // HLSL

#endif // HLSL

#endif // _GENERAL_SHADER_INTEROP_