#pragma once

#include "AudioMath.h"

#include <assert.h>
#include <iostream>
extern int _mdb;        // MIDI reverence count

/**
 * Our own little assert library, loosely inspired by Chai Assert.
 *
 * Will print information on failure, then generate a "real" assertion
 */


#define assertEQEx(actual, expected, msg) if (actual != expected) { \
    std::cout << "assertEq failed " << msg << " actual value =" << \
    actual << " expected=" << expected << std::endl; \
    assert(false); }

#define assertEQ(actual, expected) assertEQEx(actual, expected, "")

#define assertNEEx(actual, expected, msg) if (actual == expected) { \
    std::cout << "assertNE failed " << msg << " did not expect " << \
    actual << " to be == to " << expected << std::endl; \
    assert(false); }

#define assertNE(actual, expected) assertNEEx(actual, expected, "")

#define assertClose(actual, expected, diff) if (!AudioMath::closeTo(actual, expected, diff)) { \
    std::cout << "assertClose failed actual value =" << \
    actual << " expected=" << expected << std::endl << std::flush; \
    assert(false); }


// assert less than
#define assertLT(actual, expected) if ( actual >= expected) { \
    std::cout << "assertLt " << expected << " actual value = " << \
    actual << std::endl ; \
    assert(false); }

// assert less than or equal to
#define assertLE(actual, expected) if ( actual > expected) { \
    std::cout << "assertLE " << expected << " actual value = " << \
    actual << std::endl ; \
    assert(false); }

// assert greater than 
#define assertGT(actual, expected) if ( actual <= expected) { \
    std::cout << "assertGT " << expected << " actual value = " << \
    actual << std::endl ; \
    assert(false); }
// assert greater than or equal to
#define assertGE(actual, expected) if ( actual < expected) { \
    std::cout << "assertGE " << expected << " actual value = " << \
    actual << std::endl ; \
    assert(false); }

#ifndef NDEBUG
#define assertEvCount(x) assertEQ(MidiEvent::_count, x)
#define assertNoMidi() assertEvCount(0); assertEQ(_mdb, 0)
#else
#define assertEvCount(x)  ((void)0)
#define assertNoMidi()  ((void)0)
#endif
// leave space after macro


#ifndef _MSC_VER

#include <simd/vector.hpp>
#include <simd/functions.hpp>

using float_4 = rack::simd::float_4;
using int32_4 = rack::simd::int32_4;

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

#define simd_assertClose(a, b, c) assertClose(a[0], b[0], c); \
    assertClose(a[1], b[1], c); \
    assertClose(a[2], b[2], c); \
    assertClose(a[3], b[3], c);

// mask must be 0 or all ones
#define assertMask(m) \
    if (m != 0 && (unsigned int)(m) != 0xffffffff) { printf("dword is not mask: %x\n", (unsigned int) m); fflush(stdout); } \
    assert(m == 0 || (unsigned int)(m) == 0xffffffff);

#define simd_assertMask(x) \
    assertMask(x[0]); \
    assertMask(x[1]); \
    assertMask(x[2]); \
    assertMask(x[3]);

#define simd_assertLT(a, b) \
    assertLT(a[0], b[0]); \
    assertLT(a[1], b[1]); \
    assertLT(a[2], b[2]); \
    assertLT(a[3], b[3]); 

#define simd_assertGT(a, b) \
    assertGT(a[0], b[0]); \
    assertGT(a[1], b[1]); \
    assertGT(a[2], b[2]); \
    assertGT(a[3], b[3]); 

#endif