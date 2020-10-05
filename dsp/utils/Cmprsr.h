#pragma once

#include <stdint.h>
#include "simd/functions.hpp"
#include "MultiLag2.h"

class Cmprsr {
public:
    enum class Ratios {
        HardLimit,
        _4_1_hard
    };
    float_4 step(float_4);
    void setTimes(float attackMs, float releaseMs, float sampleTime);
    void setThreshold(float th);

    const MultiLag2& _lag() const;

private:
    MultiLag2 lag;
    float_4 threshold = 5;
    Ratios ratio = Ratios::HardLimit;
};

inline float_4 Cmprsr::step(float_4 input)
{
    lag.step( rack::simd::abs(input));

    float_4 reductionGain = threshold / lag.get();
    float_4 gain = SimdBlocks::ifelse( lag.get() > threshold, reductionGain, 1);

   // printf("th=%f\n", threshold[0]);
#if 0
    printf("input = %s, lag=%s\nred = %s gain=%s\n", 
        toStr(input).c_str(),
        toStr(lag.get()).c_str(),
        toStr(reductionGain).c_str(),
        toStr(gain).c_str());
#endif
    return gain * input;
}

inline const MultiLag2& Cmprsr::_lag() const
{
    return lag;
}

inline void Cmprsr::setThreshold(float th)
{
  //  printf("set thh %f\n", th);
    threshold = float_4(th);
}

inline void Cmprsr::setTimes(float attackMs, float releaseMs, float sampleTime)
{
   const float correction = 2 * M_PI;
    float attackHz = 1000.f / (attackMs * correction);
    float releaseHz = 1000.f / (releaseMs * correction);

    float normAttack = attackHz * sampleTime;
    float normRelease = releaseHz * sampleTime;

    lag.setAttack(normAttack);
    lag.setRelease(normRelease);
}


