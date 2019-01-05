#pragma once

#include "math.hpp"

namespace sq 
{

inline float clamp(float a, float b, float c)
{
    return rack::math::clamp(a, b, c);
}

inline float interpolateLinear(float* a, float b)
{
    return rack::math::interpolateLinear(a, b);
}


inline float rescale(float a, float b, float c, float d, float e)
{
    return rack::math::rescale(a, b, c, d, e);
}

}