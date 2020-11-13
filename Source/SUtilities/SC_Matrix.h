#pragma once
#ifndef SC_MATRIX_H
#define SC_MATRIX_H

namespace Shift
{
	class SC_Matrix44;
	class SC_Matrix33;
}
#include "SC_Vector.h"
#include "SC_Matrix33.hpp"
#include "SC_Matrix44.hpp"

#if IS_X86_PLATFORM
#define USE_SIMD_OPERATIONS
#endif

namespace Shift
{

	// --------------------------------------------------------------------------
	// Vector3 * Matrix33
	// --------------------------------------------------------------------------

	template <class T>
	SC_Vector3<T> operator*(const SC_Vector3<T>& v, const SC_Matrix33& aMatrix)
	{
		return SC_Vector3<T>(
			(T)((float)v.x * aMatrix.myAxisX.x + (float)v.y * aMatrix.myAxisY.x + (float)v.z * aMatrix.myAxisZ.x),
			(T)((float)v.x * aMatrix.myAxisX.y + (float)v.y * aMatrix.myAxisY.y + (float)v.z * aMatrix.myAxisZ.y),
			(T)((float)v.x * aMatrix.myAxisX.z + (float)v.y * aMatrix.myAxisY.z + (float)v.z * aMatrix.myAxisZ.z));
	}

	template <class T>
	SC_Vector3<T>& operator*=(SC_Vector3<T>& v, const SC_Matrix33& aMatrix)
	{
		v = v * aMatrix;
		return v;
	}

	// --------------------------------------------------------------------------
	// Vector3 * Matrix44
	// --------------------------------------------------------------------------

	template <class T>
	inline SC_Vector3<T> operator*(const SC_Vector3<T>& aVector, const SC_Matrix44& aMatrix)
	{
		return SC_Vector3<T>(
			(T)((float)aVector.x * aMatrix.myAxisX.x + (float)aVector.y * aMatrix.myAxisY.x + (float)aVector.z * aMatrix.myAxisZ.x + aMatrix.myPosition.x),
			(T)((float)aVector.x * aMatrix.myAxisX.y + (float)aVector.y * aMatrix.myAxisY.y + (float)aVector.z * aMatrix.myAxisZ.y + aMatrix.myPosition.y),
			(T)((float)aVector.x * aMatrix.myAxisX.z + (float)aVector.y * aMatrix.myAxisY.z + (float)aVector.z * aMatrix.myAxisZ.z + aMatrix.myPosition.z));
	}

	template <class T>
	inline SC_Vector3<T>& operator*=(SC_Vector3<T>& aVector, const SC_Matrix44& aMatrix)
	{
		aVector = aVector * aMatrix;
		return aVector;
	}

	inline void SC_Transform(SC_Vector3f& aResult, const SC_Vector3f& aVector, const SC_Matrix44& aMatrix)
	{
		aResult = aVector * aMatrix;
	}

	// --------------------------------------------------------------------------
	// Vector4 * Matrix44
	// --------------------------------------------------------------------------

	inline SC_Vector4f operator*(const SC_Vector4f& aVector, const SC_Matrix44& aMatrix)
	{
#ifdef USE_SIMD_OPERATIONS

		const __m128 X = aMatrix.myAxisX.myXMM;
		const __m128 Y = aMatrix.myAxisY.myXMM;
		const __m128 Z = aMatrix.myAxisZ.myXMM;
		const __m128 P = aMatrix.myPosition.myXMM;

		const __m128 t = aVector.myXMM;

		SC_Vector4f res(
					SC_MulAddSIMD(_mm_shuffle_ps(t, t, _MM_SHUFFLE(0, 0, 0, 0)), X,				// res = aVector.x * aMatrix.myAxisX
					SC_MulAddSIMD(_mm_shuffle_ps(t, t, _MM_SHUFFLE(1, 1, 1, 1)), Y,				//	   + aVector.y * aMatrix.myAxisY
					SC_MulAddSIMD(_mm_shuffle_ps(t, t, _MM_SHUFFLE(2, 2, 2, 2)), Z,				//	   + aVector.z * aMatrix.myAxisZ
					_mm_mul_ps(_mm_shuffle_ps(t, t, _MM_SHUFFLE(3, 3, 3, 3)), P)))));		//	   + aVector.w * aMatrix.myPosition

		return res;

#else

		return MC_Vector4f(
			(aVector.x * aMatrix.myAxisX.x + aVector.y * aMatrix.myAxisY.x + aVector.z * aMatrix.myAxisZ.x + aVector.w * aMatrix.myPosition.x),
			(aVector.x * aMatrix.myAxisX.y + aVector.y * aMatrix.myAxisY.y + aVector.z * aMatrix.myAxisZ.y + aVector.w * aMatrix.myPosition.y),
			(aVector.x * aMatrix.myAxisX.z + aVector.y * aMatrix.myAxisY.z + aVector.z * aMatrix.myAxisZ.z + aVector.w * aMatrix.myPosition.z),
			(aVector.x * aMatrix.myAxisX.w + aVector.y * aMatrix.myAxisY.w + aVector.z * aMatrix.myAxisZ.w + aVector.w * aMatrix.myPosition.w));

#endif
	}

	template <class T>
	inline SC_Vector4<T> operator*(const SC_Vector4<T>& aVector, const SC_Matrix44& aMatrix)
	{
		return SC_Vector4<T>(
			(T)((float)aVector.x * aMatrix.myAxisX.x + (float)aVector.y * aMatrix.myAxisY.x + (float)aVector.z * aMatrix.myAxisZ.x + (float)aVector.w * aMatrix.myPosition.x),
			(T)((float)aVector.x * aMatrix.myAxisX.y + (float)aVector.y * aMatrix.myAxisY.y + (float)aVector.z * aMatrix.myAxisZ.y + (float)aVector.w * aMatrix.myPosition.y),
			(T)((float)aVector.x * aMatrix.myAxisX.z + (float)aVector.y * aMatrix.myAxisY.z + (float)aVector.z * aMatrix.myAxisZ.z + (float)aVector.w * aMatrix.myPosition.z),
			(T)((float)aVector.x * aMatrix.myAxisX.w + (float)aVector.y * aMatrix.myAxisY.w + (float)aVector.z * aMatrix.myAxisZ.w + (float)aVector.w * aMatrix.myPosition.w));
	}

	template <>
	inline SC_Vector4<float> operator*(const SC_Vector4<float>& aVector, const SC_Matrix44& aMatrix)
	{
		return aVector * aMatrix;
	}

	template <class T>
	inline SC_Vector4<T>& operator*=(SC_Vector4<T>& aVector, const SC_Matrix44& aMatrix)
	{
		aVector = aVector * aMatrix;
		return aVector;
	}

	template <>
	inline SC_Vector4<float>& operator*=(SC_Vector4<float>& aVector, const SC_Matrix44& aMatrix)
	{
		aVector = aVector * aMatrix;
		return aVector;
	}

	inline SC_Vector4f& operator*=(SC_Vector4f& aVector, const SC_Matrix44& aMatrix)
	{
		aVector = aVector * aMatrix;
		return aVector;
	}

	inline void SC_Transform(SC_Vector4f& aResult, const SC_Vector4f& aVector, const SC_Matrix44& aMatrix)
	{
		aResult = aVector * aMatrix;
	}
}

#endif