#include "SCore_Precompiled.h"

#if IS_X86_PLATFORM
#include <emmintrin.h>
#endif

#if IS_VISUAL_STUDIO
#	pragma push_macro("assert")
#	undef assert
#	include <intrin.h>
#	pragma pop_macro("assert")

#	pragma intrinsic(_BitScanForward)
#	pragma intrinsic(_BitScanReverse)
#	pragma intrinsic(_BitScanForward64)
#	pragma intrinsic(_BitScanReverse64)
#endif

namespace Shift
{
	int SC_FindFirstBit(uint8 aBitField)
	{
		return SC_FindFirstBit((uint32)aBitField);
	}

	int SC_FindFirstBit(uint16 aBitField)
	{
		return SC_FindFirstBit((uint32)aBitField);
	}

	int SC_FindFirstBit(uint32 aBitField)
	{
#if IS_VISUAL_STUDIO
		unsigned long result;
		if (_BitScanForward(&result, aBitField))
			return result;
		return -1;
#else
#	error "Unknown compiler"
#endif
	}

	int SC_FindFirstBit(uint64 aBitField)
	{
#	if IS_VISUAL_STUDIO
		unsigned long result;
		if (_BitScanForward64(&result, aBitField))
			return result;
		return -1;
#	else
#		error "Unknown compiler"
#	endif
	}

	int SC_FindLastBit(uint8 aBitField)
	{
		return SC_FindLastBit((uint32)aBitField);
	}

	int SC_FindLastBit(uint16 aBitField)
	{
		return SC_FindLastBit((uint32)aBitField);
	}

	int SC_FindLastBit(uint32 aBitField)
	{
#if IS_VISUAL_STUDIO
		unsigned long result;
		if (_BitScanReverse(&result, aBitField))
			return result;
		return -1;
#else
#	error "Unknown compiler"
#endif
	}

	int SC_FindLastBit(uint64 aBitField)
	{
#	if IS_VISUAL_STUDIO
		unsigned long result;
		if (_BitScanReverse64(&result, aBitField))
			return result;
		return -1;
#	else
#		error "Unknown compiler"
#	endif
	}
}