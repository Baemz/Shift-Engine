#include <assert.h>

#undef assert

#if IS_VISUAL_STUDIO
	#define SC_DEBUG_BREAK_IMPL()	__debugbreak()
#else
	#error Not supported on this platform yet
#endif

namespace Shift
{
	bool SC_Assert_Internal(const char* aFile, int aLine, const char* aMessage);
	bool SC_Assert_Internal(const char* aFile, int aLine, const char* aMessage, const char* aMessageFmtStr, ...);
}

#define SC_ASSERT_ALWAYS(X, ...) \
	do { \
		if(SC_BRANCH_HINT(!(X), false)) \
		{ \
			if(SC_Assert_Internal(__FILE__, __LINE__, #X, ##__VA_ARGS__)) \
				SC_DEBUG_BREAK_IMPL(); \
		} \
	} while(0)


#if ENABLE_ASSERTS
	#define SC_ASSERTS_ONLY(X)		X
	#define SC_ASSERT(X, ...)		SC_ASSERT_ALWAYS(X, ##__VA_ARGS__)
	#define SC_VERIFY(X, ...)		SC_ASSERT_ALWAYS(X, ##__VA_ARGS__)
	#define SC_DEBUG_BREAK()		SC_DEBUG_BREAK_IMPL()
#else
	#define SC_ASSERTS_ONLY(X)
	#define SC_ASSERT(X, ...)		(static_cast<void>(X))
	#define SC_VERIFY(X, ...)		(static_cast<void>(X))
	#define SC_DEBUG_BREAK()		(static_cast<void>(0))
#endif

#define assert(...) SC_ASSERT(__VA_ARGS__)