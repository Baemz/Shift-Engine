#include "SCore_Precompiled.h"
#include "SC_Relocate.h"

namespace Shift
{

	enum	SC_FaultyTestEnum { I = 1 };
	struct	SC_FaultyTestData { int i; };
	struct	SC_FaultyTestStruct { SC_FaultyTestStruct() {} void operator=(const SC_FaultyTestStruct&) {} int i; };
	class	SC_FaultyTestClass { public: SC_FaultyTestClass() {} void operator=(const SC_FaultyTestClass&) {} int i; };
	union	SC_FaultyTestUnion { int i; int j; };
	struct	SC_FaultyTestNotAllowed { ~SC_FaultyTestNotAllowed() { volatile int i; i = 1; } };
	
	static_assert(SC_AllowMemcpyRelocation<int>::value, "[int] may NOT be relocated by memcpy");
	static_assert(SC_AllowMemcpyRelocation<float>::value, "[float] may NOT be relocated by memcpy");
	static_assert(SC_AllowMemcpyRelocation<void*>::value, "[pointers] may NOT be relocated by memcpy");
	static_assert(SC_AllowMemcpyRelocation<SC_FaultyTestEnum>::value, "[enum] may NOT be relocated by memcpy");
	static_assert(SC_AllowMemcpyRelocation<SC_FaultyTestData>::value, "[data-structs] may NOT be relocated by memcpy");
	static_assert(SC_AllowMemcpyRelocation<SC_FaultyTestStruct>::value, "[struct] may NOT be relocated by memcpy");
	static_assert(SC_AllowMemcpyRelocation<SC_FaultyTestClass>::value, "[class] may NOT be relocated by memcpy");
	static_assert(SC_AllowMemcpyRelocation<SC_FaultyTestUnion>::value, "[union] may NOT be relocated by memcpy");
	static_assert(!SC_AllowMemcpyRelocation<SC_FaultyTestNotAllowed>::value, "[struct with nontrivial destructor] SHOULD be relocated by memcpy");

	void SC_RelocateBytes(void* aDstLocation, void* aSrcLocation, size_t aCount)
	{
		if (aDstLocation == aSrcLocation || aCount == 0)
			return;

		const ptrdiff_t signedDiff = uintptr_t(aDstLocation) - uintptr_t(aSrcLocation);
		const ptrdiff_t absDiff = (signedDiff >= 0) ? signedDiff : -signedDiff;

		if ((size_t)absDiff >= aCount)
			SC_Memcpy(aDstLocation, aSrcLocation, aCount);
		else 
			SC_Memmove(aDstLocation, aSrcLocation, aCount);
	}
}
