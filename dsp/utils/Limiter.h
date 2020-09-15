#pragma once

#include <stdint.h>
#include "simd/functions.hpp"
#include "MultiLag2.h"

class Limiter {
public:
    float_4 step(float_4);
    void setTimes(float attack, float release, float sampleTime);

private:
    MultiLag2 lag;
    float_4 threshold = 5;
};

inline float_4 Limiter::step(float_4 input)
{
    lag.step( rack::simd::abs(input));

    float_4 reductionGain = threshold / lag.get();
    float_4 gain = SimdBlocks::ifelse( lag.get() > threshold, reductionGain, 1);
#if 0
    printf("input = %s, lag=%s\nred = %s gain=%s\n", 
        toStr(input).c_str(),
        toStr(lag.get()).c_str(),
        toStr(reductionGain).c_str(),
        toStr(gain).c_str());
#endif
    return gain * input;
}

inline void Limiter::setTimes(float attack, float release, float sampleTime)
{
    float normAttack = attack * sampleTime;
    float normRelease = release * sampleTime;
    lag.setAttack(normAttack);
    lag.setRelease(normRelease);
}

