#pragma once

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

std::string toStr(const float_4& x) {
    std::stringstream s;
    s << x[0] << ", " << x[1] << ", " << x[2] << ", " << x[3];
    return s.str();
}

inline float_4 SimdBlocks::fold(float_4 x)
{
   // return float_4::zero();
    //return (x[0] == 0) ? float_4::zero() : float_4(.5);
    auto mask = x < 0;
   // float foo = mask[0];
    float_4 bias = ifelse(mask, -1, 1);
    printf("x= %s, bias = %s\n", toStr(x).c_str(), toStr(bias).c_str()); fflush(stdout);
    float_4 temp =(x + bias) / 2.f;
    int32_4 phase;
    return x;
}