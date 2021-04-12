#pragma once

#include <stdint.h>
#include "simd/functions.hpp"
#include "MultiLag2.h"
#include "SqLog.h"
#include "SqMath.h"

class Limiter {
public:
    float_4 step(float_4);
   void setTimes(float attackMs, float releaseMs, float sampleTime);

   const MultiLag2& _lag() const;

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
    SQINFO("input = %f, lag=%f reduct = %f gain=%f", 
        input[0],
        lag.get()[0],
        reductionGain[0],
        gain[0]);
#endif
    return gain * input;
}

inline const MultiLag2& Limiter::_lag() const
{
    return lag;
}

inline void Limiter::setTimes(float attackMs, float releaseMs, float sampleTime)
{
   const float correction = 2 * M_PI;
 //  const float correction = 1;
//    printf("correct = %f\n", correction);
    float attackHz = 1000.f / (attackMs * correction);
    float releaseHz = 1000.f / (releaseMs * correction);

    float normAttack = attackHz * sampleTime;
    float normRelease = releaseHz * sampleTime;
#if 0
    printf("in set times, attackMS=%f rms=%f, st=%f\n ahz=%f rhz=%f\nna=%f nr=%f\n",
        attackMs, releaseMs, sampleTime,
        attackHz, releaseHz, 
        normAttack, normRelease); fflush(stdout);
#endif

    lag.setAttack(normAttack);
    lag.setRelease(normRelease);
}


