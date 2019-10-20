#pragma once

#include <immintrin.h>
#include <random>
#if !defined(M_PI)
#define M_PI float(3.14159265358979323846264338327950288)
#endif

#if defined(_MSC_VER)
#pragma warning (push)
#pragma warning ( disable: 4244 )
#endif

#include "math.hpp" 
#if defined(_MSC_VER)
#pragma warning (pop)
#endif


#if defined(_MSC_VER)
#pragma warning (push)
#pragma warning (disable: 4305)
#endif

#include "dsp/filter.hpp"

#if defined(_MSC_VER)
#pragma warning (pop)
#endif

/**
 * A wrapper around rack's math functions.
 * Mitigates some V1 vs V06 issues.
 */
namespace sq 
{

using RCFilter = ::rack::dsp::RCFilter;
using Vec = ::rack::math::Vec;
using Rect = ::rack::math::Rect;


inline float quadraticBipolar(float x)
{
    return ::rack::dsp::quadraticBipolar(x);
}

inline float clamp(float a, float b, float c)
{
    return ::rack::math::clamp(a, b, c);
}

inline float interpolateLinear(float* a, float b)
{
    return ::rack::math::interpolateLinear(a, b);
}


inline float rescale(float a, float b, float c, float d, float e)
{
    return ::rack::math::rescale(a, b, c, d, e);
}
}


