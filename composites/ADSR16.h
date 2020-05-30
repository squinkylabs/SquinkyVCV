/**
 * A simple 4 channel ADSR, based on the VCV Fundamental ADSR
 */

#pragma once

#include "simd.h"
#include "SimdBlocks.h"
#include "asserts.h"

class ADSR16
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
    void setParams(float a, float d, float s, float r, float k);
#endif

    /* v > 1 = on
     */
    void step(const float_4* gates, float sampleTime);
    void setNumChannels(int ch) {
        channels = ch;
    }

    float_4 get(int bank) const;
private:
 // 0..1
    float_4 env[4] = {0.f};
	float_4 attacking [4] = {float_4::zero()};
	
    // for ADSR16, there is no CV input, so don't need separate lambdas
    // user x4 vectors just to be more efficient in processing function
	float_4 attackLambda = float_4::zero();
	float_4 decayLambda = float_4::zero();
	float_4 releaseLambda = float_4::zero();
	float_4 sustain = float_4::zero();

    /* These two do something when snap is on
     */
    float_4 clipValue = float_4(1);
    float_4 makeupGain = float_4(1);

    // 1 ms orig, but I measure as 2. I guess depends 
    // how you define it.
    const float MIN_TIME = .5e-3f;
    const float MAX_TIME = 10.f;
    const float LAMBDA_BASE = MAX_TIME / MIN_TIME;

    int channels = 0;
    bool snap = false;

    void setLambda(float_4&output, float input);
};

inline float_4 ADSR16::get(int bank) const
{
#if 0
    printf("\nin get, raw env = %s\n", toStr(env[bank]).c_str());
    printf("in get,clip = %s\n", toStr(clipValue).c_str());
    printf("in get,makeup = %s\n", toStr(makeupGain).c_str());
    printf("in get,ret = %s\n", toStr(makeupGain * SimdBlocks::min(env[bank], clipValue)).c_str());
#endif
    return makeupGain * SimdBlocks::min(env[bank], clipValue);
}

inline void  ADSR16::step(const float_4* gates, float sampleTime)
{ 
    assert(channels);               // at least for unit tests, we don't expect to be called this way. would be a bug in a test.
  //  float max = snap ? (.5 * (1 + sustain[0]) : 2);
 //   float_4 maxLimit(max);
   // printf("max limit = %s\n", toStr(maxLimit).c_str());
    for (int c = 0; c < channels; c += 4) {
        simd_assertMask(gates[c/4]);
		// Get target and lambda for exponential decay
        const float_4 attackTarget(1.2f);
		float_4 target = SimdBlocks::ifelse(gates[c / 4], SimdBlocks::ifelse(attacking[c / 4], attackTarget, sustain), float_4::zero());
		float_4 lambda = SimdBlocks::ifelse(gates[c / 4], SimdBlocks::ifelse(attacking[c / 4], attackLambda, decayLambda), releaseLambda);

        // don't know what reasonable values are here...
        simd_assertLE( env[c / 4], float_4(2));
        simd_assertGE( env[c / 4], float_4(0)); 
        simd_assertMask(attacking[c / 4]);

        // Adjust env
		env[c / 4] += (target - env[c / 4]) * lambda * sampleTime;

		// Turn off attacking state if envelope is HIGH
		attacking[c / 4] = SimdBlocks::ifelse(env[c / 4] >= 1.f, float_4::zero(), attacking[c / 4]);
        simd_assertMask(attacking[c / 4]);
		// Turn on attacking state if gate is LOW
		attacking[c / 4] = SimdBlocks::ifelse(gates[c / 4], attacking[c / 4], float_4::mask());
        
        simd_assertMask(attacking[c / 4]);
    //    if (c == 0) {
     //       printf("adsr step %s\n", toStr(env[0]).c_str()); fflush(stdout);
     //   }
    }
}

inline void ADSR16::setLambda(float_4&output, float input)
{

    assert(input >= -.01);
    assert(input <= 1);
    float_4 x = rack::simd::clamp(input, 0.f, 1.f);
    output  = rack::simd::pow(LAMBDA_BASE, -x) / MIN_TIME;  
}


inline  void ADSR16::setParams(float a, float d, float s, float r, float k)
{
#if 0
    printf("setParams a=%f, d=%f, s=%f r=%f\n", a, d, s, r);
    fflush(stdout);
#endif
    setLambda(attackLambda, a);
    setLambda(decayLambda, d);
    setLambda(releaseLambda, r);



  //  assert(s >=0);
 //   assert(s <= 1);
    float_4 x = rack::simd::clamp(s, 0.f, 1.f);
    sustain = x;
    //printf("set sutain to %s\n", toStr(sustain).c_str());

   

    float clipLevel = s + k * (1 - s);
    //printf("set adsr param, clip level = %f\n", clipLevel);
    clipLevel = std::max(clipLevel, .001f);
    clipValue = float_4(clipLevel);
    makeupGain = float_4(1 /clipLevel);
    //printf("c=%s, mg=%s\n", toStr(clipValue).c_str(), toStr(makeupGain).c_str());
}

