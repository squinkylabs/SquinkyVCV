/**
 * A simple 4 channel ADSR, based on ADSR4, which is based on ADSR16, which in turn is based on.
 *  the VCV Fundamental ADSR.
 */

#pragma once

#include "SimdBlocks.h"
#include "SqLog.h"
#include "asserts.h"
#include "simd.h"

class ADSRSampler {
public:
    /** adsr 0..1
     *  sustain is 0..1
     * 
     */

    void setASec(float);
    void setDSec(float);
    void setS(float);
    void setRSec(float);

    /* gates are simd mask encode bools
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
   // const float MIN_TIME = .5e-3f;
   // const float MAX_TIME = 10.f;
   // const float LAMBDA_BASE = MAX_TIME / MIN_TIME;

    int channels = 0;
    void setLambda(float_4& output, float input);
};

inline float_4 ADSRSampler::step(const float_4& gates, float sampleTime) {
    simd_assertMask(gates);
    // Get target and lambda for exponential decay
    const float_4 attackTarget(1.2f);
    float_4 target = SimdBlocks::ifelse(gates, SimdBlocks::ifelse(attacking, attackTarget, sustain), float_4::zero());
    float_4 lambda = SimdBlocks::ifelse(gates, SimdBlocks::ifelse(attacking, attackLambda, decayLambda), releaseLambda);

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

inline void ADSRSampler::setLambda(float_4& output, float inputSec) {
    // was 10, but was too slow
    float x = 10.f / inputSec;
    float_4 x4(x);
    output = x4;
}

inline void ADSRSampler::setASec(float t) {
  //  SQINFO("set attack %f sec", t);

    // A bit of a hack, but this makes our "truncated exp"
    // attack come out somewhere near where the sfz
    // linear attack would be.
    t *= 6.25f;
    setLambda(attackLambda, t);
  //  SQINFO("lambda now %f", attackLambda[0]);
}

inline void ADSRSampler::setDSec(float t) {
  //  SQINFO("set decay %f sec", t);
    setLambda(decayLambda, t);
  //  SQINFO("lambda now %f", decayLambda[0]);
}

inline void ADSRSampler::setS(float s) {
    float_4 x = rack::simd::clamp(s, 0.f, 1.f);
    sustain = x;
}

inline void ADSRSampler::setRSec(float t) {
  //  SQINFO("set release %f sec", t);
    setLambda(releaseLambda, t);
  //  SQINFO("lambda now %f", releaseLambda[0]);
}
