#pragma once

#ifndef SC_MATRIX_H
#error You should include Matrix.h instead!
#endif

/*

	USING ROW-MAJOR LAYOUT

*/

#if IS_X86_PLATFORM
#define USE_SIMD_OPERATIONS
#endif

#include "SC_Vector.h"
#include "SC_Hash.h"

namespace Shift
{
	class SC_Matrix44
	{
	private:
		static constexpr uint MatrixMaxValues = 16;
	public:
		inline SC_Matrix44();
		inline ~SC_Matrix44();
		inline SC_Matrix44(const SC_NoInit) {};
		inline SC_Matrix44(const float& aValue);

		inline SC_Matrix44(const float a11, const float& a12, const float& a13, const float& a14,
			const float& a21, const float& a22, const float& a23, const float& a24,
			const float& a31, const float& a32, const float& a33, const float& a34,
			const float& a41, const float& a42, const float& a43, const float& a44);

		inline SC_Matrix44(const SC_Matrix44& aOther)
			: myAxisX(aOther.myAxisX), myAxisY(aOther.myAxisY), myAxisZ(aOther.myAxisZ), myPosition(aOther.myPosition)
		{};

		inline const SC_Vector3f GetPosition() const;
		inline void SetPosition(const SC_Vector3f& aPosition);

		inline SC_Matrix33 Get33() const;
		inline void Set33(const SC_Matrix33& aMat);

		inline float& At(uint aRow, uint aCol);
		inline const float& At(uint aRow, uint aCol) const;
		inline void SetAt(uint aRow, uint aCol, const float& aValue);
		inline void SetAt(uint aIndex, const float& aValue); 

		inline float& GetRowCol(uint aRow, uint aCol);
		inline const float& GetRowCol(uint aRow, uint aCol) const;
		inline void SetRowCol(const uint aRow, const uint aCol, const float aVal);

		inline float& operator[](const int aIndex);
		inline const float& operator[](const int aIndex) const; 
		inline const SC_Matrix44 operator*(const SC_Matrix44& aMatrix) const;
		inline SC_Matrix44& operator*=(const SC_Matrix44& aMatrix);
		inline bool operator==(const SC_Matrix44& aOther)
		{
			if (my11 == aOther.my11 && my12 == aOther.my12 && my13 == aOther.my13 && my14 == aOther.my14 &&
				my21 == aOther.my21 && my22 == aOther.my22 && my23 == aOther.my23 && my24 == aOther.my24 &&
				my31 == aOther.my31 && my32 == aOther.my32 && my33 == aOther.my33 && my34 == aOther.my34 &&
				my41 == aOther.my41 && my42 == aOther.my42 && my43 == aOther.my43 && my44 == aOther.my44)
			{
				return true;
			}
			return false;
		}

		inline void Transpose();
		inline void RotateAroundX(const float aAngle);
		inline void RotateAroundY(const float aAngle); 
		inline void Scale(const SC_Vector3f& aScale);
		inline void SetDirection(const SC_Vector3f& aDirection, const SC_Vector3f& aUp = SC_Vector3f(0.0f, 1.0f, 0.0f));
		
		inline SC_Matrix44 ToLocal() const;

		// Only for Transformations
		inline bool InverseFast();
		inline bool GetInverseFast(SC_Matrix44& aOutMatrix) const;

		inline bool Inverse();
		inline bool GetInverse(SC_Matrix44& aOutMatrix) const;

		inline const SC_Vector3f GetXAxis() const { return myAxisX.XYZ(); }
		inline const SC_Vector3f GetYAxis() const { return myAxisY.XYZ(); }
		inline const SC_Vector3f GetZAxis() const { return myAxisZ.XYZ(); }

		inline float GetMinor(unsigned int aRow, unsigned int aColumn) const;

		static SC_Matrix44 CreateRotateAroundX(const float& aAngleInRadius);
		static SC_Matrix44 CreateRotateAroundY(const float& aAngleInRadius);
	public:

		union 
		{
			float _dataRaw[MatrixMaxValues];
			struct  
			{
				float	my11, my12, my13, my14,
						my21, my22, my23, my24,
						my31, my32, my33, my34,
						my41, my42, my43, my44;
			};
			struct
			{
				SC_Vector4f myAxisX;
				SC_Vector4f myAxisY;
				SC_Vector4f myAxisZ;
				SC_Vector4f myPosition;
			};
		};
	};
	SC_ENABLE_RAW_HASHING(SC_Matrix44);

	inline const SC_Matrix44 SC_Matrix44::operator*(const SC_Matrix44& aMatrix) const
	{
		SC_Matrix44 newMatrix(SC_NO_INIT);

#ifdef USE_SIMD_OPERATIONS

		const __m128 X = aMatrix.myAxisX.myXMM;
		const __m128 Y = aMatrix.myAxisY.myXMM;
		const __m128 Z = aMatrix.myAxisZ.myXMM;
		const __m128 P = aMatrix.myPosition.myXMM;
		__m128 temp;

		temp = myAxisX.myXMM;
		newMatrix.myAxisX.myXMM =
			SC_MulAddSIMD(_mm_shuffle_ps(temp, temp, _MM_SHUFFLE(0, 0, 0, 0)), X,
			SC_MulAddSIMD(_mm_shuffle_ps(temp, temp, _MM_SHUFFLE(1, 1, 1, 1)), Y,
			SC_MulAddSIMD(_mm_shuffle_ps(temp, temp, _MM_SHUFFLE(2, 2, 2, 2)), Z,
			_mm_mul_ps(_mm_shuffle_ps(temp, temp, _MM_SHUFFLE(3, 3, 3, 3)), P))));

		temp = myAxisY.myXMM;
		newMatrix.myAxisY.myXMM =
			SC_MulAddSIMD(_mm_shuffle_ps(temp, temp, _MM_SHUFFLE(0, 0, 0, 0)), X,
			SC_MulAddSIMD(_mm_shuffle_ps(temp, temp, _MM_SHUFFLE(1, 1, 1, 1)), Y,
			SC_MulAddSIMD(_mm_shuffle_ps(temp, temp, _MM_SHUFFLE(2, 2, 2, 2)), Z,
			_mm_mul_ps(_mm_shuffle_ps(temp, temp, _MM_SHUFFLE(3, 3, 3, 3)), P))));

		temp = myAxisZ.myXMM;
		newMatrix.myAxisZ.myXMM =
			SC_MulAddSIMD(_mm_shuffle_ps(temp, temp, _MM_SHUFFLE(0, 0, 0, 0)), X,
			SC_MulAddSIMD(_mm_shuffle_ps(temp, temp, _MM_SHUFFLE(1, 1, 1, 1)), Y,
			SC_MulAddSIMD(_mm_shuffle_ps(temp, temp, _MM_SHUFFLE(2, 2, 2, 2)), Z,
			_mm_mul_ps(_mm_shuffle_ps(temp, temp, _MM_SHUFFLE(3, 3, 3, 3)), P))));

		temp = myPosition.myXMM;
		newMatrix.myPosition.myXMM =
			SC_MulAddSIMD(_mm_shuffle_ps(temp, temp, _MM_SHUFFLE(0, 0, 0, 0)), X,
			SC_MulAddSIMD(_mm_shuffle_ps(temp, temp, _MM_SHUFFLE(1, 1, 1, 1)), Y,
			SC_MulAddSIMD(_mm_shuffle_ps(temp, temp, _MM_SHUFFLE(2, 2, 2, 2)), Z,
			_mm_mul_ps(_mm_shuffle_ps(temp, temp, _MM_SHUFFLE(3, 3, 3, 3)), P))));

#else
		newMatrix[0] = _dataRaw[0] * aMatrix[0] + _dataRaw[1] * aMatrix[4] + _dataRaw[2] * aMatrix[8] + _dataRaw[3] * aMatrix[12];
		newMatrix[1] = _dataRaw[0] * aMatrix[1] + _dataRaw[1] * aMatrix[5] + _dataRaw[2] * aMatrix[9] + _dataRaw[3] * aMatrix[13];
		newMatrix[2] = _dataRaw[0] * aMatrix[2] + _dataRaw[1] * aMatrix[6] + _dataRaw[2] * aMatrix[10] + _dataRaw[3] * aMatrix[14];
		newMatrix[3] = _dataRaw[0] * aMatrix[3] + _dataRaw[1] * aMatrix[7] + _dataRaw[2] * aMatrix[11] + _dataRaw[3] * aMatrix[15];

		newMatrix[4] = _dataRaw[4] * aMatrix[0] + _dataRaw[5] * aMatrix[4] + _dataRaw[6] * aMatrix[8] + _dataRaw[7] * aMatrix[12];
		newMatrix[5] = _dataRaw[4] * aMatrix[1] + _dataRaw[5] * aMatrix[5] + _dataRaw[6] * aMatrix[9] + _dataRaw[7] * aMatrix[13];
		newMatrix[6] = _dataRaw[4] * aMatrix[2] + _dataRaw[5] * aMatrix[6] + _dataRaw[6] * aMatrix[10] + _dataRaw[7] * aMatrix[14];
		newMatrix[7] = _dataRaw[4] * aMatrix[3] + _dataRaw[5] * aMatrix[7] + _dataRaw[6] * aMatrix[11] + _dataRaw[7] * aMatrix[15];

		newMatrix[8] = _dataRaw[8] * aMatrix[0] + _dataRaw[9] * aMatrix[4] + _dataRaw[10] * aMatrix[8] + _dataRaw[11] * aMatrix[12];
		newMatrix[9] = _dataRaw[8] * aMatrix[1] + _dataRaw[9] * aMatrix[5] + _dataRaw[10] * aMatrix[9] + _dataRaw[11] * aMatrix[13];
		newMatrix[10] = _dataRaw[8] * aMatrix[2] + _dataRaw[9] * aMatrix[6] + _dataRaw[10] * aMatrix[10] + _dataRaw[11] * aMatrix[14];
		newMatrix[11] = _dataRaw[8] * aMatrix[3] + _dataRaw[9] * aMatrix[7] + _dataRaw[10] * aMatrix[11] + _dataRaw[11] * aMatrix[15];

		newMatrix[12] = _dataRaw[12] * aMatrix[0] + _dataRaw[13] * aMatrix[4] + _dataRaw[14] * aMatrix[8] + _dataRaw[15] * aMatrix[12];
		newMatrix[13] = _dataRaw[12] * aMatrix[1] + _dataRaw[13] * aMatrix[5] + _dataRaw[14] * aMatrix[9] + _dataRaw[15] * aMatrix[13];
		newMatrix[14] = _dataRaw[12] * aMatrix[2] + _dataRaw[13] * aMatrix[6] + _dataRaw[14] * aMatrix[10] + _dataRaw[15] * aMatrix[14];
		newMatrix[15] = _dataRaw[12] * aMatrix[3] + _dataRaw[13] * aMatrix[7] + _dataRaw[14] * aMatrix[11] + _dataRaw[15] * aMatrix[15];
#endif
		return newMatrix;
	}

	inline SC_Matrix44& SC_Matrix44::operator*=(const SC_Matrix44& aMatrix)
	{
		float tempMatrix[16]{ _dataRaw[0], _dataRaw[1], _dataRaw[2], _dataRaw[3]
								   , _dataRaw[4], _dataRaw[5], _dataRaw[6], _dataRaw[7]
								   , _dataRaw[8], _dataRaw[9], _dataRaw[10], _dataRaw[11]
								   , _dataRaw[12], _dataRaw[13], _dataRaw[14], _dataRaw[15] };

		_dataRaw[0] = (tempMatrix[0] * aMatrix[0]) + (tempMatrix[1] * aMatrix[4]) + (tempMatrix[2] * aMatrix[8]) + (tempMatrix[3] * aMatrix[12]);
		_dataRaw[1] = (tempMatrix[0] * aMatrix[1]) + (tempMatrix[1] * aMatrix[5]) + (tempMatrix[2] * aMatrix[9]) + (tempMatrix[3] * aMatrix[13]);
		_dataRaw[2] = (tempMatrix[0] * aMatrix[2]) + (tempMatrix[1] * aMatrix[6]) + (tempMatrix[2] * aMatrix[10]) + (tempMatrix[3] * aMatrix[14]);
		_dataRaw[3] = (tempMatrix[0] * aMatrix[3]) + (tempMatrix[1] * aMatrix[7]) + (tempMatrix[2] * aMatrix[11]) + (tempMatrix[3] * aMatrix[15]);
		
		_dataRaw[4] = (tempMatrix[4] * aMatrix[0]) + (tempMatrix[5] * aMatrix[4]) + (tempMatrix[6] * aMatrix[8]) + (tempMatrix[7] * aMatrix[12]);
		_dataRaw[5] = (tempMatrix[4] * aMatrix[1]) + (tempMatrix[5] * aMatrix[5]) + (tempMatrix[6] * aMatrix[9]) + (tempMatrix[7] * aMatrix[13]);
		_dataRaw[6] = (tempMatrix[4] * aMatrix[2]) + (tempMatrix[5] * aMatrix[6]) + (tempMatrix[6] * aMatrix[10]) + (tempMatrix[7] * aMatrix[14]);
		_dataRaw[7] = (tempMatrix[4] * aMatrix[3]) + (tempMatrix[5] * aMatrix[7]) + (tempMatrix[6] * aMatrix[11]) + (tempMatrix[7] * aMatrix[15]);
		
		_dataRaw[8] = (tempMatrix[8] * aMatrix[0]) + (tempMatrix[9] * aMatrix[4]) + (tempMatrix[10] * aMatrix[8]) + (tempMatrix[11] * aMatrix[12]);
		_dataRaw[9] = (tempMatrix[8] * aMatrix[1]) + (tempMatrix[9] * aMatrix[5]) + (tempMatrix[10] * aMatrix[9]) + (tempMatrix[11] * aMatrix[13]);
		_dataRaw[10] = (tempMatrix[8] * aMatrix[2]) + (tempMatrix[9] * aMatrix[6]) + (tempMatrix[10] * aMatrix[10]) + (tempMatrix[11] * aMatrix[14]);
		_dataRaw[11] = (tempMatrix[8] * aMatrix[3]) + (tempMatrix[9] * aMatrix[7]) + (tempMatrix[10] * aMatrix[11]) + (tempMatrix[11] * aMatrix[15]);
		
		_dataRaw[12] = (tempMatrix[12] * aMatrix[0]) + (tempMatrix[13] * aMatrix[4]) + (tempMatrix[14] * aMatrix[8]) + (tempMatrix[15] * aMatrix[12]);
		_dataRaw[13] = (tempMatrix[12] * aMatrix[1]) + (tempMatrix[13] * aMatrix[5]) + (tempMatrix[14] * aMatrix[9]) + (tempMatrix[15] * aMatrix[13]);
		_dataRaw[14] = (tempMatrix[12] * aMatrix[2]) + (tempMatrix[13] * aMatrix[6]) + (tempMatrix[14] * aMatrix[10]) + (tempMatrix[15] * aMatrix[14]);
		_dataRaw[15] = (tempMatrix[12] * aMatrix[3]) + (tempMatrix[13] * aMatrix[7]) + (tempMatrix[14] * aMatrix[11]) + (tempMatrix[15] * aMatrix[15]);

		return *this;
	}

	inline SC_Matrix44::SC_Matrix44(const float& aValue)
		:	my11(aValue), my12(aValue), my13(aValue), my14(aValue),
			my21(aValue), my22(aValue), my23(aValue), my24(aValue),
			my31(aValue), my32(aValue), my33(aValue), my34(aValue),
			my41(aValue), my42(aValue), my43(aValue), my44(aValue)
	{
	}
	inline SC_Matrix44::SC_Matrix44(const float a11, const float& a12, const float& a13, const float& a14,
		const float& a21, const float& a22, const float& a23, const float& a24,
		const float& a31, const float& a32, const float& a33, const float& a34,
		const float& a41, const float& a42, const float& a43, const float& a44)
		:	my11(a11), my12(a12), my13(a13), my14(a14),
			my21(a21), my22(a22), my23(a23), my24(a24),
			my31(a31), my32(a32), my33(a33), my34(a34),
			my41(a41), my42(a42), my43(a43), my44(a44)
	{
	}

	inline SC_Matrix44::SC_Matrix44()
		:	my11(1), my12(0), my13(0), my14(0),
			my21(0), my22(1), my23(0), my24(0),
			my31(0), my32(0), my33(1), my34(0),
			my41(0), my42(0), my43(0), my44(1)
	{
	}

	inline SC_Matrix44::~SC_Matrix44()
	{
	}

	inline const SC_Vector3f SC_Matrix44::GetPosition() const
	{
		return SC_Vector3f(my41, my42, my43);
	}

	inline bool SC_Matrix44::InverseFast()
	{
		return GetInverseFast(*this);
	}

	inline bool SC_Matrix44::GetInverseFast(SC_Matrix44& aOutMatrix) const
	{
		float scaleX = myAxisX.Length2_3D();
		float scaleY = myAxisY.Length2_3D();
		float scaleZ = myAxisZ.Length2_3D();
		bool result = (scaleX > 0.000001f) && (scaleY > 0.000001f) && (scaleZ > 0.000001f) && (myPosition.w == 1);

		if (result)
		{
			scaleX = 1.f / scaleX;
			scaleY = 1.f / scaleY;
			scaleZ = 1.f / scaleZ;

			aOutMatrix = SC_Matrix44(myAxisX.x * scaleX, myAxisY.x * scaleY, myAxisZ.x * scaleZ, 0.f,
				myAxisX.y * scaleX, myAxisY.y * scaleY, myAxisZ.y * scaleZ, 0.f,
				myAxisX.z * scaleX, myAxisY.z * scaleY, myAxisZ.z * scaleZ, 0.f,
				-(myPosition.x * myAxisX.x + myPosition.y * myAxisX.y + myPosition.z * myAxisX.z) * scaleX,
				-(myPosition.x * myAxisY.x + myPosition.y * myAxisY.y + myPosition.z * myAxisY.z) * scaleY,
				-(myPosition.x * myAxisZ.x + myPosition.y * myAxisZ.y + myPosition.z * myAxisZ.z) * scaleZ,
				1.f);
		}

		return result;
	}

	inline void SC_Matrix44::SetPosition(const SC_Vector3f& aPosition)
	{
		myPosition.x = aPosition.x;
		myPosition.y = aPosition.y;
		myPosition.z = aPosition.z;
	}

	inline SC_Matrix33 SC_Matrix44::Get33() const
	{
		return SC_Matrix33(myAxisX, myAxisY, myAxisZ);
	}

	inline void SC_Matrix44::Set33(const SC_Matrix33& aMat)
	{
		myAxisX = aMat.myAxisX;
		myAxisY = aMat.myAxisY;
		myAxisZ = aMat.myAxisZ;
		myAxisX.w = 0.0f;
		myAxisY.w = 0.0f;
		myAxisZ.w = 0.0f;
	}

	inline void SC_Matrix44::SetAt(uint aRow, uint aCol, const float& aValue)
	{
		uint idx = (aRow * 4) + aCol;
		SetAt(idx, aValue);
	}
	inline void SC_Matrix44::SetAt(uint aIndex, const float& aValue)
	{
		_dataRaw[aIndex] = aValue;
	}

	inline float& SC_Matrix44::At(uint aRow, uint aCol)
	{
		return _dataRaw[aRow * 4 + aCol];
	}

	inline const float& SC_Matrix44::At(uint aRow, uint aCol) const
	{
		return _dataRaw[aRow * 4 + aCol];
	}

	inline float& SC_Matrix44::GetRowCol(uint aRow, uint aCol)
	{
		return _dataRaw[aRow * 4 + aCol];
	}

	inline const float& SC_Matrix44::GetRowCol(uint aRow, uint aCol) const
	{
		return _dataRaw[aRow * 4 + aCol];
	}

	inline void SC_Matrix44::SetRowCol(const uint aRow, const uint aCol, const float aVal)
	{
		_dataRaw[aRow * 4 + aCol] = aVal;
	}

	inline float & SC_Matrix44::operator[](const int aIndex)
	{
		return _dataRaw[aIndex];
	}

	inline const float & SC_Matrix44::operator[](const int aIndex) const
	{
		return _dataRaw[aIndex];
	}

	inline void SC_Matrix44::Transpose()
	{
		SC_Matrix44 temp(*this);

		_dataRaw[0] = temp._dataRaw[0];
		_dataRaw[1] = temp._dataRaw[4];
		_dataRaw[2] = temp._dataRaw[8];
		_dataRaw[3] = temp._dataRaw[12];
		_dataRaw[4] = temp._dataRaw[1];
		_dataRaw[5] = temp._dataRaw[5];
		_dataRaw[6] = temp._dataRaw[9];
		_dataRaw[7] = temp._dataRaw[13];
		_dataRaw[8] = temp._dataRaw[2];
		_dataRaw[9] = temp._dataRaw[6];
		_dataRaw[10] = temp._dataRaw[10];
		_dataRaw[11] = temp._dataRaw[14];
		_dataRaw[12] = temp._dataRaw[3];
		_dataRaw[13] = temp._dataRaw[7];
		_dataRaw[14] = temp._dataRaw[11];
		_dataRaw[15] = temp._dataRaw[15];
	}

	inline void SC_Matrix44::RotateAroundY(const float aAngle)
	{
		SC_Matrix44& _this = *this;
		SC_Vector3f pos = GetPosition();
		_this = _this * CreateRotateAroundY(aAngle);
		SetPosition(pos);
	}

	inline void SC_Matrix44::Scale(const SC_Vector3f& aScale)
	{
		SC_Matrix44 scale;
		scale[0] = aScale.x;
		scale[5] = aScale.y;
		scale[10] = aScale.z;

		const SC_Vector3f pos(GetPosition());
		*this *= scale;
		SetPosition(pos);
	}

	inline void SC_Matrix44::SetDirection(const SC_Vector3f& aDirection, const SC_Vector3f& aUp)
	{
		SC_Vector3f tempVector = aUp;
		SC_Vector3f direction = aDirection;
		direction.Normalize();
		myAxisZ.x = direction.x;
		myAxisZ.y = direction.y;
		myAxisZ.z = direction.z;

		tempVector = tempVector.Cross(GetZAxis().GetNormalized());
		myAxisX.x = tempVector.x;
		myAxisX.y = tempVector.y;
		myAxisX.z = tempVector.z;

		tempVector = GetZAxis().Cross(GetXAxis().GetNormalized());
		myAxisY.x = tempVector.x;
		myAxisY.y = tempVector.y;
		myAxisY.z = tempVector.z;
	}

	inline SC_Matrix44 SC_Matrix44::ToLocal() const
	{
		return SC_Matrix44(
			myAxisX.x, myAxisY.x, myAxisZ.x, myAxisX.w,
			myAxisX.y, myAxisY.y, myAxisZ.y, myAxisX.w,
			myAxisX.z, myAxisY.z, myAxisZ.z, myAxisX.w,
			-(myPosition.x * myAxisX.x + myPosition.y * myAxisX.y + myPosition.z * myAxisX.z),
			-(myPosition.x * myAxisY.x + myPosition.y * myAxisY.y + myPosition.z * myAxisY.z),
			-(myPosition.x * myAxisZ.x + myPosition.y * myAxisZ.y + myPosition.z * myAxisZ.z),
			myPosition.w
			);
	}

	inline void SC_Matrix44::RotateAroundX(const float aAngle)
	{
		SC_Matrix44& _this = *this;
		SC_Vector3f pos = GetPosition();
		_this = _this * CreateRotateAroundX(aAngle);
		SetPosition(pos);
	}

	inline SC_Matrix44 SC_Matrix44::CreateRotateAroundX(const float& aAngleInRadius)
	{
		float cosinus = cosf(aAngleInRadius);
		float sinus = sinf(aAngleInRadius);

		SC_Matrix44 rotationMatrix;
		rotationMatrix._dataRaw[0] = 1;
		rotationMatrix._dataRaw[1] = 0;
		rotationMatrix._dataRaw[2] = 0;
		rotationMatrix._dataRaw[3] = 0;
		rotationMatrix._dataRaw[4] = 0;
		rotationMatrix._dataRaw[5] = cosinus;
		rotationMatrix._dataRaw[6] = sinus;
		rotationMatrix._dataRaw[7] = 0;
		rotationMatrix._dataRaw[8] = 0;
		rotationMatrix._dataRaw[9] = -sinus;
		rotationMatrix._dataRaw[10] = cosinus;
		rotationMatrix._dataRaw[11] = 0;
		rotationMatrix._dataRaw[12] = 0;
		rotationMatrix._dataRaw[13] = 0;
		rotationMatrix._dataRaw[14] = 0;
		rotationMatrix._dataRaw[15] = 1;

		return rotationMatrix;
	}

	inline SC_Matrix44 SC_Matrix44::CreateRotateAroundY(const float& aAngleInRadius)
	{
		float cosinus = cosf(aAngleInRadius);
		float sinus = sinf(aAngleInRadius);

		SC_Matrix44 rotationMatrix;
		rotationMatrix._dataRaw[0] = cosinus;
		rotationMatrix._dataRaw[1] = 0;
		rotationMatrix._dataRaw[2] = -sinus;
		rotationMatrix._dataRaw[3] = 0;
		rotationMatrix._dataRaw[4] = 0;
		rotationMatrix._dataRaw[5] = 1;
		rotationMatrix._dataRaw[6] = 0;
		rotationMatrix._dataRaw[7] = 0;
		rotationMatrix._dataRaw[8] = sinus;
		rotationMatrix._dataRaw[9] = 0;
		rotationMatrix._dataRaw[10] = cosinus;
		rotationMatrix._dataRaw[11] = 0;
		rotationMatrix._dataRaw[12] = 0;
		rotationMatrix._dataRaw[13] = 0;
		rotationMatrix._dataRaw[14] = 0;
		rotationMatrix._dataRaw[15] = 1;

		return rotationMatrix;
	}

	inline bool SC_Matrix44::GetInverse(SC_Matrix44& aOutMatrix) const
	{
		float inv[16], m[16], det;

		for (int j = 0; j < 4; ++j)
			for (int k = 0; k < 4; ++k)
				m[j * 4 + k] = this->At(j, k);

		inv[0] = m[5] * m[10] * m[15] -
			m[5] * m[11] * m[14] -
			m[9] * m[6] * m[15] +
			m[9] * m[7] * m[14] +
			m[13] * m[6] * m[11] -
			m[13] * m[7] * m[10];

		inv[4] = -m[4] * m[10] * m[15] +
			m[4] * m[11] * m[14] +
			m[8] * m[6] * m[15] -
			m[8] * m[7] * m[14] -
			m[12] * m[6] * m[11] +
			m[12] * m[7] * m[10];

		inv[8] = m[4] * m[9] * m[15] -
			m[4] * m[11] * m[13] -
			m[8] * m[5] * m[15] +
			m[8] * m[7] * m[13] +
			m[12] * m[5] * m[11] -
			m[12] * m[7] * m[9];

		inv[12] = -m[4] * m[9] * m[14] +
			m[4] * m[10] * m[13] +
			m[8] * m[5] * m[14] -
			m[8] * m[6] * m[13] -
			m[12] * m[5] * m[10] +
			m[12] * m[6] * m[9];

		inv[1] = -m[1] * m[10] * m[15] +
			m[1] * m[11] * m[14] +
			m[9] * m[2] * m[15] -
			m[9] * m[3] * m[14] -
			m[13] * m[2] * m[11] +
			m[13] * m[3] * m[10];

		inv[5] = m[0] * m[10] * m[15] -
			m[0] * m[11] * m[14] -
			m[8] * m[2] * m[15] +
			m[8] * m[3] * m[14] +
			m[12] * m[2] * m[11] -
			m[12] * m[3] * m[10];

		inv[9] = -m[0] * m[9] * m[15] +
			m[0] * m[11] * m[13] +
			m[8] * m[1] * m[15] -
			m[8] * m[3] * m[13] -
			m[12] * m[1] * m[11] +
			m[12] * m[3] * m[9];

		inv[13] = m[0] * m[9] * m[14] -
			m[0] * m[10] * m[13] -
			m[8] * m[1] * m[14] +
			m[8] * m[2] * m[13] +
			m[12] * m[1] * m[10] -
			m[12] * m[2] * m[9];

		inv[2] = m[1] * m[6] * m[15] -
			m[1] * m[7] * m[14] -
			m[5] * m[2] * m[15] +
			m[5] * m[3] * m[14] +
			m[13] * m[2] * m[7] -
			m[13] * m[3] * m[6];

		inv[6] = -m[0] * m[6] * m[15] +
			m[0] * m[7] * m[14] +
			m[4] * m[2] * m[15] -
			m[4] * m[3] * m[14] -
			m[12] * m[2] * m[7] +
			m[12] * m[3] * m[6];

		inv[10] = m[0] * m[5] * m[15] -
			m[0] * m[7] * m[13] -
			m[4] * m[1] * m[15] +
			m[4] * m[3] * m[13] +
			m[12] * m[1] * m[7] -
			m[12] * m[3] * m[5];

		inv[14] = -m[0] * m[5] * m[14] +
			m[0] * m[6] * m[13] +
			m[4] * m[1] * m[14] -
			m[4] * m[2] * m[13] -
			m[12] * m[1] * m[6] +
			m[12] * m[2] * m[5];

		inv[3] = -m[1] * m[6] * m[11] +
			m[1] * m[7] * m[10] +
			m[5] * m[2] * m[11] -
			m[5] * m[3] * m[10] -
			m[9] * m[2] * m[7] +
			m[9] * m[3] * m[6];

		inv[7] = m[0] * m[6] * m[11] -
			m[0] * m[7] * m[10] -
			m[4] * m[2] * m[11] +
			m[4] * m[3] * m[10] +
			m[8] * m[2] * m[7] -
			m[8] * m[3] * m[6];

		inv[11] = -m[0] * m[5] * m[11] +
			m[0] * m[7] * m[9] +
			m[4] * m[1] * m[11] -
			m[4] * m[3] * m[9] -
			m[8] * m[1] * m[7] +
			m[8] * m[3] * m[5];

		inv[15] = m[0] * m[5] * m[10] -
			m[0] * m[6] * m[9] -
			m[4] * m[1] * m[10] +
			m[4] * m[2] * m[9] +
			m[8] * m[1] * m[6] -
			m[8] * m[2] * m[5];

		det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];

		if (det == 0)
			return false;

		det = 1.0f / det;

		for (int j = 0; j < 4; ++j)
			for (int k = 0; k < 4; ++k)
				aOutMatrix.SetRowCol(j, k, inv[j * 4 + k] * det);

		return true;
	}
	inline bool SC_Matrix44::Inverse()
	{
		return GetInverseFast(*this);
	}
}