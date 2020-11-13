#include "SCore_Precompiled.h"
#include "SC_Random.h"

#include <time.h>

#if IS_X86_PLATFORM
#	include "SC_SIMDIntrinsics.h"
#	include <intrin.h>
#endif

namespace Shift
{

	static thread_local uint32 locThreadRandomSeed = 0;
	static thread_local SC_Random* locThreadRNG;

	void locCreateRNG()
	{
		locThreadRNG = new SC_Random(locThreadRandomSeed);
	}

	SC_Random* locGetRNG()
	{
		if (!locThreadRNG)
			locCreateRNG();

		return locThreadRNG;
	}

#if IS_PC_PLATFORM
	static bool locHardwareRNG()
	{
		static volatile int hasHardwareRNG = -1; // -1 unknown, 0 no, 1 yes

		if (hasHardwareRNG != -1)
			return hasHardwareRNG == 1;
		int info[4];
		__cpuid(info, 0);
		if (info[1] == 0x756e6547 && info[2] == 0x6c65746e && info[3] == 0x49656e69) // "GenuineIntel"
		{
			__cpuid(info, 1);
			if ((info[2] & 0x40000000) == 0x40000000) // CPU has a hardware true random number generator
			{
				hasHardwareRNG = 1;
				return true;
			}
		}
		hasHardwareRNG = 0;
		return false;
}
#endif

	void SC_SRand(uint aSeed)
	{
		locThreadRandomSeed = SC_GenerateRandomSeed(aSeed);
	}

	void SC_RandCleanup()
	{
		if (SC_Random* gen = locThreadRNG)
		{
			delete gen;
			locThreadRNG = 0;
		}
	}

	uint SC_GenerateRandomSeed(uint aFuzzValue)
	{
#if IS_PS4_PLATFORM
		return 0;
#else
#if IS_PC_PLATFORM
		if (locHardwareRNG())
		{
			uint randomValue = 0;
			if (_rdrand32_step(&randomValue))
				return randomValue;
		}
#endif
		return ((uint)time(nullptr)) ^ (aFuzzValue << 1);
#endif
	}

	uint SC_Rand32()
	{
		SC_Random* gen = locGetRNG();
		return gen->NextUint();
	}

	uint locOldVAXRandom(uint aPreviousNumber)
	{
		return (aPreviousNumber * 69069) + 362437;
	}

	SC_Random::SC_Random(uint aSeed)
	{
		SetSeed(aSeed);
	}

	void SC_Random::SetSeed(uint aSeed)
	{
		if (!aSeed)
			aSeed = SC_GenerateRandomSeed(static_cast<uint>(reinterpret_cast<uint64>(this)));

		for (uint i = 0; i != 4; ++i)
			myData[i] = aSeed = (locOldVAXRandom(aSeed));
	}

	uint SC_Random::NextUint()
	{
		uint t = (myData[0] ^ (myData[0] << 11));
		myData[0] = myData[1];
		myData[1] = myData[2];
		myData[2] = myData[3];
		myData[3] = (myData[3] ^ (myData[3] >> 19)) ^ (t ^ (t >> 8));
		return myData[3];
	}

	float SC_Random::Next1To2()
	{
		const uint val = 0x3F800000 | (NextUint() & 0x7fffff);
		return *(float*)&val;
	}

	float SC_Random::Next0To1()
	{
		const uint val = 0x3F800000 | (NextUint() & 0x7fffff);
		return *(float*)&val - 1.f;
	}

	float SC_Random::NextMinus1To1()
	{
		const uint val = 0x40000000 | (NextUint() & 0x7fffff);
		return *(float*)&val - 3.0f;
	}

	SC_Vector2f SC_Random::NextNorm2f()
	{
		const uint val = NextUint();
		SC_Vector2f returnVal;
		*(uint*)&returnVal.x = 0x40000000U | ((val & 0x0000ffffU) << 7);
		*(uint*)&returnVal.y = 0x40000000U | ((val & 0xffff0000U) >> 9);
		returnVal.x -= 3.0f;
		returnVal.y -= 3.0f;
		const float length = returnVal.x * returnVal.x + returnVal.y * returnVal.y;
		if (length != 0.0f)
			returnVal *= SC_InvSqrt(length);
		else
			returnVal.x = 1.0f;
		return returnVal;
	}

	SC_Vector3f SC_Random::NextNorm3f()
	{
		const uint val = NextUint();
		SC_Vector3f returnVal;
		*(uint*)&returnVal.x = 0x40000000U | ((val & 0xffe00000U) >> 9);
		*(uint*)&returnVal.y = 0x40000000U | ((val & 0x001ffc00U) << 2);
		*(uint*)&returnVal.z = 0x40000000U | ((val & 0x000003ffU) << 13);
		returnVal.x -= 3.0f;
		returnVal.y -= 3.0f;
		returnVal.z -= 3.0f;
		const float length = returnVal.x * returnVal.x + returnVal.y * returnVal.y + returnVal.z * returnVal.z;
		if (length != 0.0f)
			returnVal *= SC_InvSqrt(length);
		else
			returnVal.x = 1.0f;
		return returnVal;
	}

	SC_Vector2f SC_Random::NextInCircle()
	{
		SC_Vector2f returnVal;
		for (int a = 0; a < 20; ++a)
		{
			const uint val = NextUint();
			*(uint*)&returnVal.x = 0x40000000U | ((val & 0x0000ffffU) << 7);
			*(uint*)&returnVal.y = 0x40000000U | ((val & 0xffff0000U) >> 9);
			returnVal.x -= 3.0f;
			returnVal.y -= 3.0f;
			if (returnVal.Length2() <= 1)
				return returnVal;
		}
		returnVal.Normalize();
		return returnVal;
	}

	SC_Vector3f SC_Random::NextInSphere()
	{
		SC_Vector3f returnVal;
		for (int a = 0; a < 20; ++a)
		{
			const uint val = NextUint();
			*(uint*)&returnVal.x = 0x40000000U | ((val & 0xffe00000U) >> 9);	
			*(uint*)&returnVal.y = 0x40000000U | ((val & 0x001ffc00U) << 2);	
			*(uint*)&returnVal.z = 0x40000000U | ((val & 0x000003ffU) << 13);	
			returnVal.x -= 3.0f;
			returnVal.y -= 3.0f;
			returnVal.z -= 3.0f;
			if (returnVal.Length2() <= 1)
				return returnVal;
		}
		returnVal.Normalize();
		return returnVal;
	}

	SC_Vector2f SC_Random::NextVec2f_0To1()
	{
		const uint val = NextUint();
		SC_Vector2f returnVal;
		*(uint*)&returnVal.x = 0x3F800000U | ((val & 0x0000ffffU) << 7);
		*(uint*)&returnVal.y = 0x3F800000U | ((val & 0xffff0000U) >> 9);
		returnVal.x -= 1.0f;
		returnVal.y -= 1.0f;
		return returnVal;
	}

	SC_Vector3f SC_Random::NextVec3f_0To1()
	{
		const uint val = NextUint();
		SC_Vector2f returnVal;
		*(uint*)&returnVal.x = 0x3F800000U | ((val & 0xffe00000U) >> 9);
		*(uint*)&returnVal.y = 0x3F800000U | ((val & 0x001ffc00U) << 2);
		*(uint*)&returnVal.z = 0x3F800000U | ((val & 0x000003ffU) << 13);
		returnVal.x -= 1.0f;
		returnVal.y -= 1.0f;
		return returnVal;
	}
}
