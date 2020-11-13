#pragma once
#include "SC_Vector.h"

namespace Shift
{
	void SC_SRand(uint aSeed);
	void SC_RandCleanup();
	uint SC_GenerateRandomSeed(uint aFuzzValue = 0);
	uint SC_Rand32();

	// Journal of Statistical Software Vol. 8, Issue 14, Jul 2003
	// http://www.jstatsoft.org/v08/i14/
	class SC_Random
	{
	public:
		SC_Random(uint aSeed = 0);

		void SetSeed(uint aSeed);

		uint		NextUint();
		float		Next1To2();			// Returns [1..2)	(Random between 1 and 2 is the very fastest)
		float		Next0To1();			// Returns [0..1)
		float		NextMinus1To1();	// Returns [-1..1)
		SC_Vector2f NextNorm2f();		// Returns normalized vector
		SC_Vector3f NextNorm3f();		// Returns normalized vector
		SC_Vector2f NextInCircle();		// Returns vector with length [0..1]
		SC_Vector3f NextInSphere();		// Returns vector with length [0..1]
		SC_Vector2f NextVec2f_0To1();	// Returns [0..1, 0..1]
		SC_Vector3f NextVec3f_0To1();	// Returns [0..1, 0..1, 0..1]
	private:
		uint myData[4];

	};
}