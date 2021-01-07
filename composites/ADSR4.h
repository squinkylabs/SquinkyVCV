/**
 * A simple 4 channel ADSR, based on ADSR16, which in turn is based on.
 *  the VCV Fundamental ADSR.
 */

#pragma once

#include "SimdBlocks.h"
#include "asserts.h"
#include "simd.h"

class ADSR4 {
public:
    /** adsr 0..1
     *  sustain is 0..1
     * 
     */
#if 1
    // the first param is the usual time constant exp mapping
    // the mult param speeds everything up by a factor of mult;
    void setA(float, float mult);
    void setD(float, float mult);
    void setS(float);
    void setR(float, float mult);

    // set relese to a specific time
    void setR_L(float seconds);
#else
    void setParams(float a, float d, float s, float r);
#endif

    /* v > 1 = on
     */
    float_4 step(const float_4& gates, float sampleTime);

private:
    // 0..1
    float_4 env = 0;
    float_4 attacking = {float_4::zero()};

    // for ADSR16, there is no CV input, so don't need separate lambdas
    // user x4 vectors just to be more efficient in processing function
    float_4 attackLambda = float_4::zero();
    float_4 decayLambda = float_4::zero();
    float_4 releaseLambda = float_4::zero();
    float_4 sustain = float_4::zero();

    // 1 ms orig, but I measure as 2. I guess depends
    // how you define it.
    const float MIN_TIME = .5e-3f;
    const float MAX_TIME = 10.f;
    const float LAMBDA_BASE = MAX_TIME / MIN_TIME;

    int channels = 0;
    //bool snap = false;

    void setLambda(float_4& output, float input, float mult);
    void setLambda_L(float_4& output, float input);
};

inline float_4 ADSR4::step(const float_4& gates, float sampleTime) {
    simd_assertMask(gates);
    // Get target and lambda for exponential decay
    const float_4 attackTarget(1.2f);
    float_4 target = SimdBlocks::ifelse(gates, SimdBlocks::ifelse(attacking, attackTarget, sustain), float_4::zero());
    float_4 lambda = SimdBlocks::ifelse(gates, SimdBlocks::ifelse(attacking, attackLambda, decayLambda), releaseLambda);

    // don't know what reasonable values are here...
    simd_assertLE(env, float_4(2));
    simd_assertGE(env, float_4(0));
    simd_assertMask(attacking);

    // Adjust env
    env += (target - env) * lambda * sampleTime;

    // Turn off attacking state if envelope is HIGH
    attacking = SimdBlocks::ifelse(env >= 1.f, float_4::zero(), attacking);
    simd_assertMask(attacking);
    // Turn on attacking state if gate is LOW
    attacking = SimdBlocks::ifelse(gates, attacking, float_4::mask());

    simd_assertMask(attacking);
    return env;
}

inline void ADSR4::setLambda(float_4& output, float input, float mult) {
    assert(input >= -.01);
    assert(input <= 1);
    float_4 x = rack::simd::clamp(input, 0.f, 1.f);
    output = rack::simd::pow(LAMBDA_BASE, -x) / MIN_TIME;
    output *= float_4(mult);
    // printf("set lambda input=%f, output=%f\n", input, output[0]); fflush(stdout);
}


// this isn't exact - but close enough?
inline void ADSR4::setLambda_L(float_4& output, float inputSec) {
     output = 100 * rack::simd::pow(100.f, -inputSec) ;
}


inline void ADSR4::setA(float t, float mult) {
    setLambda(attackLambda, t, mult);
}

inline void ADSR4::setD(float t, float mult) {
    setLambda(decayLambda, t, mult);
}

inline void ADSR4::setS(float s) {
    float_4 x = rack::simd::clamp(s, 0.f, 1.f);
    sustain = x;
}

inline void ADSR4::setR(float t, float mult) {
    setLambda(releaseLambda, t, mult);
}

inline void ADSR4::setR_L(float tSec) {
        setLambda_L(releaseLambda, tSec);
}

