#pragma once

#ifdef _WIN64
typedef void* SC_Handle;
#define SE_NULL_HANDLE nullptr
#else
typedef uint64_t SC_Handle;
#define SE_NULL_HANDLE 0
#endif

typedef unsigned __int64 ulong;
typedef unsigned int uint;
typedef unsigned short ushort;
typedef unsigned char uchar;
typedef ulong uint64;
typedef uint uint32;
typedef ushort uint16;
typedef uchar uint8;
typedef __int64 int64;
typedef int int32;
typedef short int16;
typedef char int8;

static constexpr ulong SC_ULONG_MAX = 0xffffffffffffffffui64;
static constexpr uint SC_UINT_MAX = 0xffffffff;
static constexpr ushort SC_USHORT_MAX = 0xffff;
static constexpr uchar SC_UCHAR_MAX = 0xff;
static constexpr uint64 SC_UINT64_MAX = SC_ULONG_MAX;
static constexpr uint32 SC_UINT32_MAX = SC_UINT_MAX;
static constexpr uint16 SC_UINT16_MAX = SC_USHORT_MAX;
static constexpr uint8 SC_UINT8_MAX = SC_UCHAR_MAX;

static constexpr __int64 SC_LONG_MIN = (-9223372036854775807i64 - 1);
static constexpr int SC_INT_MIN = (-2147483647 - 1);
static constexpr short SC_SHORT_MIN = (-32768);
static constexpr char SC_CHAR_MIN = (-128);
static constexpr __int64 SC_INT64_MIN = SC_LONG_MIN;
static constexpr int SC_INT32_MIN = SC_INT_MIN;
static constexpr short SC_INT16_MIN = SC_SHORT_MIN;
static constexpr char SC_INT8_MIN = SC_CHAR_MIN;

static constexpr __int64 SC_LONG_MAX = 9223372036854775807i64;
static constexpr int SC_INT_MAX = 2147483647;
static constexpr short SC_SHORT_MAX = 32767;
static constexpr char SC_CHAR_MAX = 127;
static constexpr __int64 SC_INT64_MAX = SC_LONG_MAX;
static constexpr int SC_INT32_MAX = SC_INT_MAX;
static constexpr short SC_INT16_MAX = SC_SHORT_MAX;
static constexpr char SC_INT8_MAX = SC_CHAR_MAX;

static constexpr float SC_FLT_MAX = 3.402823466e+38f;
static constexpr float SC_FLT_MIN = 1.175494351e-38f;
static constexpr float SC_FLT_LOWEST = -SC_FLT_MAX;