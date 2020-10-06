#pragma once


#include "CompCurves.h"
#include "simd/functions.hpp"
#include "MultiLag2.h"

#include <stdint.h>
#include <assert.h>

class Cmprsr {
public:
    Cmprsr();
    enum class Ratios {
        HardLimit,
        _4_1_hard,
        NUM_RATIOS
    };
    float_4 step(float_4);
    void setTimes(float attackMs, float releaseMs, float sampleTime);
    void setThreshold(float th);

    const MultiLag2& _lag() const;

    static std::vector<std::string> ratios();

private:
    MultiLag2 lag;
    float_4 threshold = 5;
    Ratios ratio = Ratios::HardLimit;

    static CompCurves::LookupPtr ratioCurves[int(Ratios::NUM_RATIOS)];
    static bool wasInit()  {
        return !!ratioCurves[0];
    }
};

inline Cmprsr::Cmprsr()
{
    if (!wasInit()) {
        CompCurves::Recipe r;
        r.ratio = 4;
        r.threshold = 1;
        ratioCurves[int(Ratios::_4_1_hard)] = CompCurves::makeCompGainLookup(r);


        // LImiter is a special case. We don't use the table , but we just need some entry here.
        ratioCurves[int(Ratios::HardLimit)] = ratioCurves[int(Ratios::_4_1_hard)];
    }
}

 inline std::vector<std::string> Cmprsr::ratios()
 {
     assert(int(Ratios::NUM_RATIOS) == 2);
     return {"Limit", "4:1 hard" };
 }


inline float_4 Cmprsr::step(float_4 input)
{
    assert(wasInit());
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


