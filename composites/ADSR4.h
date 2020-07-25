/**
 * A simple 4 channel ADSR, based on ADSR16, which in turn is based on.
 *  the VCV Fundamental ADSR.
 */

#pragma once

#include "simd.h"
#include "SimdBlocks.h"
#include "asserts.h"

class ADSR4
{
public:
    /** adsr 0..1
     *  sustain is 0..1
     * 
     */
#if 0  // old way
    void setA(float);
    void setD(float);
    void setS(float);
    void setR(float );
    void setSnap(bool b) {
        snap = b;
        clipValue = float_4(snap ? .75 : 1);
        makeupGain = float_4(snap ? 1.33f : 1); 
    }
#else
    void setParams(float a, float d, float s, float r);
#endif

    /* v > 1 = on
     */
    float_4 step(const float_4& gates, float sampleTime);
    #if 0
    void setNumChannels(int ch) {
        channels = ch;
    }
    #endif

  //  float_4 get(int bank) const;
private:
 // 0..1
    float_4 env=0;
	float_4 attacking = {float_4::zero()};
	
    // for ADSR16, there is no CV input, so don't need separate lambdas
    // user x4 vectors just to be more efficient in processing function
	float_4 attackLambda = float_4::zero();
	float_4 decayLambda = float_4::zero();
	float_4 releaseLambda = float_4::zero();
	float_4 sustain = float_4::zero();

    /* These two do something when snap is on
     */
  //  float_4 clipValue = float_4(1);
 //   float_4 makeupGain = float_4(1);

    // 1 ms orig, but I measure as 2. I guess depends 
    // how you define it.
    const float MIN_TIME = .5e-3f;
    const float MAX_TIME = 10.f;
    const float LAMBDA_BASE = MAX_TIME / MIN_TIME;

    int channels = 0;
//bool snap = false;

    void setLambda(float_4&output, float input);
};

#if 0
inline float_4 ADSR4::get(int bank) const
{
#if 0
    printf("\nin get, raw env = %s\n", toStr(env[bank]).c_str());
    printf("in get,clip = %s\n", toStr(clipValue).c_str());
    printf("in get,makeup = %s\n", toStr(makeupGain).c_str());
    printf("in get,ret = %s\n", toStr(makeupGain * SimdBlocks::min(env[bank], clipValue)).c_str());
#endif
    return makeupGain * SimdBlocks::min(env[bank], clipValue);
}
#endif

inline float_4 ADSR4::step(const float_4& gates, float sampleTime)
{ 

    simd_assertMask(gates);
    // Get target and lambda for exponential decay
    const float_4 attackTarget(1.2f);
    float_4 target = SimdBlocks::ifelse(gates, SimdBlocks::ifelse(attacking, attackTarget, sustain), float_4::zero());
    float_4 lambda = SimdBlocks::ifelse(gates, SimdBlocks::ifelse(attacking, attackLambda, decayLambda), releaseLambda);

    // don't know what reasonable values are here...
    simd_assertLE( env, float_4(2));
    simd_assertGE( env, float_4(0)); 
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

inline void ADSR4::setLambda(float_4&output, float input)
{

    assert(input >= -.01);
    assert(input <= 1);
    float_4 x = rack::simd::clamp(input, 0.f, 1.f);
    output  = rack::simd::pow(LAMBDA_BASE, -x) / MIN_TIME;  
}


inline  void ADSR4::setParams(float a, float d, float s, float r)
{
#if 0
    printf("setParams a=%f, d=%f, s=%f r=%f\n", a, d, s, r);
    fflush(stdout);
#endif
    setLambda(attackLambda, a);
    setLambda(decayLambda, d);
    setLambda(releaseLambda, r);

    float_4 x = rack::simd::clamp(s, 0.f, 1.f);
    sustain = x;
}

