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

inline float_4 SimdBlocks::fold(float_4)
{
    return float_4::zero();
}