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
};




/*
 static inline float fold(float x)
    {
        float fold;
        const float bias = (x < 0) ? -1.f : 1.f;
        int phase = int((x + bias) / 2.f);
        bool isEven = !(phase & 1);
        if (isEven) {
            fold = x - 2.f * phase;
        } else {
            fold = -x + 2.f * phase;
        }
        return fold;
    }
*/




inline float_4 SimdBlocks::fold(float_4 x)
{
    auto mask = x < 0;
    float_4 bias = ifelse(mask, -1, 1);

    float_4 temp =(x + bias) / 2.f;
    int32_4 phase(temp);

    int32_4 one(1);
    int32_4 isEven = one ^ (phase & one);

    isEven = (isEven > 0);

   // int32_4 isEven= 
   // assert(isEven[0] == 0 || isEven[0] == 0xffffffff);
   simd_assertMask(isEven);

    // TODO: can optimize! both sides are mirrors
    float_4 evenFold = x - (2.f * phase);
    float_4 oddFold = (0 - x) + (2.f * phase);
    auto ret = ifelse(isEven, evenFold, oddFold);

    return ret;
   // return x;
}