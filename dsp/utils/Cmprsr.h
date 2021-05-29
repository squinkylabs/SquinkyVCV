#pragma once

#include "CompCurves.h"
// #include "simd/functions.hpp"
#include <assert.h>
#include <stdint.h>

#include <atomic>

#include "MultiLag2.h"
#include "SqLog.h"
#include "SqMath.h"
#include "simd.h"

#define _USESPLINE
//#define _GLOOK            // just do gain look
#define _FASTLOOK       // new uniform lookups
#define _SQR  // pseudo RMS instead of rectify
//#define _ENV            // output the envelope

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
    float_4 stepPoly(float_4, float_4);
    void setTimes(float attackMs, float releaseMs, float sampleTime);
    void setThreshold(float th);
    void setCurve(Ratios);

    void setTimesPoly(float_4 attackMs, float_4 releaseMs, float sampleTime);
    void setThresholdPoly(float_4 th);
    void setCurvePoly(const Ratios*);

    void setNumChannels(int);

    const MultiLag2& _lag() const;
    static const std::vector<std::string>& ratios();
    static const std::vector<std::string>& ratiosLong();
    float_4 getGain() const;
    void setIsPolyCV(bool poly);

    /**
     * when linked adjacent stereo pairs
     * operate as linked stereo
     */
    void setLinked(bool bLinked) { isLinked = bLinked; }

    static bool wasInit() {
        return !!ratioCurves[0];
    }

    const MultiLag2& _getLag() const { return lag; };
    const MultiLPF2& _getAF() const { return attackFilter; }
    float_4 _getTh() const { return threshold; }
    const Ratios* _getRatio() const { return ratio; }

    static void _reset();
    int _id = 0;            // just for debugging
private:
    MultiLag2 lag;
    MultiLPF2 attackFilter;

    // TODO: get rid of the non-poly version
    bool reduceDistortion = false;
    float_4 reduceDistortionPoly = {0};

    float_4 threshold = 5;
    float_4 invThreshold = 1.f / 5.f;
    bool isLinked = false;

    int ratioIndex[4] = {0};
    Ratios ratio[4] = {Ratios::HardLimit, Ratios::HardLimit, Ratios::HardLimit, Ratios::HardLimit};
    int maxChannel = 3;

    bool cvIsPoly = false;
    bool polySet = false;

#ifdef _SQATOMIC
    std::atomic<float_4> gain_;
#else
    float_4 gain_;
#endif

    static CompCurves::LookupPtr ratioCurves[int(Ratios::NUM_RATIOS)];
    static CompCurves::CompCurveLookupPtr ratioCurves2[int(Ratios::NUM_RATIOS)];

    using processFunction = float_4 (Cmprsr::*)(float_4 input);
    processFunction procFun = &Cmprsr::stepGeneric;
    void updateProcFun();

    float_4 stepGeneric(float_4);
    float_4 step1NoDistComp(float_4);
    float_4 step1Comp(float_4);

    float_4 stepPolyMultiMono(float_4, float_4);
    float_4 stepPolyLinked(float_4, float_4);
    void setThresholdPolySub(float_4 th);
};

inline float_4 Cmprsr::getGain() const {
    return gain_;
}

inline void Cmprsr::setNumChannels(int ch) {
    maxChannel = ch - 1;
    updateProcFun();
}

inline void Cmprsr::setIsPolyCV(bool poly) {
    polySet = true;
    cvIsPoly = poly;
}

// only called for non poly
inline void Cmprsr::updateProcFun() {
    // printf("in update, max = %d\n", maxChannel);
    procFun = &Cmprsr::stepGeneric;
    if (maxChannel == 0 && (ratio[0] != Ratios::HardLimit)) {
        if (reduceDistortion) {
            procFun = &Cmprsr::step1NoDistComp;
        } else {
            procFun = &Cmprsr::step1Comp;
        }
    }
}

inline void Cmprsr::setCurve(Ratios r) {
    assert(polySet);
    assert(!cvIsPoly);

    ratio[0] = r;
    ratio[1] = r;
    ratio[2] = r;
    ratio[3] = r;

    ratioIndex[0] = int(r);
    ratioIndex[1] = int(r);
    ratioIndex[2] = int(r);
    ratioIndex[3] = int(r);
}

inline void Cmprsr::setCurvePoly(const Ratios* r) {
    assert(polySet);
    assert(cvIsPoly);

    ratio[0] = r[0];
    ratio[1] = r[1];
    ratio[2] = r[2];
    ratio[3] = r[3];

    ratioIndex[0] = int(r[0]);
    ratioIndex[1] = int(r[1]);
    ratioIndex[2] = int(r[2]);
    ratioIndex[3] = int(r[3]);
}

inline float_4 Cmprsr::step(float_4 input) {
#ifdef _GLOOK
    assert(false);
    return 0;
#else
    assert(polySet);
    assert(!cvIsPoly);
    return (this->*procFun)(input);
#endif
}

// only non poly
inline float_4 Cmprsr::step1Comp(float_4 input) {
    assert(wasInit());
    assert(polySet);
    assert(!cvIsPoly);

#ifdef _SQR
    lag.step(input * input);
    float_4 envelope = lag.get();
    envelope = rack::simd::sqrt(envelope);
#else
    lag.step(rack::simd::abs(input));
    float_4 envelope = lag.get();
#endif
#ifdef _ENV
    return envelope;
#else
    //  SQINFO("step1comp");

    CompCurves::LookupPtr table = ratioCurves[ratioIndex[0]];
    const float_4 level = envelope * invThreshold;

    float_4 t = gain_;
    t[0] = CompCurves::lookup(table, level[0]);
    gain_ = t;
    return gain_ * input;
#endif
}

// only non poly
inline float_4 Cmprsr::step1NoDistComp(float_4 input) {
    assert(wasInit());
    assert(polySet);
    assert(!cvIsPoly);

#ifdef _SQR
    lag.step(input * input);
    attackFilter.step(lag.get());
    float_4 envelope = attackFilter.get();
    envelope = rack::simd::sqrt(envelope);
#else
    lag.step(rack::simd::abs(input));
    attackFilter.step(lag.get());
    float_4 envelope = attackFilter.get();
#endif

#ifdef _ENV
    return envelope;
#else
    CompCurves::LookupPtr table = ratioCurves[ratioIndex[0]];

    const float_4 level = envelope * invThreshold;

    float_4 t = gain_;
    t[0] = CompCurves::lookup(table, level[0]);
    gain_ = t;
    return gain_ * input;
#endif
}

// only non-poly
inline float_4 Cmprsr::stepGeneric(float_4 input) {
    assert(wasInit());
    assert(polySet);
    assert(!cvIsPoly);

    float_4 envelope;

#ifdef _SQR
    if (reduceDistortion) {
        lag.step(rack::simd::abs(input * input));
        attackFilter.step(lag.get());
        envelope = rack::simd::sqrt(attackFilter.get());
    } else {
        lag.step(rack::simd::abs(input * input));
        envelope = rack::simd::sqrt(lag.get());
    }
#else
    if (reduceDistortion) {
        lag.step(rack::simd::abs(input));
        attackFilter.step(lag.get());
        envelope = attackFilter.get();
    } else {
        lag.step(rack::simd::abs(input));
        envelope = lag.get();
    }
#endif

    if (ratio[0] == Ratios::HardLimit) {
        float_4 reductionGain = threshold / envelope;
        gain_ = SimdBlocks::ifelse(envelope > threshold, reductionGain, 1);
        return gain_ * input;
    } else {
        CompCurves::LookupPtr table = ratioCurves[ratioIndex[0]];
        const float_4 level = envelope * invThreshold;

        float_4 t = gain_;
        for (int i = 0; i < 4; ++i) {
            if (i <= maxChannel) {
                t[i] = CompCurves::lookup(table, level[i]);
            }
        }
        gain_ = t;
        return gain_ * input;
    }
}

#ifdef _GLOOK
inline float_4 Cmprsr::stepPoly(float_4 input, float_4 detectorInput) {
    for (int i=0; i<4; ++i) {
       // float in = detectorInput[i] * 2;
       // float in = 2 + detectorInput[i] * .5;

#if 1
        float in = std::abs(detectorInput[i]);

#else
        float in = detectorInput[i];        // -5 .. 5
        in += 5;                            // 0 ..10
        in *= ( 1.7f / 10.f);               // 0 .. 1.7
        in += .4f;                          // .45 .. 2.1
#endif
        //if (in < 0) in = 0;
        if (i == 1) {
            gain_[i] = in;
        } else if (i == 2) {
            gain_[i] = (in > 1.f) ? 5.f : 0.f;
        } else if (i == 0) {
#if defined(_USESPLINE) 
        // super hack using static
        CompCurves::Recipe r;
        r.ratio = 2;
        r.kneeWidth = 12;
        static CompCurves::CompCurveLookupPtr stable = CompCurves::makeCompGainLookup3(r);
        gain_[i] =  stable->lookup(in);
#elif defined( _FASTLOOK)
        CompCurves::CompCurveLookupPtr table = ratioCurves2[ratioIndex[i]];
        gain_[i] =  table->lookup(in);
#else
        CompCurves::LookupPtr table = ratioCurves[ratioIndex[i]];
        gain_[i] = CompCurves::lookup(table, in);
#endif
        }
    }
    return gain_;
}
#else
inline float_4 Cmprsr::stepPoly(float_4 input, float_4 detectorInput) {
    assert(wasInit());
    simd_assertMask(reduceDistortionPoly);
    assert(polySet);
    assert(cvIsPoly);

    return isLinked ? stepPolyLinked(input, detectorInput) : stepPolyMultiMono(input, detectorInput);
}
#endif

inline float_4 Cmprsr::stepPolyLinked(float_4 input, float_4 detectorInput) {
    float_4 envelope;
    {
        auto inp = detectorInput * detectorInput;

        float avg = (inp[0] + inp[1]) * .5f;
        inp[0] = avg;
        inp[1] = avg;
        // SQINFO("input=%f, %f avg=%f", input[0], input[1], avg);
        avg = (inp[2] + inp[3]) * .5f;
        inp[2] = avg;
        inp[3] = avg;

      
        lag.step(inp);
        attackFilter.step(lag.get());
        envelope = SimdBlocks::ifelse(reduceDistortionPoly, attackFilter.get(), lag.get());
        envelope = rack::simd::sqrt(envelope);
    }

    if (ratio[0] == Ratios::HardLimit) {
        gain_[0] = (envelope[0] > threshold[0]) ? threshold[0] / envelope[0] : 1.f;
    } else {
        const float level = envelope[0] * invThreshold[0];
#ifdef _FASTLOOK
        CompCurves::CompCurveLookupPtr table = ratioCurves2[ratioIndex[0]];
        gain_[0] = table->lookup(level);
#else
        CompCurves::LookupPtr table = ratioCurves[ratioIndex[0]];
        gain_[0] = CompCurves::lookup(table, level);
#endif
    }
    gain_[1] = gain_[0];
    if (ratio[2] == Ratios::HardLimit) {
        gain_[2] = (envelope[2] > threshold[2]) ? threshold[2] / envelope[2] : 1.f;
    } else {
        const float level = envelope[2] * invThreshold[2];
#ifdef _FASTLOOK
        CompCurves::CompCurveLookupPtr table = ratioCurves2[ratioIndex[2]];
        gain_[2] = table->lookup(level);
#else
        CompCurves::LookupPtr table = ratioCurves[ratioIndex[2]];
        gain_[2] = CompCurves::lookup(table, level);
#endif
    }
    gain_[3] = gain_[2];
    return gain_ * input;
}

inline float_4 Cmprsr::stepPolyMultiMono(float_4 input, float_4 detectorInput) {
    float_4 envelope;
   
    {
        auto inp = detectorInput * detectorInput;

        lag.step(inp);
        attackFilter.step(lag.get());
        envelope = SimdBlocks::ifelse(reduceDistortionPoly, attackFilter.get(), lag.get());
        envelope = rack::simd::sqrt(envelope);
    }

    // have to do the rest non-simd - in case the curves are all different.
    // TODO: optimized case for all curves the same
    for (int iChan = 0; iChan < 4; ++iChan) {
        if (ratio[iChan] == Ratios::HardLimit) {
            // const float reductionGain = threshold[iChan] / envelope[iChan];
            gain_[iChan] = (envelope[iChan] > threshold[iChan]) ? threshold[iChan] / envelope[iChan] : 1.f;
        } else {
            const float level = envelope[iChan] * invThreshold[iChan];
#ifdef _FASTLOOK
            CompCurves::CompCurveLookupPtr table = ratioCurves2[ratioIndex[iChan]];
            gain_[iChan] = table->lookup(level);
#else
            CompCurves::LookupPtr table = ratioCurves[ratioIndex[iChan]];
            gain_[iChan] = CompCurves::lookup(table, level);
#endif
        }
    }
    return gain_ * input;
}

inline void Cmprsr::setTimesPoly(float_4 attackMs, float_4 releaseMs, float sampleTime) {
    assert(polySet);
    assert(cvIsPoly);
    // simd_assertMask(enableDistortionReduction);
    const float_4 correction = 2 * M_PI;
    const float_4 releaseHz = 1000.f / (releaseMs * correction);
    //  const float_4 attackHz = 1000.f / (attackMs * correction);
    const float_4 normRelease = releaseHz * sampleTime;

    // this sets:
    // this->reduce dist
    // lag.instantAttack
    // lag.release
    // lag.attack
    // attackFilter.cutoff

    // this->reduceDistortionPoly = SimdBlocks::ifelse( attackMs < float_4(.1f), SimdBlocks::maskFalse(), enableDistortionReduction);

    // let's had code on for poly. It's always on anyway.
#if 0
    this->reduceDistortionPoly = SimdBlocks::maskTrue();
    lag.setInstantAttackPoly(attackMs < float_4(.1f));

    lag.setAttackPoly(attackHz * sampleTime);
    attackFilter.setCutoffPoly(attackHz * sampleTime);
    lag.setReleasePoly(releaseHz * sampleTime);
#endif

#if 1

    // for now, just do the scalar code 4 times
    // build these up
    float_4 instantAttack;
    float_4 lagAttack = 10;
    float_4 filterAttack = 10;
    float_4 reduceDistortion;

    for (int i = 0; i < 4; ++i) {
        if (attackMs[i] < .1) {
            reduceDistortion[i] = SimdBlocks::maskFalse()[0];  // no way to do this at zero attack
            instantAttack[i] = SimdBlocks::maskTrue()[0];
        } else {
            reduceDistortion[i] = SimdBlocks::maskTrue()[0];
            const float correction = 2 * M_PI;
            const float attackHz = 1000.f / (attackMs[i] * correction);
            const float normAttack = attackHz * sampleTime;
            lagAttack[i] = 4 * normAttack;
            filterAttack[i] = normAttack;
            instantAttack[i] = SimdBlocks::maskFalse()[0];
        }
    }
    lag.setAttackPoly(lagAttack);
    lag.setInstantAttackPoly(instantAttack);
    attackFilter.setCutoffPoly(filterAttack);
    lag.setReleasePoly(normRelease);
#endif

#if 0
    for (int i = 0; i < 4; ++i) {
        if (attackMs[i] < .1) {
            reduceDistortion = false;  // no way to do this at zero attack
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
    }
#endif
    // updateProcFun();
}

inline void Cmprsr::setTimes(float attackMs, float releaseMs, float sampleTime) {
    assert(polySet);
    assert(!cvIsPoly);
    const float correction = 2 * M_PI;
    const float releaseHz = 1000.f / (releaseMs * correction);
    const float normRelease = releaseHz * sampleTime;

    if (attackMs < .1) {
        reduceDistortion = false;  // no way to do this at zero attack
        lag.setInstantAttack(true);
        lag.setRelease(normRelease);
    } else {
        reduceDistortion = true;
        const float correction = 2 * M_PI;
        float attackHz = 1000.f / (attackMs * correction);
        lag.setInstantAttack(false);

        const float normAttack = attackHz * sampleTime;

        lag.setAttack(normAttack * 4);
        attackFilter.setCutoff(normAttack * 1);
    }

    lag.setRelease(normRelease);
    updateProcFun();
}

inline Cmprsr::Cmprsr() {
    const float softKnee = 12;

    gain_ = float_4(1);

    if (wasInit()) {
        return;
    }

    for (int i = int(Ratios::NUM_RATIOS) - 1; i >= 0; --i) {
        Ratios ratio = Ratios(i);
        switch (ratio) {
            case Ratios::HardLimit:
                // just need to have something here
                ratioCurves[i] = ratioCurves[int(Ratios::_4_1_hard)];
                ratioCurves2[i] = ratioCurves2[int(Ratios::_4_1_hard)];
                assert(wasInit());
                break;
            case Ratios::_2_1_soft: {
                CompCurves::Recipe r;
                r.ratio = 2;
                r.kneeWidth = softKnee;
                ratioCurves[i] = CompCurves::makeCompGainLookup(r);
                ratioCurves2[i] = CompCurves::makeCompGainLookup2(r);
            } break;
            case Ratios::_2_1_hard: {
                CompCurves::Recipe r;
                r.ratio = 2;
                ratioCurves[i] = CompCurves::makeCompGainLookup(r);
                ratioCurves2[i] = CompCurves::makeCompGainLookup2(r);
            } break;
            case Ratios::_4_1_soft: {
                CompCurves::Recipe r;
                r.ratio = 4;
                r.kneeWidth = softKnee;
                ratioCurves[i] = CompCurves::makeCompGainLookup(r);
                ratioCurves2[i] = CompCurves::makeCompGainLookup2(r);
            } break;
            case Ratios::_4_1_hard: {
                CompCurves::Recipe r;
                r.ratio = 4;
                ratioCurves[i] = CompCurves::makeCompGainLookup(r);
                ratioCurves2[i] = CompCurves::makeCompGainLookup2(r);
            } break;
            case Ratios::_8_1_soft: {
                CompCurves::Recipe r;
                r.ratio = 8;
                r.kneeWidth = softKnee;
                ratioCurves[i] = CompCurves::makeCompGainLookup(r);
                ratioCurves2[i] = CompCurves::makeCompGainLookup2(r);
            } break;
            case Ratios::_8_1_hard: {
                CompCurves::Recipe r;
                r.ratio = 8;
                ratioCurves[i] = CompCurves::makeCompGainLookup(r);
                ratioCurves2[i] = CompCurves::makeCompGainLookup2(r);
            } break;
            case Ratios::_20_1_soft: {
                CompCurves::Recipe r;
                r.ratio = 20;
                r.kneeWidth = softKnee;
                ratioCurves[i] = CompCurves::makeCompGainLookup(r);
                ratioCurves2[i] = CompCurves::makeCompGainLookup2(r);
            } break;
            case Ratios::_20_1_hard: {
                CompCurves::Recipe r;
                r.ratio = 20;
                ratioCurves[i] = CompCurves::makeCompGainLookup(r);
                ratioCurves2[i] = CompCurves::makeCompGainLookup2(r);
            } break;
            default:
                assert(false);
        }
    }
    assert(wasInit());
}

inline const std::vector<std::string>& Cmprsr::ratios() {
    assert(int(Ratios::NUM_RATIOS) == 9);
    static const std::vector<std::string> theRatios = {"Limit", "2:1 soft", "2:1 hard", "4:1 soft", "4:1 hard", "8:1 soft", "8:1 hard", "20:1 soft", "20:1 hard"};
    return theRatios;
}

inline const std::vector<std::string>& Cmprsr::ratiosLong() {
    assert(int(Ratios::NUM_RATIOS) == 9);
    static const std::vector<std::string> theRatios = {"Infinite (limiter)", "2:1 soft-knee", "2:1 hard-knee", "4:1 soft-knee", "4:1 har-kneed", "8:1 soft-knee", "8:1 hard-knee", "20:1 soft-knee", "20:1 hard-knee"};
    return theRatios;
}

inline const MultiLag2& Cmprsr::_lag() const {
    return lag;
}

inline void Cmprsr::setThreshold(float th) {
    assert(polySet);
    assert(!cvIsPoly);
    threshold = th;
    setThresholdPolySub(float_4(th));
}

inline void Cmprsr::setThresholdPoly(float_4 th) {
    assert(polySet);
    assert(cvIsPoly);
    setThresholdPolySub(th);
}

inline void Cmprsr::setThresholdPolySub(float_4 th) {
    threshold = th;
    invThreshold = 1.f / threshold;
}