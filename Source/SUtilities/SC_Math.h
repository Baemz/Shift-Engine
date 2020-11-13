#pragma once
#include <math.h>
#include <float.h>
#include "SC_Types.h"

#define SC_Truncf(aFloat) ((float)((int)(aFloat)))
#define SC_Modf_Macro(aX, aY) ((aX) - SC_Truncf((aX) * (1.0f / (aY))) * (aY))

#define SC_Pi (3.1415926535897932384626433832795)

namespace Shift
{

	//-------------------------------------------------------------//
	// ANGLES
	static constexpr float SC_PI_FLOAT = float(SC_Pi);
	static constexpr double SC_PI_DOUBLE = double(SC_Pi);

	float SC_DegreesToRadians(const float aDegree);
	float SC_RadiansToDegrees(const float aRadian);
	float SC_Cos(float aValue);
	float SC_Sin(float aValue);
	void SC_SinCos(float aValue, float& aOutSin, float& aOutCos);

	//-------------------------------------------------------------//
	// POW2
	bool SC_IsPowerOfTwo(const unsigned int aValue);
	bool SC_IsPowerOfTwo(const float aValue);
	uint SC_FindLowestPossiblePowerOfTwo(unsigned int aValue);
	uint64 SC_FindLowestPossiblePowerOfTwo(uint64 aValue);

	//-------------------------------------------------------------//
	// ABS
	#define SC_Fabs(aValue) fabsf(aValue)

	inline float SC_Abs(float aValue) 
	{
		return aValue < 0.0f ? -aValue : aValue;
	}

	inline double SC_Abs(double aVal)
	{
		return aVal < 0.0 ? -aVal : aVal;
	}

	inline int SC_Abs(int aVal)
	{
		const int signMask = aVal >> (sizeof(int) * 8 - 1);
		return (aVal ^ signMask) - signMask;
	}

	inline int64 SC_Abs(int64 aVal)
	{
		const int64 signMask = aVal >> 63;
		return (aVal ^ signMask) - signMask;
	}

	float SC_Ceil(float aFloat);
	float SC_Floor(float aFloat);


	//-------------------------------------------------------------//
	// Generic Math

	inline float SC_InvSqrt(const float aValue)
	{
		if (aValue == 0.0f)
			return 0.0f;

		return 1.0f / sqrtf(aValue);
	}

	inline float SC_Fmod(float aX, float aY)
	{
		const float t = aX / aY;
		return aX - SC_Truncf(t) * aY;
	}

	inline double SC_Fmod(double aX, double aY)
	{
		const double t = aX / aY;
		return aX - ((double)((int)(t))) * aY;
	}

	enum SC_NoInit
	{
		SC_NO_INIT
	};

}