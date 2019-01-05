#pragma once

#ifdef _V1
    #include "math.hpp"
#else
    #include "util/math.hpp"
#endif


/**
 * A wrapper around rack's math functions.
 * Mitigates some V1 vs V06 issues.
 */
namespace sq 
{

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