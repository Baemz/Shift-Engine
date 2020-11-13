#include "SC_CommonFuncs.h"
#include "SC_Types.h"
#include <algorithm>

namespace Shift
{
	void SC_Memcpy(void* aDst, void* aSrc, unsigned long long aSize)
	{
		memcpy(aDst, aSrc, aSize);
	}

	void SC_Memmove(void* aDst, void* aSrc, unsigned long long aSize)
	{
		memmove(aDst, aSrc, aSize);
	}

	void SC_Memset(void* aDst, int aValue, unsigned long long aSize)
	{
		memset(aDst, aValue, aSize);
	}

	void SC_ZeroMemory(void* aDst, unsigned long long aSize)
	{
		memset(aDst, 0, aSize);
	}
}