#pragma once

#if IS_X86_PLATFORM
#include <intrin.h>

namespace Shift
{
	SC_FORCEINLINE __m128 SC_MulAddSIMD(__m128 aFactor1, __m128 aFactor2, __m128 anAddend)
	{
		return _mm_add_ps(_mm_mul_ps(aFactor1, aFactor2), anAddend);
	}
	SC_FORCEINLINE __m128 SC_NegateSIMD(__m128 aValue)
	{
		return _mm_xor_ps(aValue, _mm_set_ps1(-0.f));
	}
	SC_FORCEINLINE __m128 SC_AbsSIMD(__m128 aValue)
	{
		return _mm_andnot_ps(_mm_set_ps1(-0.f), aValue);
	}
}

#endif