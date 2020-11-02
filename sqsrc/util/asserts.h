#pragma once

#include "simd.h"
#include "AudioMath.h"

#include <assert.h>
#include <iostream>

extern int _mdb;        // MIDI reverence count

/**
 * Our own little assert library, loosely inspired by Chai Assert.
 *
 * Will print information on failure, then generate a "real" assertion
 */

// make all our assserts do nothing with debug is off
#ifdef NDEBUG
//#define assert(_Expression) ((void)0)

#define assertEQ(_Experssion1, _Expression2) ((void)0)
#define assertNE(_Experssion1, _Expression2) ((void)0)
#define assertLE(_Experssion1, _Expression2) ((void)0)
#define assertLT(_Experssion1, _Expression2) ((void)0)
#define assertGT(_Experssion1, _Expression2) ((void)0)
#define assertGE(_Experssion1, _Expression2) ((void)0)
#define assertClose(_Experssion1, _Expression2, _Expression3) ((void)0)
#define assertClosePct(_Experssion1, _Expression2, _Expression3) ((void)0)
#define assertEvCount(x)  ((void)0)
#define assertNoMidi()  ((void)0)

#define simd_assertEQ(_Experssion1, _Expression2) ((void)0)
#define simd_assertNE(_Experssion1, _Expression2) ((void)0)
#define simd_assertSame(_Experssion1) ((void)0)
#define simd_assertGT(_Experssion1, _Expression2) ((void)0)
#define simd_assertGE(_Experssion1, _Expression2) ((void)0)
#define simd_assertLT(_Experssion1, _Expression2) ((void)0)
#define simd_assertLE(_Experssion1, _Expression2) ((void)0)
#define simd_assertMask(_Experssion1) ((void)0)
#define simd_assertClose(_Experssion1, _Expression2, _Expression3) ((void)0)
#define simd_assertClosePct(_Experssion1, _Expression2, _Expression3) ((void)0)
#define simd_assertBetween(_Experssion1, _Expression2, _Expression3) ((void)0)

#else

#define assertEQEx(actual, expected, msg) if (actual != expected) { \
    std::cout << "assertEq failed " << msg << " actual value =" << \
    actual << " expected=" << expected << std::endl << std::flush; \
    assert(false); }

#define assertEQ(actual, expected) assertEQEx(actual, expected, "")

#define assertNEEx(actual, expected, msg) if (actual == expected) { \
    std::cout << "assertNE failed " << msg << " did not expect " << \
    actual << " to be == to " << expected << std::endl << std::flush; \
    assert(false); }

#define assertNE(actual, expected) assertNEEx(actual, expected, "")

#define assertCloseEx(actual, expected, diff, msg) if (!AudioMath::closeTo(actual, expected, diff)) { \
    std::cout << "assertClose failed " << msg << " actual value =" << \
    actual << " expected=" << expected << std::endl << std::flush; \
    assert(false); }

#define assertClose(actual, expected, diff) assertCloseEx(actual, expected, diff, "")

#define assertClosePct(actual, expected, pct) { float diff = expected * pct / 100; \
    if (!AudioMath::closeTo(actual, expected, diff)) { \
    std::cout << "assertClosePct failed actual value =" << \
    actual << " expected=" << expected << " computed diff = " << diff << std::endl << std::flush; \
    assert(false); }}


// assert less than
#define assertLT(actual, expected) if ( actual >= expected) { \
    std::cout << "assertLt " << expected << " actual value = " << \
    actual << std::endl  << std::flush; \
    assert(false); }

// assert less than or equal to
#define assertLE(actual, expected) if ( actual > expected) { \
    std::cout << "assertLE " << expected << " actual value = " << \
    actual << std::endl  << std::flush; \
    assert(false); }

// assert greater than 
#define assertGT(actual, expected) if ( actual <= expected) { \
    std::cout << "assertGT " << expected << " actual value = " << \
    actual << std::endl  << std::flush; \
    assert(false); }
// assert greater than or equal to
#define assertGE(actual, expected) if ( actual < expected) { \
    std::cout << "assertGE " << expected << " actual value = " << \
    actual << std::endl  << std::flush; \
    assert(false); }


#define assertEvCount(x) assertEQ(MidiEvent::_count, x)
#define assertNoMidi() assertEvCount(0); assertEQ(_mdb, 0)

// leave space after macro


// #ifndef _MSC_VER
#if 1

//#include <simd/vector.hpp>
//#include <simd/functions.hpp>
//#include "SimdBlocks.h"


using float_4 = rack::simd::float_4;
using int32_4 = rack::simd::int32_4;

// these ones are anything not zero is true. is that valid?
#define simd_assertFalse(x) (  assert ((int(x[0]) == 0) && (int(x[1]) == 0) && (int(x[2]) == 0) && (int(x[3]) == 0)) )
#define simd_assert(x) (  assert ((int(x[0]) != 0) && (int(x[1]) != 0) && (int(x[2]) != 0) && (int(x[3]) != 0)) )

#define simd_assertEQ(a, b) assertEQEx(a[0], b[0], "simd0"); \
    assertEQEx(a[1], b[1], "simd1"); \
    assertEQEx(a[2], b[2], "simd2"); \
    assertEQEx(a[3], b[3], "simd3");

#define simd_assertNE(a, b) assertNEEx(a[0], b[0], "simd0"); \
    assertNEEx(a[1], b[1], "simd1"); \
    assertNEEx(a[2], b[2], "simd2"); \
    assertNEEx(a[3], b[3], "simd3");

#define simd_assertClose(a, b, c) assertCloseEx(a[0], b[0], c, "simd0"); \
    assertCloseEx(a[1], b[1], c, "simd1"); \
    assertCloseEx(a[2], b[2], c, "simd2"); \
    assertCloseEx(a[3], b[3], c, "simde3");

#define simd_assertClosePct(a, b, c) assertClosePct(a[0], b[0], c); \
    assertClosePct(a[1], b[1], c); \
    assertClosePct(a[2], b[2], c); \
    assertClosePct(a[3], b[3], c);

#if 0
    #define assertClosePct(actual, expected, pct) { float diff = expected * pct / 100; \
    if (!AudioMath::closeTo(actual, expected, diff)) { \
    std::cout << "assertClosePct failed actual value =" << \
    actual << " expected=" << expected << " computed diff = " << diff << std::endl << std::flush; \
    assert(false); }}
#endif

// mask must be 0 or all ones
#if 0   // is this used? it looks wrong
#define assertMask(m) \
    if (((unsigned int) m != 0) && (unsigned int)(m) != 0xffffffff) { printf("dword is not mask: %x\n", (unsigned int) m); fflush(stdout); } \
    assert((unsigned int) m == 0 || (unsigned int)(m) == 0xffffffff);
#endif

/** returns true if m is a valid mask for vector operations expecting a mask
 *
 */
inline bool isMask(float_4 m)
{
    float_4 nm = ~m;
    return (
        (m[0]==0 || nm[0]==0) && 
        (m[1]==0 || nm[1]==0) && 
        (m[2]==0 || nm[2]==0) && 
        (m[3]==0 || nm[3]==0)
    );
}

std::string toStrLiteral(const float_4& x);
std::string toStr(const float_4& x);
std::string toStr(const int32_4& x);
inline void printBadMask(float_4 m) {
    printf("asserts.h (a): not a valid float_4 mask: %s\n", toStr(m).c_str());
    printf("asserts.h (b): not a valid float_4 mask: literal %s\n", toStrLiteral(m).c_str());
    fflush(stdout);
}

inline void printBadMask(int32_4 m) {
    printf("asserts.h: not a valid int32_4 mask: %s\n", toStr(m).c_str());
    fflush(stdout);
}

inline bool isMask(int32_4 m)
{
    int32_4 nm = ~m;
    return (
        (m[0]==0 || nm[0]==0) && 
        (m[1]==0 || nm[1]==0) && 
        (m[2]==0 || nm[2]==0) && 
        (m[3]==0 || nm[3]==0)
    );
}

#define simd_assertMask(x) \
    if (!isMask(x)) {   \
        printBadMask(x);    \
        assert(false); \
    }

#define simd_assertGT(a, b) \
    if ((b[0] >= a[0]) || (b[1] >= a[1]) || (b[2] >= a[2]) || (b[3] >= a[3])) { \
        printf("simd_assertGT(<%s>, <%s>) failed\n", toStr(a).c_str(), toStr(b).c_str()); \
        fflush(stdout); \
        assert(false); \
    }

#define simd_assertGE(a, b) \
    if ((b[0] > a[0]) || (b[1] > a[1]) || (b[2] > a[2]) || (b[3] > a[3])) { \
        printf("simd_assertGE(<%s>, <%s>) failed\n", toStr(a).c_str(), toStr(b).c_str()); \
        fflush(stdout); \
        assert(false); \
    }

#define simd_assertLT(a, b) \
    if ((b[0] <= a[0]) || (b[1] <= a[1]) || (b[2] <= a[2]) || (b[3] <= a[3])) { \
        printf("simd_assertLT(<%s>, <%s>) failed\n", toStr(a).c_str(), toStr(b).c_str()); \
        fflush(stdout); \
        assert(false); \
    }

#define simd_assertLE(a, b) \
    if ((b[0] < a[0]) || (b[1] < a[1]) || (b[2] < a[2]) || (b[3] < a[3])) { \
        printf("simd_assertLE(<%s>, <%s>) failed\n", toStr(a).c_str(), toStr(b).c_str()); \
        fflush(stdout); \
        assert(false); \
    }

#define simd_assertBetween(a, low, high) \
    simd_assertGE((a), (low)); \
    simd_assertLE((a), (high));

#define simd_assertSame(a) \
    assertEQ(a[0], a[1]); \
    assertEQ(a[0], a[2]); \
    assertEQ(a[0], a[3]); 

inline std::string toStr(const float_4& x) {
    std::stringstream s;
    s << x[0] << ", " << x[1] << ", " << x[2] << ", " << x[3];
    return s.str();
}

inline std::string toStr(const int32_4& x) {
    std::stringstream s;
    s << x[0] << ", " << x[1] << ", " << x[2] << ", " << x[3];
    return s.str();
}

inline std::string toStrHex(const int32_4& x) {
    std::stringstream s;
    s << std::hex << x[0] << ", " << x[1] << ", " << x[2] << ", " << x[3];
    return s.str();
}

inline std::string toStrM(const float_4& x) {
    simd_assertMask(x);
    int32_4 i = x;
    std::stringstream s;
    s << (i[0] ? "t" : "f") <<
     ", " << (i[1] ? "t" : "f") <<
     ", " << (i[2] ? "t" : "f") << 
     ", " << (i[3]  ? "t" : "f");
    return s.str();
}

inline std::string toStrLiteral(const float_4& x)
{
    std::stringstream s;
    const float* p = &x[0]; 
    const int* pi;

    pi = reinterpret_cast<const int*>(p);
    s <<  std::hex << "0=" << *pi; 
    ++p;
    pi = reinterpret_cast<const int*>(p);
    s << " 1=" << *pi; 
    ++p;
    pi = reinterpret_cast<const int*>(p);
    s << " 2=" << *pi; 
    ++p;
    pi = reinterpret_cast<const int*>(p);
    s << " 3=" << *pi; 
    return s.str();
}
#endif  // MS
#endif  // NDEBUG