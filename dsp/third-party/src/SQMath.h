#pragma once

#if defined(_MSC_VER) && !defined(DEPRECATED)
//#define DEPRECATED
#endif

#if !defined(M_PI)
#define M_PI 3.14159265358979323846264338327950288
#endif

#ifdef _V1
    #include "math.hpp"
    #include "dsp/filter.hpp"
#else
    #include "util/math.hpp"
    #include "dsp/functions.hpp"
    #include "dsp/filter.hpp"
#endif

#if 0
/**
 * A wrapper around rack's math functions.
 * Mitigates some V1 vs V06 issues.
 */
namespace sq 
{
#ifdef _V1
    using RCFilter = rack::dsp::RCFilter;
#else
    using RCFilter = rack::RCFilter;
#endif

inline float quadraticBipolar(float x)
{
#ifdef _V1
    return rack::dsp::quadraticBipolar(x);
#else
    return rack::quadraticBipolar(x);
#endif
}

inline float clamp(float a, float b, float c)
{
    #ifdef _V1 
        return rack::math::clamp(a, b, c);
    #else
        return rack::clamp(a, b, c);
    #endif
}

inline float interpolateLinear(float* a, float b)
{
    #ifdef _V1
        return rack::math::interpolateLinear(a, b);
    #else
        return rack::interpolateLinear(a, b);
    #endif
}


inline float rescale(float a, float b, float c, float d, float e)
{
    #ifdef _V1
        return rack::math::rescale(a, b, c, d, e);
    #else
        return rack::rescale(a, b, c, d, e);
    #endif
}
}
#endif

