#pragma once
#include "SC_Vector.h"

namespace Shift
{
	struct SVertex_Terrain
	{
		SC_Vector4f myPosition;
		SC_Vector2f myUV;
	};

	struct SVertex_Simple
	{
		SVertex_Simple() {};
		SVertex_Simple(const SC_Vector4f& aPos, const SC_Vector2f& aUV, const SC_Vector3f& aNormal, const SC_Vector3f& aTangent, const SC_Vector3f& aBinormal, const SC_Vector4f& aColor)
			: position(aPos)
			, normal(aNormal)
			, tangent(aTangent)
			, binormal(aBinormal)
			, uv(aUV)
			, color(aColor)
		{}
		SVertex_Simple(const SC_Vector4f& aPos, const SC_Vector2f& aUV, const SC_Vector4f& aColor)
			: position(aPos)
			, normal(0.0f, 0.0f, 0.0f)
			, tangent(0.0f, 0.0f, 0.0f)
			, binormal(0.0f, 0.0f, 0.0f)
			, uv(aUV)
			, color(aColor)
		{}
		SVertex_Simple(const SC_Vector4f& aPos, const SC_Vector4f& aColor)
			: position(aPos)
			, uv(0.0f, 0.0f)
			, color(aColor)
		{}
		SC_Vector4f position;
		SC_Vector4f color;
		SC_Vector3f normal;
		SC_Vector3f tangent;
		SC_Vector3f binormal;
		SC_Vector2f uv;
	};
	struct SVertex_Common
	{
		SVertex_Common() {};
		SVertex_Common(const SC_Vector4f& aPos, const SC_Vector4f& aPrevPos, const SC_Vector2f& aUV)
			: position(aPos)
			, prevPosition(aPrevPos)
			, uv(aUV)
		{}

		SC_Vector4f position;
		SC_Vector4f prevPosition;
		SC_Vector2f uv;
	};


	struct SVertex_ObjectInputInstance
	{

		bool operator==(const SVertex_ObjectInputInstance& aOther)
		{
			if (mat0 == aOther.mat0 &&
				mat1 == aOther.mat1 &&
				mat2 == aOther.mat2 &&
				mat3 == aOther.mat3 &&
				instanceColor == aOther.instanceColor)
			{
				return true;
			}

			return false;
		}

		SC_Vector4f mat0;
		SC_Vector4f mat1;
		SC_Vector4f mat2;
		SC_Vector4f mat3;
		SC_Vector4f instanceColor;
	};

	static const SVertex_Simple globalQuadVertexList[] =
	{
		//const float leftX = -1.0f;
		//const float topY = 1.0f;
		//const float rightX = 1.0f;
		//const float bottomY = -1.0f;
		//SSimpleTexturedVertex vertices[4] =
		//{
		//	// X	Y			Z	 W		U	 V
		//	{ leftX, bottomY,	0.f, 1.f,	0.f, 1.f }, // Bottom left
		//	{ leftX, topY,		0.f, 1.f,	0.f, 0.f }, // Top left
		//	{ rightX, topY,		0.f, 1.f,	1.f, 0.f }, // Top right
		//	{ rightX, bottomY,	0.f, 1.f,	1.f, 1.f }  // Bottom right
		//};


		SVertex_Simple(SC_Vector4f(-1.0f, -1.0f, 0.0f, 1.0f), SC_Vector2f(0.0f, 1.0f), SC_Vector4f(0.0f, 1.0f, 0.0f, 1.0f)), // Bottom Left

		SVertex_Simple(SC_Vector4f(-1.0f, 1.0f, 0.0f, 1.0f), SC_Vector2f(0.0f, 0.0f), SC_Vector4f(0.0f, 0.0f, 1.0f, 1.0f)), // Top Left

		SVertex_Simple(SC_Vector4f(1.0f, 1.0f, 0.0f, 1.0f), SC_Vector2f(1.0f, 0.0f), SC_Vector4f(1.0f, 1.0f, 1.0f, 1.0f)), // Top Right

		SVertex_Simple(SC_Vector4f(1.0f, -1.0f, 0.0f, 1.0f), SC_Vector2f(1.0f, 1.0f), SC_Vector4f(1.f, 0.0f, 0.0f, 1.0f)) // Bottom Right
	}; 
	static const unsigned int globalQuadIndexList[] =
	{
		1, 2, 0,
		0, 2, 3
	};

	static const SVertex_Simple globalCubeVertexList[] =
	{
		SVertex_Simple(SC_Vector4f(-0.5f,  0.5f, -0.5f, 1.0f), SC_Vector4f(1.f, 0.0f, 0.0f, 1.0f)),
		SVertex_Simple(SC_Vector4f(0.5f, 0.5f, -0.5f, 1.0f), SC_Vector4f(0.0f, 1.0f, 0.0f, 1.0f)),
		SVertex_Simple(SC_Vector4f(-0.5f, -0.5f, -0.5f, 1.0f), SC_Vector4f(0.0f, 0.0f, 1.0f, 1.0f)),
		SVertex_Simple(SC_Vector4f(0.5f,  -0.5f, -0.5f, 1.0f), SC_Vector4f(1.0f, 1.0f, 1.0f, 1.0f)),

		SVertex_Simple(SC_Vector4f(-0.5f, 0.5f, 0.5f, 1.0f), SC_Vector4f(1.f, 0.0f, 0.0f, 1.0f)),
		SVertex_Simple(SC_Vector4f(0.5f, 0.5f, 0.5f, 1.0f), SC_Vector4f(0.0f, 1.0f, 0.0f, 1.0f)),
		SVertex_Simple(SC_Vector4f(-0.5f, -0.5f, 0.5f, 1.0f), SC_Vector4f(0.0f, 0.0f, 1.0f, 1.0f)),
		SVertex_Simple(SC_Vector4f(0.5f, -0.5f, 0.5f, 1.0f), SC_Vector4f(1.0f, 1.0f, 1.0f, 1.0f))
	};

	static const SVertex_Simple globalSphereVertexList[] =
	{
		SVertex_Simple(SC_Vector4f(0.000000f, -1.000000f, 0.000000f, 1.0f), SC_Vector4f(1.f, 0.0f, 0.0f, 1.0f)),
		SVertex_Simple(SC_Vector4f(0.723607f, -0.447220f, 0.525725f, 1.0f), SC_Vector4f(1.f, 0.0f, 0.0f, 1.0f)),
		SVertex_Simple(SC_Vector4f(-0.276388f, -0.447220f, 0.850649f, 1.0f), SC_Vector4f(1.f, 0.0f, 0.0f, 1.0f)),
		SVertex_Simple(SC_Vector4f(-0.894426f, -0.447216f, 0.000000f, 1.0f), SC_Vector4f(1.f, 0.0f, 0.0f, 1.0f)),
		SVertex_Simple(SC_Vector4f(-0.276388f, -0.447220f, -0.850649f, 1.0f), SC_Vector4f(1.f, 0.0f, 0.0f, 1.0f)),
		SVertex_Simple(SC_Vector4f(0.723607f, -0.447220f, -0.525725f, 1.0f), SC_Vector4f(1.f, 0.0f, 0.0f, 1.0f)),
		SVertex_Simple(SC_Vector4f(0.276388f, 0.447220f, 0.850649f, 1.0f), SC_Vector4f(1.f, 0.0f, 0.0f, 1.0f)),
		SVertex_Simple(SC_Vector4f(-0.723607f, 0.447220f, 0.525725f, 1.0f), SC_Vector4f(1.f, 0.0f, 0.0f, 1.0f)),
		SVertex_Simple(SC_Vector4f(-0.723607f, 0.447220f, -0.525725f, 1.0f), SC_Vector4f(1.f, 0.0f, 0.0f, 1.0f)),
		SVertex_Simple(SC_Vector4f(0.276388f, 0.447220f, -0.850649f, 1.0f), SC_Vector4f(1.f, 0.0f, 0.0f, 1.0f)),
		SVertex_Simple(SC_Vector4f(0.894426f, 0.447216f, 0.000000f, 1.0f), SC_Vector4f(1.f, 0.0f, 0.0f, 1.0f)),
		SVertex_Simple(SC_Vector4f(0.000000f, 1.000000f, 0.000000f, 1.0f), SC_Vector4f(1.f, 0.0f, 0.0f, 1.0f)),
		SVertex_Simple(SC_Vector4f(0.425323f, -0.850654f, 0.309011f, 1.0f), SC_Vector4f(1.f, 0.0f, 0.0f, 1.0f)),
		SVertex_Simple(SC_Vector4f(0.262869f, -0.525738f, 0.809012f, 1.0f), SC_Vector4f(1.f, 0.0f, 0.0f, 1.0f)),
		SVertex_Simple(SC_Vector4f(-0.162456f, -0.850654f, 0.499995f, 1.0f), SC_Vector4f(1.f, 0.0f, 0.0f, 1.0f)),
		SVertex_Simple(SC_Vector4f(0.425323f, - 0.850654f, - 0.309011f, 1.0f), SC_Vector4f(1.f, 0.0f, 0.0f, 1.0f)),
		SVertex_Simple(SC_Vector4f(0.850648f, - 0.525736f, 0.000000f, 1.0f), SC_Vector4f(1.f, 0.0f, 0.0f, 1.0f)),
		SVertex_Simple(SC_Vector4f(-0.688189f, - 0.525736f, 0.499997f, 1.0f), SC_Vector4f(1.f, 0.0f, 0.0f, 1.0f)),
		SVertex_Simple(SC_Vector4f(-0.525730f, - 0.850652f, 0.000000f, 1.0f), SC_Vector4f(1.f, 0.0f, 0.0f, 1.0f)),
		SVertex_Simple(SC_Vector4f(-0.688189f, -0.525736f, -0.499997f, 1.0f), SC_Vector4f(1.f, 0.0f, 0.0f, 1.0f)),
		SVertex_Simple(SC_Vector4f(-0.162456f, -0.850654f, -0.499995f, 1.0f), SC_Vector4f(1.f, 0.0f, 0.0f, 1.0f)),
		SVertex_Simple(SC_Vector4f(0.262869f, -0.525738f, -0.809012f, 1.0f), SC_Vector4f(1.f, 0.0f, 0.0f, 1.0f)),
		SVertex_Simple(SC_Vector4f(0.951058f, 0.000000f, -0.309013f, 1.0f), SC_Vector4f(1.f, 0.0f, 0.0f, 1.0f)),
		SVertex_Simple(SC_Vector4f(0.951058f, 0.000000f, 0.309013f, 1.0f), SC_Vector4f(1.f, 0.0f, 0.0f, 1.0f)),
		SVertex_Simple(SC_Vector4f(0.587786f, 0.000000f, 0.809017f, 1.0f), SC_Vector4f(1.f, 0.0f, 0.0f, 1.0f)),
		SVertex_Simple(SC_Vector4f(0.000000f, 0.000000f, 1.000000f, 1.0f), SC_Vector4f(1.f, 0.0f, 0.0f, 1.0f)),
		SVertex_Simple(SC_Vector4f(-0.587786f, 0.000000f, 0.809017f, 1.0f), SC_Vector4f(1.f, 0.0f, 0.0f, 1.0f)),
		SVertex_Simple(SC_Vector4f(-0.951058f, 0.000000f, 0.309013f, 1.0f), SC_Vector4f(1.f, 0.0f, 0.0f, 1.0f)),
		SVertex_Simple(SC_Vector4f(-0.951058f, 0.000000f, - 0.309013f, 1.0f), SC_Vector4f(1.f, 0.0f, 0.0f, 1.0f)),
		SVertex_Simple(SC_Vector4f(-0.587786f, 0.000000f, - 0.809017f, 1.0f), SC_Vector4f(1.f, 0.0f, 0.0f, 1.0f)),
		SVertex_Simple(SC_Vector4f(0.000000f, 0.000000f, -1.000000f, 1.0f), SC_Vector4f(1.f, 0.0f, 0.0f, 1.0f)),
		SVertex_Simple(SC_Vector4f(0.587786f, 0.000000f, -0.809017f, 1.0f), SC_Vector4f(1.f, 0.0f, 0.0f, 1.0f)),
		SVertex_Simple(SC_Vector4f(0.688189f, 0.525736f, 0.499997f, 1.0f), SC_Vector4f(1.f, 0.0f, 0.0f, 1.0f)),
		SVertex_Simple(SC_Vector4f(-0.262869f, 0.525738f, 0.809012f, 1.0f), SC_Vector4f(1.f, 0.0f, 0.0f, 1.0f)),
		SVertex_Simple(SC_Vector4f(-0.850648f, 0.525736f, 0.000000f, 1.0f), SC_Vector4f(1.f, 0.0f, 0.0f, 1.0f)),
		SVertex_Simple(SC_Vector4f(-0.262869f, 0.525738f, -0.809012f, 1.0f), SC_Vector4f(1.f, 0.0f, 0.0f, 1.0f)),
		SVertex_Simple(SC_Vector4f(0.688189f, 0.525736f, -0.499997f, 1.0f), SC_Vector4f(1.f, 0.0f, 0.0f, 1.0f)),
		SVertex_Simple(SC_Vector4f(0.525730f, 0.850652f, 0.000000f, 1.0f), SC_Vector4f(1.f, 0.0f, 0.0f, 1.0f)),
		SVertex_Simple(SC_Vector4f(0.162456f, 0.850654f, 0.499995f, 1.0f), SC_Vector4f(1.f, 0.0f, 0.0f, 1.0f)),
		SVertex_Simple(SC_Vector4f(-0.425323f, 0.850654f, 0.309011f, 1.0f), SC_Vector4f(1.f, 0.0f, 0.0f, 1.0f)),
		SVertex_Simple(SC_Vector4f(-0.425323f, 0.850654f, -0.309011f, 1.0f), SC_Vector4f(1.f, 0.0f, 0.0f, 1.0f)),
		SVertex_Simple(SC_Vector4f(0.162456f, 0.850654f, -0.499995f, 1.0f), SC_Vector4f(1.f, 0.0f, 0.0f, 1.0f))
	};
	//static const std::vector<unsigned int> globalInvertedSphereIndexList =
	//{
	//	1 , 13, 15,
	//	2 , 13, 17,
	//	1 , 15, 19,
	//	1 , 19, 21,
	//	1 , 21, 16,
	//	2 , 17, 24,
	//	3 , 14, 26,
	//	4 , 18, 28,
	//	5 , 20, 30,
	//	6 , 22, 32,
	//	2 , 24, 25,
	//	3 , 26, 27,
	//	4 , 28, 29,
	//	5 , 30, 31,
	//	6 , 32, 23,
	//	7 , 33, 39,
	//	8 , 34, 40,
	//	9 , 35, 41,
	//	10, 36, 42,
	//	11, 37, 38,
	//	15, 14, 3 ,
	//	15, 13, 14,
	//	13, 2 , 14,
	//	17, 16, 6 ,
	//	17, 13, 16,
	//	13, 1 , 16,
	//	19, 18, 4 ,
	//	19, 15, 18,
	//	15, 3 , 18,
	//	21, 20, 5 ,
	//	21, 19, 20,
	//	19, 4 , 20,
	//	16, 22, 6 ,
	//	16, 21, 22,
	//	21, 5 , 22,
	//	24, 23, 11,
	//	24, 17, 23,
	//	17, 6 , 23,
	//	26, 25, 7 ,
	//	26, 14, 25,
	//	14, 2 , 25,
	//	28, 27, 8 ,
	//	28, 18, 27,
	//	18, 3 , 27,
	//	30, 29, 9 ,
	//	30, 20, 29,
	//	20, 4 , 29,
	//	32, 31, 10,
	//	32, 22, 31,
	//	22, 5 , 31,
	//	25, 33, 7 ,
	//	25, 24, 33,
	//	24, 11, 33,
	//	27, 34, 8 ,
	//	27, 26, 34,
	//	26, 7 , 34,
	//	29, 35, 9 ,
	//	29, 28, 35,
	//	28, 8 , 35,
	//	31, 36, 10,
	//	31, 30, 36,
	//	30, 9 , 36,
	//	23, 37, 11,
	//	23, 32, 37,
	//	32, 10, 37,
	//	39, 38, 12,
	//	39, 33, 38,
	//	33, 11, 38,
	//	40, 39, 12,
	//	40, 34, 39,
	//	34, 7 , 39,
	//	41, 40, 12,
	//	41, 35, 40,
	//	35, 8 , 40,
	//	42, 41, 12,
	//	42, 36, 41,
	//	36, 9 , 41,
	//	38, 42, 12,
	//	38, 37, 42,
	//	37, 10, 42
	//};
	//static const std::vector<unsigned int> globalSphereIndexList =
	//{
	//	1 , 13, 15,
	//	2 , 13, 17,
	//	1 , 15, 19,
	//	1 , 19, 21,
	//	1 , 21, 16,
	//	2 , 17, 24,
	//	3 , 14, 26,
	//	4 , 18, 28,
	//	5 , 20, 30,
	//	6 , 22, 32,
	//	2 , 24, 25,
	//	3 , 26, 27,
	//	4 , 28, 29,
	//	5 , 30, 31,
	//	6 , 32, 23,
	//	7 , 33, 39,
	//	8 , 34, 40,
	//	9 , 35, 41,
	//	10, 36, 42,
	//	11, 37, 38,
	//	15, 14, 3 ,
	//	15, 13, 14,
	//	13, 2 , 14,
	//	17, 16, 6 ,
	//	17, 13, 16,
	//	13, 1 , 16,
	//	19, 18, 4 ,
	//	19, 15, 18,
	//	15, 3 , 18,
	//	21, 20, 5 ,
	//	21, 19, 20,
	//	19, 4 , 20,
	//	16, 22, 6 ,
	//	16, 21, 22,
	//	21, 5 , 22,
	//	24, 23, 11,
	//	24, 17, 23,
	//	17, 6 , 23,
	//	26, 25, 7 ,
	//	26, 14, 25,
	//	14, 2 , 25,
	//	28, 27, 8 ,
	//	28, 18, 27,
	//	18, 3 , 27,
	//	30, 29, 9 ,
	//	30, 20, 29,
	//	20, 4 , 29,
	//	32, 31, 10,
	//	32, 22, 31,
	//	22, 5 , 31,
	//	25, 33, 7 ,
	//	25, 24, 33,
	//	24, 11, 33,
	//	27, 34, 8 ,
	//	27, 26, 34,
	//	26, 7 , 34,
	//	29, 35, 9 ,
	//	29, 28, 35,
	//	28, 8 , 35,
	//	31, 36, 10,
	//	31, 30, 36,
	//	30, 9 , 36,
	//	23, 37, 11,
	//	23, 32, 37,
	//	32, 10, 37,
	//	39, 38, 12,
	//	39, 33, 38,
	//	33, 11, 38,
	//	40, 39, 12,
	//	40, 34, 39,
	//	34, 7 , 39,
	//	41, 40, 12,
	//	41, 35, 40,
	//	35, 8 , 40,
	//	42, 41, 12,
	//	42, 36, 41,
	//	36, 9 , 41,
	//	38, 42, 12,
	//	38, 37, 42,
	//	37, 10, 42
	//};
	static const unsigned int globalInvertedCubeIndexList[] =
	{
		2, 1, 0,    // side 1
		3, 1, 2,
		6, 0, 4,    // side 2
		2, 0, 6,
		6, 5, 7,    // side 3
		4, 5, 6,
		7, 1, 3,    // side 4
		5, 1, 7,
		0, 5, 4,    // side 5
		1, 5, 0,
		2, 7, 3,    // side 6
		6, 7, 2,
	};
	static const unsigned int globalCubeIndexList[] =
	{
		0, 1, 2,   // side 1
		2, 1, 3,
		4, 0, 6,   // side 2
		6, 0, 2,
		7, 5, 6,   // side 3
		6, 5, 4,
		3, 1, 7,   // side 4
		7, 1, 5,
		4, 5, 0,   // side 5
		0, 5, 1,
		3, 7, 2,   // side 6
		2, 7, 6,
	};


}