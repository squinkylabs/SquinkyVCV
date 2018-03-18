#pragma once

#include <assert.h>
#include <iostream>

/**
 * Our own little assert library
 */


#define assertEqEx(actual, expected, msg) if (actual != expected) { \
    std::cout << "assertEq failed " << msg << " actual value =" << \
    actual << " expected=" << expected << std::endl; \
    assert(false); }

#define assertEq(actual, expected) assertEqEx(actual, expected, "")

#define assertClose(actual, expected, diff) if (!AudioMath::closeTo(actual, expected, diff)) { \
    std::cout << "assertClose failed actual value =" << \
    actual << " expected=" << expected << std::endl; \
    assert(false); }


// assert less than
#define assertLt(actual, expected) if ( actual >= expected) { \
    std::cout << "assertLt " << expected << " actual value = " << \
    actual << std::endl ; \
    assert(false); }
  
// assert less than or equal to
#define assertLE(actual, expected) if ( actual > expected) { \
    std::cout << "assertLt " << expected << " actual value = " << \
    actual << std::endl ; \
    assert(false); }
// leave space after macro