#pragma once
#include <cstddef>
#include <cstring>

#define SC_ENUM_EXPAND(x) x
#define SC_ENUM_CHOOSE_MAP_START(count) SC_ENUM_MAP ## count
#define SC_ENUM_APPLY(macro, ...) SC_ENUM_EXPAND(macro(__VA_ARGS__))

#define SC_ENUM_MAP(macro, ...) \
    SC_ENUM_EXPAND( \
        SC_ENUM_APPLY(SC_ENUM_CHOOSE_MAP_START, SC_ENUM_COUNT(__VA_ARGS__)) \
            (macro, __VA_ARGS__))

#define SC_ENUM_MAP1(m, x)      m(x)
#define SC_ENUM_MAP2(m, x, ...) m(x) SC_ENUM_EXPAND(SC_ENUM_MAP1(m, __VA_ARGS__))
#define SC_ENUM_MAP3(m, x, ...) m(x) SC_ENUM_EXPAND(SC_ENUM_MAP2(m, __VA_ARGS__))
#define SC_ENUM_MAP4(m, x, ...) m(x) SC_ENUM_EXPAND(SC_ENUM_MAP3(m, __VA_ARGS__))
#define SC_ENUM_MAP5(m, x, ...) m(x) SC_ENUM_EXPAND(SC_ENUM_MAP4(m, __VA_ARGS__))
#define SC_ENUM_MAP6(m, x, ...) m(x) SC_ENUM_EXPAND(SC_ENUM_MAP5(m, __VA_ARGS__))
#define SC_ENUM_MAP7(m, x, ...) m(x) SC_ENUM_EXPAND(SC_ENUM_MAP6(m, __VA_ARGS__))
#define SC_ENUM_MAP8(m, x, ...) m(x) SC_ENUM_EXPAND(SC_ENUM_MAP7(m, __VA_ARGS__))
#define SC_ENUM_MAP9(m, x, ...) m(x) SC_ENUM_EXPAND(SC_ENUM_MAP8(m, __VA_ARGS__))
#define SC_ENUM_MAP10(m, x, ...) m(x) SC_ENUM_EXPAND(SC_ENUM_MAP9(m, __VA_ARGS__))
#define SC_ENUM_MAP11(m, x, ...) m(x) SC_ENUM_EXPAND(SC_ENUM_MAP10(m, __VA_ARGS__))
#define SC_ENUM_MAP12(m, x, ...) m(x) SC_ENUM_EXPAND(SC_ENUM_MAP11(m, __VA_ARGS__))
#define SC_ENUM_MAP13(m, x, ...) m(x) SC_ENUM_EXPAND(SC_ENUM_MAP12(m, __VA_ARGS__))
#define SC_ENUM_MAP14(m, x, ...) m(x) SC_ENUM_EXPAND(SC_ENUM_MAP13(m, __VA_ARGS__))
#define SC_ENUM_MAP15(m, x, ...) m(x) SC_ENUM_EXPAND(SC_ENUM_MAP14(m, __VA_ARGS__))
#define SC_ENUM_MAP16(m, x, ...) m(x) SC_ENUM_EXPAND(SC_ENUM_MAP15(m, __VA_ARGS__))
#define SC_ENUM_MAP17(m, x, ...) m(x) SC_ENUM_EXPAND(SC_ENUM_MAP16(m, __VA_ARGS__))
#define SC_ENUM_MAP18(m, x, ...) m(x) SC_ENUM_EXPAND(SC_ENUM_MAP17(m, __VA_ARGS__))
#define SC_ENUM_MAP19(m, x, ...) m(x) SC_ENUM_EXPAND(SC_ENUM_MAP18(m, __VA_ARGS__))
#define SC_ENUM_MAP20(m, x, ...) m(x) SC_ENUM_EXPAND(SC_ENUM_MAP19(m, __VA_ARGS__))
#define SC_ENUM_MAP21(m, x, ...) m(x) SC_ENUM_EXPAND(SC_ENUM_MAP20(m, __VA_ARGS__))
#define SC_ENUM_MAP22(m, x, ...) m(x) SC_ENUM_EXPAND(SC_ENUM_MAP21(m, __VA_ARGS__))
#define SC_ENUM_MAP23(m, x, ...) m(x) SC_ENUM_EXPAND(SC_ENUM_MAP22(m, __VA_ARGS__))
#define SC_ENUM_MAP24(m, x, ...) m(x) SC_ENUM_EXPAND(SC_ENUM_MAP23(m, __VA_ARGS__))
#define SC_ENUM_MAP25(m, x, ...) m(x) SC_ENUM_EXPAND(SC_ENUM_MAP24(m, __VA_ARGS__))
#define SC_ENUM_MAP26(m, x, ...) m(x) SC_ENUM_EXPAND(SC_ENUM_MAP25(m, __VA_ARGS__))
#define SC_ENUM_MAP27(m, x, ...) m(x) SC_ENUM_EXPAND(SC_ENUM_MAP26(m, __VA_ARGS__))
#define SC_ENUM_MAP28(m, x, ...) m(x) SC_ENUM_EXPAND(SC_ENUM_MAP27(m, __VA_ARGS__))
#define SC_ENUM_MAP29(m, x, ...) m(x) SC_ENUM_EXPAND(SC_ENUM_MAP28(m, __VA_ARGS__))
#define SC_ENUM_MAP30(m, x, ...) m(x) SC_ENUM_EXPAND(SC_ENUM_MAP29(m, __VA_ARGS__))
#define SC_ENUM_MAP31(m, x, ...) m(x) SC_ENUM_EXPAND(SC_ENUM_MAP30(m, __VA_ARGS__))
#define SC_ENUM_MAP32(m, x, ...) m(x) SC_ENUM_EXPAND(SC_ENUM_MAP31(m, __VA_ARGS__))
#define SC_ENUM_MAP33(m, x, ...) m(x) SC_ENUM_EXPAND(SC_ENUM_MAP32(m, __VA_ARGS__))
#define SC_ENUM_MAP34(m, x, ...) m(x) SC_ENUM_EXPAND(SC_ENUM_MAP33(m, __VA_ARGS__))
#define SC_ENUM_MAP35(m, x, ...) m(x) SC_ENUM_EXPAND(SC_ENUM_MAP34(m, __VA_ARGS__))
#define SC_ENUM_MAP36(m, x, ...) m(x) SC_ENUM_EXPAND(SC_ENUM_MAP35(m, __VA_ARGS__))
#define SC_ENUM_MAP37(m, x, ...) m(x) SC_ENUM_EXPAND(SC_ENUM_MAP36(m, __VA_ARGS__))
#define SC_ENUM_MAP38(m, x, ...) m(x) SC_ENUM_EXPAND(SC_ENUM_MAP37(m, __VA_ARGS__))
#define SC_ENUM_MAP39(m, x, ...) m(x) SC_ENUM_EXPAND(SC_ENUM_MAP38(m, __VA_ARGS__))
#define SC_ENUM_MAP40(m, x, ...) m(x) SC_ENUM_EXPAND(SC_ENUM_MAP39(m, __VA_ARGS__))
#define SC_ENUM_MAP41(m, x, ...) m(x) SC_ENUM_EXPAND(SC_ENUM_MAP40(m, __VA_ARGS__))
#define SC_ENUM_MAP42(m, x, ...) m(x) SC_ENUM_EXPAND(SC_ENUM_MAP41(m, __VA_ARGS__))
#define SC_ENUM_MAP43(m, x, ...) m(x) SC_ENUM_EXPAND(SC_ENUM_MAP42(m, __VA_ARGS__))
#define SC_ENUM_MAP44(m, x, ...) m(x) SC_ENUM_EXPAND(SC_ENUM_MAP43(m, __VA_ARGS__))
#define SC_ENUM_MAP45(m, x, ...) m(x) SC_ENUM_EXPAND(SC_ENUM_MAP44(m, __VA_ARGS__))
#define SC_ENUM_MAP46(m, x, ...) m(x) SC_ENUM_EXPAND(SC_ENUM_MAP45(m, __VA_ARGS__))
#define SC_ENUM_MAP47(m, x, ...) m(x) SC_ENUM_EXPAND(SC_ENUM_MAP46(m, __VA_ARGS__))
#define SC_ENUM_MAP48(m, x, ...) m(x) SC_ENUM_EXPAND(SC_ENUM_MAP47(m, __VA_ARGS__))
#define SC_ENUM_MAP49(m, x, ...) m(x) SC_ENUM_EXPAND(SC_ENUM_MAP48(m, __VA_ARGS__))
#define SC_ENUM_MAP50(m, x, ...) m(x) SC_ENUM_EXPAND(SC_ENUM_MAP49(m, __VA_ARGS__))
#define SC_ENUM_MAP51(m, x, ...) m(x) SC_ENUM_EXPAND(SC_ENUM_MAP50(m, __VA_ARGS__))
#define SC_ENUM_MAP52(m, x, ...) m(x) SC_ENUM_EXPAND(SC_ENUM_MAP51(m, __VA_ARGS__))
#define SC_ENUM_MAP53(m, x, ...) m(x) SC_ENUM_EXPAND(SC_ENUM_MAP52(m, __VA_ARGS__))
#define SC_ENUM_MAP54(m, x, ...) m(x) SC_ENUM_EXPAND(SC_ENUM_MAP53(m, __VA_ARGS__))
#define SC_ENUM_MAP55(m, x, ...) m(x) SC_ENUM_EXPAND(SC_ENUM_MAP54(m, __VA_ARGS__))
#define SC_ENUM_MAP56(m, x, ...) m(x) SC_ENUM_EXPAND(SC_ENUM_MAP55(m, __VA_ARGS__))
#define SC_ENUM_MAP57(m, x, ...) m(x) SC_ENUM_EXPAND(SC_ENUM_MAP56(m, __VA_ARGS__))
#define SC_ENUM_MAP58(m, x, ...) m(x) SC_ENUM_EXPAND(SC_ENUM_MAP57(m, __VA_ARGS__))
#define SC_ENUM_MAP59(m, x, ...) m(x) SC_ENUM_EXPAND(SC_ENUM_MAP58(m, __VA_ARGS__))
#define SC_ENUM_MAP60(m, x, ...) m(x) SC_ENUM_EXPAND(SC_ENUM_MAP59(m, __VA_ARGS__))
#define SC_ENUM_MAP61(m, x, ...) m(x) SC_ENUM_EXPAND(SC_ENUM_MAP60(m, __VA_ARGS__))
#define SC_ENUM_MAP62(m, x, ...) m(x) SC_ENUM_EXPAND(SC_ENUM_MAP61(m, __VA_ARGS__))
#define SC_ENUM_MAP63(m, x, ...) m(x) SC_ENUM_EXPAND(SC_ENUM_MAP62(m, __VA_ARGS__))
#define SC_ENUM_MAP64(m, x, ...) m(x) SC_ENUM_EXPAND(SC_ENUM_MAP63(m, __VA_ARGS__))
#define SC_ENUM_EVALUATE_COUNT(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11 , _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, _59, _0, _61, _62, _63, _64, count, ...) count
#define SC_ENUM_COUNT(...) \
    SC_ENUM_EXPAND(SC_ENUM_EVALUATE_COUNT(__VA_ARGS__, 64, 63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1))

struct SC_Enum_Ignore_Assign {
    SC_Enum_Ignore_Assign(int value) : myValue(value) { }
    operator int() const { return myValue; }

    const SC_Enum_Ignore_Assign& operator=(int /*dummy*/) { return *this; }

    int myValue;
};

#define SC_ENUM_IGNORE_ASSIGN_SINGLE(expression) (SC_Enum_Ignore_Assign)expression,
#define SC_ENUM_IGNORE_ASSIGN(...) SC_ENUM_EXPAND(SC_ENUM_MAP(SC_ENUM_IGNORE_ASSIGN_SINGLE, __VA_ARGS__))

#define SC_ENUM_STRINGIFY_SINGLE(expression) #expression,
#define SC_ENUM_STRINGIFY(...) SC_ENUM_EXPAND(SC_ENUM_MAP(SC_ENUM_STRINGIFY_SINGLE, __VA_ARGS__))

#define SC_ENUM(EnumName, ...)                                                  \
struct EnumName {                                                               \
    enum Enumerated { __VA_ARGS__ };                                            \
                                                                                \
    Enumerated     myValue;                                                     \
    static constexpr const char* TypeName = SC_STRINGIFY(EnumName);           \
    EnumName(Enumerated value) : myValue(value) { }                             \
    operator Enumerated() const { return myValue; }                             \
                                                                                \
    const char* ToString() const                                                \
    {                                                                           \
        for (size_t index = 0; index < myCount; ++index) {                      \
            if (Values()[index] == myValue)                                     \
                return Names()[index];                                          \
        }                                                                       \
                                                                                \
        return nullptr;                                                         \
    }                                                                           \
                                                                                \
    static const size_t myCount = SC_ENUM_EXPAND(SC_ENUM_COUNT(__VA_ARGS__));   \
                                                                                \
    static const int* Values()                                                  \
    {                                                                           \
        static const int values[] =                                             \
            { SC_ENUM_EXPAND(SC_ENUM_IGNORE_ASSIGN(__VA_ARGS__)) };             \
        return values;                                                          \
    }                                                                           \
                                                                                \
    static const char* const* Names()                                           \
    {                                                                           \
        static const char* const    rawNames[] =                                \
            { SC_ENUM_EXPAND(SC_ENUM_STRINGIFY(__VA_ARGS__)) };                 \
                                                                                \
        static char*                processedNames[myCount];                    \
        static bool                 initialized = false;                        \
                                                                                \
        if (!initialized) {                                                     \
            for (size_t index = 0; index < myCount; ++index) {                  \
                size_t length =                                                 \
                    std::strcspn(rawNames[index], " =\t\n\r");                  \
                                                                                \
                processedNames[index] = new char[length + 1];                   \
                                                                                \
                std::strncpy(                                                   \
                    processedNames[index], rawNames[index], length);            \
                processedNames[index][length] = '\0';                           \
            }                                                                   \
        }                                                                       \
                                                                                \
        return processedNames;                                                 \
    }                                                                           \
};