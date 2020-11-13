#include "SC_Math.h"

namespace Shift
{
	float SC_DegreesToRadians(const float aDegree)
	{
		return (aDegree * (SC_PI_FLOAT / 180.0f));
	}

	float SC_RadiansToDegrees(const float aRadian)
	{
		return (aRadian * (180.0f / SC_PI_FLOAT));
	}

	float SC_Cos(float aValue)
	{
		// Calculate abs value (cos(x) == cos(-x))
		float t = fabsf(aValue);

		// Mod to [0 .. 2PI] range
		t = SC_Modf_Macro(t, 2.0f * SC_PI_FLOAT);

		// Reflect in PI
		t = fabsf(t - SC_PI_FLOAT);

		// Offset to sin space
		t -= 0.5f * SC_PI_FLOAT;

		// Taylor series constants
		const float c0 = -1.0f / 6.0f;			// 1/3!
		const float c1 = 1.0f / 120.f;			// 1/5! 
		const float c2 = -1.0f / 5040.f;		// 1/7! 
		const float c3 = 1.0f / 362880.f;		// 1/9! 
		const float c4 = -1.0f / 39916800.f;	// 1/11! 

		// Do Taylor approximation
		const float t2 = t * t;
		return t + t * (t2 * (c0 + t2 * (c1 + t2 * (c2 + t2 * (c3 + t2 * c4)))));
	}

	float SC_Sin(float aValue)
	{
		return SC_Cos(aValue - 0.5f * SC_PI_FLOAT);
	}

	void SC_SinCos(float aValue, float& aOutSin, float& aOutCos)
	{
		aOutSin = SC_Sin(aValue);
		aOutCos = SC_Cos(aValue);
	}

	bool SC_IsPowerOfTwo(const unsigned int aValue)
	{
		return (aValue && !(aValue & (aValue - 1)));
	}
	bool SC_IsPowerOfTwo(const float aValue)
	{
		return SC_IsPowerOfTwo(unsigned int(aValue));
	}

	uint SC_FindLowestPossiblePowerOfTwo(unsigned int aValue)
	{
		static_assert(sizeof(aValue) == 4);
		aValue--;
		aValue |= (aValue >> 1);
		aValue |= (aValue >> 2);
		aValue |= (aValue >> 4);
		aValue |= (aValue >> 8);
		aValue |= (aValue >> 16);
		return (aValue + 1);
	}

	uint64 SC_FindLowestPossiblePowerOfTwo(uint64 aValue)
	{
		static_assert(sizeof(aValue) == 8);
		aValue--;
		aValue |= (aValue >> 1);
		aValue |= (aValue >> 2);
		aValue |= (aValue >> 4);
		aValue |= (aValue >> 8);
		aValue |= (aValue >> 16);
		aValue |= (aValue >> 32);
		return (aValue + 1);
	}

	float SC_Ceil(float aFloat)
	{
		return ceilf(aFloat);
	}

	float SC_Floor(float aFloat)
	{
		return floorf(aFloat);
	}

}