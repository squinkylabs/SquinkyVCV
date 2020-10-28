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
        _2_1_soft,
        _2_1_hard,
        _4_1_soft,
        _4_1_hard,
        _8_1_soft,
        _8_1_hard,
        _20_1_soft,
        _20_1_hard,
        NUM_RATIOS
    };

    float_4 step(float_4);
    void setTimes(float attackMs, float releaseMs, float sampleTime, bool enableDistortionReduction);
    void setThreshold(float th);
    void setCurve(Ratios);
    void setNumChannels(int);

    const MultiLag2& _lag() const;
    static std::vector<std::string> ratios();
    float_4 getGain() const;

    static bool wasInit() {
        return !!ratioCurves[0];
    }

private:
    MultiLag2 lag;
    MultiLPF2 attackFilter;

    bool reduceDistortion = false;
    float_4 threshold = 5;
    float_4 invThreshold = 1.f / 5.f;
    int ratioIndex = 0;
    Ratios ratio = Ratios::HardLimit;
    int maxChannel = 3;
    float_4 gain;

    static CompCurves::LookupPtr ratioCurves[int(Ratios::NUM_RATIOS)];
  
    using  processFunction = float_4 (Cmprsr::*)(float_4 input);
    processFunction procFun = stepGeneric;
    void updateProcFun();

    float_4 stepGeneric(float_4);
    float_4 step1NoDistComp(float_4);
    float_4 step1Comp(float_4);
};

inline float_4 Cmprsr::getGain() const
{
    return gain;
}

inline void Cmprsr::setNumChannels(int ch)
{
    maxChannel = ch - 1;
    updateProcFun();
}

inline void Cmprsr::updateProcFun()
{
    procFun = stepGeneric;
    if (maxChannel == 0 && (ratio != Ratios::HardLimit)) {
        if (reduceDistortion) {
            procFun = step1NoDistComp;
        } else {
            procFun = step1Comp;
        }
    }
}

inline void Cmprsr::setCurve(Ratios r)
{
    ratio = r;
    ratioIndex = int(ratio);
}

inline float_4 Cmprsr::step(float_4 input)
{
   // return (*procFun)(input);
    return (this->*procFun)(input);
}

inline float_4 Cmprsr::step1Comp(float_4 input)
{
    assert(wasInit());
    //printf("comp::step1Soft\n");

    lag.step( rack::simd::abs(input));
    float_4 envelope = lag.get();
    // printf("step no dist reduct 102\n"); fflush(stdout);

    CompCurves::LookupPtr table =  ratioCurves[ratioIndex];
    gain = float_4(1);
    const float_4 level = envelope * invThreshold;

    gain[0] = CompCurves::lookup(table, level[0]);
    // printf("step1soft, input = %s, gain = %s\n", toStr(input).c_str(), toStr(gain).c_str());

    return gain * input;
}

inline float_4 Cmprsr::step1NoDistComp(float_4 input)
{
    //printf("comp::step1NoD\n");
    assert(wasInit());

    lag.step( rack::simd::abs(input));
    attackFilter.step(lag.get());
    float_4 envelope = attackFilter.get();
    // printf("reduce 119\n"); fflush(stdout);


    CompCurves::LookupPtr table =  ratioCurves[ratioIndex];
    gain = float_4(1);
    const float_4 level = envelope * invThreshold;

    gain[0] = CompCurves::lookup(table, level[0]);

    return gain * input;
}

inline float_4 Cmprsr::stepGeneric(float_4 input)
{
    // printf("comp::stepGen\n");
    assert(wasInit());

    float_4 envelope;
    if (reduceDistortion) {
        lag.step( rack::simd::abs(input));
        attackFilter.step(lag.get());
        envelope = attackFilter.get();
        // printf("reduce 140\n"); fflush(stdout);
    } else {
        lag.step( rack::simd::abs(input));
        envelope = lag.get();
        // printf("no reduce 144\n"); fflush(stdout);
    }

    if (ratio == Ratios::HardLimit) {
        float_4 reductionGain = threshold / envelope;
        gain = SimdBlocks::ifelse( envelope > threshold, reductionGain, 1);
        return gain * input;
    } else {
        CompCurves::LookupPtr table =  ratioCurves[ratioIndex];
        gain = float_4(1);
        const float_4 level = envelope * invThreshold;
        for (int i=0; i<4; ++i) {
            if (i <= maxChannel) {
                gain[i] = CompCurves::lookup(table, level[i]);
            }
        }
        return gain * input;
    }
}

inline void Cmprsr::setTimes(float attackMs, float releaseMs, float sampleTime, bool enableDistortionReduction)
{
    const float correction = 2 * M_PI;
    // float attackHz = 1000.f / (attackMs * correction);
    const float releaseHz = 1000.f / (releaseMs * correction);
    const float normRelease = releaseHz * sampleTime;

    if (attackMs < .1) {
         printf("attack zero!\n");
        reduceDistortion = false;       // no way to do this at zero attack
        lag.setInstantAttack(true);
        lag.setRelease(normRelease);
    } else {
        reduceDistortion = enableDistortionReduction;
        const float correction = 2 * M_PI;
        float attackHz = 1000.f / (attackMs * correction);
        lag.setInstantAttack(false);

        const float normAttack = attackHz * sampleTime;
        if (enableDistortionReduction) {
            lag.setAttack(normAttack * 4);
            attackFilter.setCutoff(normAttack * 1);
            
        } else {
            lag.setAttack(normAttack); 
        }
    }

    lag.setRelease(normRelease);
    updateProcFun();
}

inline Cmprsr::Cmprsr()
{
    const float softKnee = 12;
    if (wasInit()) {
        return;
    }

    for (int i = int(Ratios::NUM_RATIOS) - 1; i >= 0; --i) {
        Ratios ratio = Ratios(i);
        switch (ratio) {
            case Ratios::HardLimit:
                // just need to have something here
                ratioCurves[i] = ratioCurves[int(Ratios::_4_1_hard)];
                assert(wasInit());
                break;
            case Ratios::_2_1_soft:
                {
                    CompCurves::Recipe r;
                    r.ratio = 2;
                    r.kneeWidth = softKnee;
                    ratioCurves[i] = CompCurves::makeCompGainLookup(r);
                }
                break;
            case Ratios::_2_1_hard:
                {
                    CompCurves::Recipe r;
                    r.ratio = 2;
                    ratioCurves[i] = CompCurves::makeCompGainLookup(r);
                }
                break;
            case Ratios::_4_1_soft:
                {
                    CompCurves::Recipe r;
                    r.ratio = 4;
                    r.kneeWidth = softKnee;
                    ratioCurves[i] = CompCurves::makeCompGainLookup(r);
                }
                break;
            case Ratios::_4_1_hard:
                {
                    CompCurves::Recipe r;
                    r.ratio = 4;
                    ratioCurves[i] = CompCurves::makeCompGainLookup(r);
                }
                break;
                case Ratios::_8_1_soft:
                {
                    CompCurves::Recipe r;
                    r.ratio = 8;
                    r.kneeWidth = softKnee;
                    ratioCurves[i] = CompCurves::makeCompGainLookup(r);
                }
                break;
                case Ratios::_8_1_hard:
                {
                    CompCurves::Recipe r;
                    r.ratio = 8;
                    ratioCurves[i] = CompCurves::makeCompGainLookup(r);
                }
                break;
                case Ratios::_20_1_soft:
                {
                    CompCurves::Recipe r;
                    r.ratio = 20;
                    r.kneeWidth = softKnee;
                    ratioCurves[i] = CompCurves::makeCompGainLookup(r);
                }
                break;
                case Ratios::_20_1_hard:
                {
                    CompCurves::Recipe r;
                    r.ratio = 20;  
                    ratioCurves[i] = CompCurves::makeCompGainLookup(r);
                }
                break;
            default:
                assert(false);
        }
    }
    assert(wasInit());
}

 inline std::vector<std::string> Cmprsr::ratios()
 {
     assert(int(Ratios::NUM_RATIOS) == 9);
     return {"Limit", "2:1 soft","2:1 hard", "4:1 soft","4:1 hard", "8:1 soft", "8:1 hard", "20:1 soft", "20:1 hard"  };
 }

inline const MultiLag2& Cmprsr::_lag() const
{
    return lag;
}

inline void Cmprsr::setThreshold(float th)
{
    threshold = float_4(th);
    invThreshold = 1.f / threshold;
}



