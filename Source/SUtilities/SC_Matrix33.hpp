#pragma once
#ifndef SC_MATRIX_H
#error You should include Matrix.h instead!
#endif

namespace Shift
{
	static constexpr unsigned char __NUM_VAULES_MATRIX3X3 = 9;
	class SC_Matrix33
	{
	public:
		inline SC_Matrix33();
		inline ~SC_Matrix33();
		inline SC_Matrix33(const SC_Vector3f& aVector1, const SC_Vector3f& aVector2, const SC_Vector3f& aVector3);
		inline SC_Matrix33(float a00, float a01, float a02, float a10, float a11, float a12, float a20, float a21, float a22);

		inline static SC_Matrix33 CreateRotationX(const float anAngle);
		inline static SC_Matrix33 CreateRotationY(const float anAngle);
		inline static SC_Matrix33 CreateRotationZ(const float anAngle);

		inline void MakeLookAt(const SC_Vector3f& aForwardVector, const SC_Vector3f& anUpVector = SC_Vector3f(0.0f, 1.0f, 0.0f));

		//void Normalize();
		//float Length();
		//float Length2();

	public:
		union 
		{
			float _data[__NUM_VAULES_MATRIX3X3];
			struct  
			{
				float	_m11, _m12, _m13,
						_m21, _m22, _m23,
						_m31, _m32, _m33;
			};
			struct
			{
				SC_Vector3f myAxisX;
				SC_Vector3f myAxisY;
				SC_Vector3f myAxisZ;
			};
		};
	};

	inline SC_Matrix33::SC_Matrix33()
		: myAxisX(1.0f, 0.0f, 0.0f)
		, myAxisY(0.0f, 1.0f, 0.0f)
		, myAxisZ(0.0f, 0.0f, 1.0f)
	{
	}

	inline SC_Matrix33::~SC_Matrix33()
	{
	}

	inline SC_Matrix33::SC_Matrix33(const SC_Vector3f& aVector1, const SC_Vector3f& aVector2, const SC_Vector3f& aVector3)
		: myAxisX(aVector1)
		, myAxisY(aVector2)
		, myAxisZ(aVector3)
	{
	}

	inline SC_Matrix33::SC_Matrix33(float a00, float a01, float a02, float a10, float a11, float a12, float a20, float a21, float a22)
		: myAxisX(a00, a01, a02)
		, myAxisY(a10, a11, a12)
		, myAxisZ(a20, a21, a22)
	{
	}

	inline SC_Matrix33 SC_Matrix33::CreateRotationX(const float anAngle)
	{
		float sin;
		float cos;
		SC_SinCos(anAngle, sin, cos);

		return SC_Matrix33(1.0f, 0.0f, 0.0f,
						0.0f, cos, sin,
						0.0f, -sin, cos);
	}

	inline SC_Matrix33 SC_Matrix33::CreateRotationY(const float anAngle)
	{
		float sin;
		float cos;
		SC_SinCos(anAngle, sin, cos);

		return SC_Matrix33(cos, 0.0f, -sin,
						0.0f, 1.0f, 0.0f,
						sin, 0.0f, cos);
	}

	inline SC_Matrix33 SC_Matrix33::CreateRotationZ(const float anAngle)
	{
		float sin;
		float cos;
		SC_SinCos(anAngle, sin, cos);

		return SC_Matrix33(cos, sin, 0.0f,
						-sin, cos, 0.0f,
						0.0f, 0.0f, 1.0f);
	}


	inline void SC_Matrix33::MakeLookAt(const SC_Vector3f& aForwardVector, const SC_Vector3f& anUpVector)
	{
		myAxisX = anUpVector.Cross(aForwardVector);
		myAxisY = aForwardVector.Cross(myAxisX);
		myAxisZ = aForwardVector;

		if (myAxisY.Length2() < 0.00001f)
		{
			*this = SC_Matrix33();
		}
		else
		{
			myAxisX.Normalize();
			myAxisY.Normalize();
			myAxisZ.Normalize();
		}
	}

}