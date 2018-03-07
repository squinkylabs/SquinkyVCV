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
    