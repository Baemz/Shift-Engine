#pragma once

#ifndef VECTOR_H
#error Vector.h should be included instead of Vector4.hpp
#endif

namespace Shift
{
	template<>
	class SC_ALIGN(16) SC_Vector4<float>
	{
	public:
		SC_Vector4() {}
		SC_Vector4(float aX, float aY, float aZ, float aW = 1.0f) 
		{ 
			SC_ALIGN(16) float f[4] = { aX, aY, aZ, aW };
			myXMM = _mm_load_ps(f);
		}
		SC_Vector4(const SC_Vector3<float>& aVector, const float aW = 1.0f)
		{
			x = aVector.x;
			y = aVector.y;
			z = aVector.z;
			w = aW;
		}

		SC_Vector4(const SC_Vector4<float>& aVec) { myXMM = aVec.myXMM; }
		explicit SC_Vector4(float aValue) { myXMM = _mm_load_ps1(&aValue); }
		explicit SC_Vector4(__m128 aVector) { myXMM = aVector; }

		template <class OtherType>
		SC_Vector4(const SC_Vector4<OtherType>& aVec) { x = (float)aVec.x; y = (float)aVec.y; z = (float)aVec.z; w = (float)aVec.w; }
		template <class OtherType>
		SC_Vector4(const SC_Vector3<OtherType>& aVec) { x = (float)aVec.x; y = (float)aVec.y; z = (float)aVec.z; w = 1.0f; }
	
		template <class OtherType>
		SC_Vector4<float>& operator=(const SC_Vector3<OtherType>& aVec) { x = (float)aVec.x; y = (float)aVec.y; z = (float)aVec.z; w = 1.0f; return *this; }
		template <class OtherType>
		SC_Vector4<float>& operator=(const SC_Vector4<OtherType>& aVec) { x = (float)aVec.x; y = (float)aVec.y; z = (float)aVec.z; w = (float)aVec.w; return *this; }

		//Negate operator
		inline SC_Vector4<float> operator-() const											{ return SC_Vector4<float>(SC_NegateSIMD(myXMM)); }

		// Subtraction operators
		inline SC_Vector4<float>& operator-=(const SC_Vector4<float>& aVector4)				{ myXMM = _mm_sub_ps(myXMM, aVector4.myXMM); return *this; }
		inline SC_Vector4<float>& operator-=(float aValue)									{ myXMM = _mm_sub_ps(myXMM, _mm_load_ps1(&aValue)); return *this; }
		inline SC_Vector4<float> operator-(const SC_Vector4<float>& aVector4) const			{ return SC_Vector4<float>(_mm_sub_ps(myXMM, aVector4.myXMM)); }
		inline SC_Vector4<float> operator-(float aValue) const								{ return SC_Vector4<float>(_mm_sub_ps(myXMM, _mm_load_ps1(&aValue))); }

		//Multiplication operators
		inline SC_Vector4<float>& operator*=(const SC_Vector4<float>& aVector4)				{ myXMM = _mm_mul_ps(myXMM, aVector4.myXMM); return *this; }
		inline SC_Vector4<float>& operator*=(float aValue)									{ myXMM = _mm_mul_ps(myXMM, _mm_load_ps1(&aValue)); return *this; }
		inline SC_Vector4<float> operator*(const SC_Vector4<float>& aVector4) const			{ return SC_Vector4<float>(_mm_mul_ps(myXMM, aVector4.myXMM)); }
		inline SC_Vector4<float> operator*(float aValue) const								{ return SC_Vector4<float>(_mm_mul_ps(myXMM, _mm_load_ps1(&aValue))); }

		//Addition operators
		inline SC_Vector4<float>& operator+=(const SC_Vector4<float>& aVector4)				{ myXMM = _mm_add_ps(myXMM, aVector4.myXMM); return *this; }
		inline SC_Vector4<float>& operator+=(float aValue)									{ myXMM = _mm_add_ps(myXMM, _mm_load_ps1(&aValue)); return *this; }
		inline SC_Vector4<float> operator+(const SC_Vector4<float>& aVector4) const			{ return SC_Vector4<float>(_mm_add_ps(myXMM, aVector4.myXMM)); }
		inline SC_Vector4<float> operator+(float aValue) const								{ return SC_Vector4<float>(_mm_add_ps(myXMM, _mm_load_ps1(&aValue))); }

		//Division operators
		inline SC_Vector4<float>& operator/=(const SC_Vector4<float>& aVector4)				{ myXMM = _mm_div_ps(myXMM, aVector4.myXMM); return *this; }
		inline SC_Vector4<float>& operator/=(float aValue)									{ myXMM = _mm_div_ps(myXMM, _mm_load_ps1(&aValue)); return *this; }
		inline SC_Vector4<float> operator/(const SC_Vector4<float>& aVector4) const			{ return SC_Vector4<float>(_mm_div_ps(myXMM, aVector4.myXMM)); }
		inline SC_Vector4<float> operator/(float aValue) const								{ return SC_Vector4<float>(_mm_div_ps(myXMM, _mm_load_ps1(&aValue))); }

		inline bool operator==(const SC_Vector4<float>& aVector4) const						{ return _mm_movemask_ps(_mm_cmpeq_ps(myXMM, aVector4.myXMM)) == 0xF; }
		inline bool operator!=(const SC_Vector4<float>& aVector4) const						{ return !(*this == aVector4); }
		inline uint operator<(const SC_Vector4<float>& aVector4) const						{ return _mm_movemask_ps(_mm_cmpgt_ps(myXMM, aVector4.myXMM)); }
		inline uint operator<=(const SC_Vector4<float>& aVector4) const						{ return _mm_movemask_ps(_mm_cmpge_ps(myXMM, aVector4.myXMM)); }
		inline uint operator>(const SC_Vector4<float>& aVector4) const						{ return _mm_movemask_ps(_mm_cmplt_ps(myXMM, aVector4.myXMM)); }
		inline uint operator>=(const SC_Vector4<float>& aVector4) const						{ return _mm_movemask_ps(_mm_cmple_ps(myXMM, aVector4.myXMM)); }

		inline float& operator[](uint aIndex) 
		{
			SC_ASSERT(aIndex <= 3 && "Out of Bounds!");

			if (aIndex == 0) return x;
			else if (aIndex == 1) return y;
			else if (aIndex == 2) return z;
			else return w;
		}

		inline bool IsEqual(const SC_Vector4<float>& aVector4, float aEpsilon) const		{ SC_UNUSED(aVector4); SC_UNUSED(aEpsilon); SC_ASSERT(false, "Implement for float!"); }
		inline bool IsNotEqual(const SC_Vector4<float>& aVector4, float aEpsilon) const		{ return !IsEqual(aVector4, aEpsilon); }


		inline SC_Vector2<float> XY() const { return SC_Vector2<float>(x, y); }
		inline SC_Vector3<float> XYZ() const { return SC_Vector3<float>(x, y, z); }

		inline float Length() const { return sqrtf(x * x + y * y + z * z + w * w); }
		inline float Length2() const { return x * x + y * y + z * z + w * w; }
		inline float Length_3D() const { return sqrtf(x * x + y * y + z * z); }
		inline float Length2_3D() const { return (x * x + y * y + z * z); }

		void Normalize();
		SC_Vector4<float> GetNormalized() const;
		inline SC_Vector4<float>& Scale(float aScalar)									{ Normalize(); operator *= (aScalar); return *this; }

		union
		{
			struct
			{
				union { float x, r; };
				union { float y, g; };
				union { float z, b; };
				union { float w, a; };
			};
			__m128 myXMM;
		};

		struct SC_AllowRawHashing_INTERNAL;
	};

	inline void SC_Vector4<float>::Normalize()
	{
		const float length2 = x * x + y * y + z * z + w * w;

		if (length2 > 0)
		{
			const float inversedLength = 1.0f / (float)sqrtf(length2);
			x *= inversedLength;
			y *= inversedLength;
			z *= inversedLength;
			w *= inversedLength;
		}

	}
	inline SC_Vector4<float> SC_Vector4<float>::GetNormalized() const
	{
		SC_Vector4<float> newVec(*this);
		newVec.Normalize();
		return newVec;
	}

	using SC_Vector4f = SC_Vector4<float>;
	static_assert(sizeof(SC_Vector4<float>) == 16);
}