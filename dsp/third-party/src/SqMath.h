#pragma once

#include <immintrin.h>
#include <random>
#if !defined(M_PI)
#define M_PI float(3.14159265358979323846264338327950288)
#endif

#ifdef __V1x

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

   // #include "dsp/minblep.hpp"

#else
    #include "util/math.hpp"
    #include "dsp/functions.hpp"
    #include "dsp/filter.hpp"
   // #include "dsp/minblep.hpp"
#endif

#if 1
/**
 * A wrapper around rack's math functions.
 * Mitigates some V1 vs V06 issues.
 */
namespace sq 
{
#ifdef __V1x
    using RCFilter = rack::dsp::RCFilter;
    using Vec = rack::math::Vec;
    using Rect = rack::math::Rect;
#else
    using RCFilter = rack::RCFilter;
    using Vec = rack::Vec;
    using Rect = rack::Rect;
#endif

inline float quadraticBipolar(float x)
{
#ifdef __V1x
    return rack::dsp::quadraticBipolar(x);
#else
    return rack::quadraticBipolar(x);
#endif
}

inline float clamp(float a, float b, float c)
{
    #ifdef __V1x 
        return rack::math::clamp(a, b, c);
    #else
        return rack::clamp(a, b, c);
    #endif
}

inline float interpolateLinear(float* a, float b)
{
    #ifdef __V1x
        return rack::math::interpolateLinear(a, b);
    #else
        return rack::interpolateLinear(a, b);
    #endif
}


inline float rescale(float a, float b, float c, float d, float e)
{
    #ifdef __V1x
        return rack::math::rescale(a, b, c, d, e);
    #else
        return rack::rescale(a, b, c, d, e);
    #endif
}
}
#endif

