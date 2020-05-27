#pragma once

#include "asserts.h"

#include <simd/vector.hpp>
#include <simd/functions.hpp>

using float_4 = rack::simd::float_4;
using int32_4 = rack::simd::int32_4;

class SimdBlocks
{
public:
    static float_4 fold(float_4);
    static float_4 wrapPhase01(float_4 phase);
    static float_4 ifelse(float_4 mask, float_4 a, float_4 b) {
        simd_assertMask(mask);
        return rack::simd::ifelse(mask, a, b);      
    }

    static float_4 ifelse(int32_4 mask, int32_4 a, int32_4 b) {
        simd_assertMask(mask);
        return rack::simd::ifelse(mask, a, b);     
    }

    // these ones either don't make sense, or are not implemented
    static float_4 ifelse(int32_4 mask, float_4 a, float_4 b);
    static float_4 ifelse(float_4 mask, int32_4 a, int32_4 b);
  
    static float_4 ifelse(float_4 mask, float a, float b);
};

inline float_4 SimdBlocks::wrapPhase01(float_4 x)
{
     x -= rack::simd::floor(x);
     simd_assertGE(x, float_4(0));
     simd_assertLE(x, float_4(1));
     return x;
}

// put back here once it works.

inline float_4 SimdBlocks::fold(float_4 x)
{
    auto mask = x < 0;
    simd_assertMask(mask);
    float_4 bias = SimdBlocks::ifelse(mask, float_4(-1), float_4(1));

    float_4 temp =(x + bias) / 2.f;
    int32_4 phase(temp);

    int32_4 one(1);
    int32_4 isEven = one ^ (phase & one);

    // convert to float 4 and compare to make mask
    float_4 isEvenMask = float_4(isEven) > float_4::zero();
    simd_assertMask(isEvenMask);
    // TODO: can optimize! both sides are mirrors
    float_4 evenFold = x - (2.f * phase);
    float_4 oddFold = (0 - x) + (2.f * phase);
    auto ret = SimdBlocks::ifelse(isEvenMask, evenFold, oddFold);
    return ret;
}
