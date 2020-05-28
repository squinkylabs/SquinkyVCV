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
    /** All times in milliseocnds. (no they aren't! more TBD)
     *  sustain is 0..1
     * 
     */
    void setA(float attacksMilliseconds);
    void setD(float decaysMilliseconds);
    void setS(float sustains);
    void setR(float releasesMilliseconds);

    /* v > 1 = on
     */
    void step(const float_4* gates, float sampleTime);
    void setNumChannels(int ch) {
        channels = ch;
    }

    // 0..1
    float_4 env[4] = {0.f};
private:
	float_4 attacking [4] = {float_4::zero()};
	
    // for ADSR16, there is no CV input, so don't need separate lambdas
    // user x4 vectors just to be more efficient in processing function
	float_4 attackLambda = float_4::zero();
	float_4 decayLambda = float_4::zero();
	float_4 releaseLambda = float_4::zero();
	float_4 sustain = float_4::zero();

// 1 ms orig, but I measure as 2
  //  const float MIN_TIME = 1e-3f;
    const float MIN_TIME = .5e-3f;
    const float MAX_TIME = 10.f;
    const float LAMBDA_BASE = MAX_TIME / MIN_TIME;

    int channels = 0;

    void set(float_4&output, float input);

};

inline void  ADSR16::step(const float_4* gates, float sampleTime)
{ 
    for (int c = 0; c < channels; c += 4) {
        simd_assertMask(gates[c/4]);
		// Get target and lambda for exponential decay
        const float_4 attackTarget(1.2f);
		float_4 target = SimdBlocks::ifelse(gates[c / 4], SimdBlocks::ifelse(attacking[c / 4], attackTarget, sustain), float_4::zero());
		float_4 lambda = SimdBlocks::ifelse(gates[c / 4], SimdBlocks::ifelse(attacking[c / 4], attackLambda, decayLambda), releaseLambda);

#if 0

        float_4 at = attacking[c / 4];
        simd_assertMask(at);

        float_4 xx = attackLambda[c / 4];
        float_4 yy = decayLambda[c / 4];
        float_4 zz = releaseLambda[c / 4];

        simd_assertBetween(xx, float_4(-10000),float_4(10000) );
        simd_assertBetween(yy, float_4(-10000),float_4(10000) );
        simd_assertBetween(zz, float_4(-10000),float_4(10000) );

        if (lambda[0] > 10000) {
            printf("lambda overflow gates=%s\n", toStr(gates[c/4]).c_str());
            printf("lambda overflow al=%s\n", toStr(xx).c_str());
            printf("lambda overflow dl=%s\n", toStr(yy).c_str());
            printf("lambda overflow rl=%s\n", toStr(zz).c_str());
        }
        simd_assertBetween(lambda, float_4(-10000),float_4(10000) );

        
       if (c == 0) {
           printf("gates[0] = %s\n", toStr(gates[c/4]).c_str());
       }
 
     float_4 g = gates[c/4];
        printf("gattes[%d] = %s\ntarg=%s\nlamd=%s\nenv=%s\nattacking=%s\n",
            c, 
            toStrM(g).c_str(),
            toStr(target).c_str(),
            toStr(lambda).c_str(),
            toStr(env[c/4]).c_str(),
            toStrM(attacking[c/4]).c_str()
        );
#endif

		// Adjust env
		env[c / 4] += (target - env[c / 4]) * lambda * sampleTime;

        // don't know what reasonable values are here...
        simd_assertLE( env[c / 4], float_4(2));
        simd_assertGE( env[c / 4], float_4(0)); 
        simd_assertMask(attacking[c / 4]);

		// Turn off attacking state if envelope is HIGH
		attacking[c / 4] = SimdBlocks::ifelse(env[c / 4] >= 1.f, float_4::zero(), attacking[c / 4]);
        simd_assertMask(attacking[c / 4]);
		// Turn on attacking state if gate is LOW
		attacking[c / 4] = SimdBlocks::ifelse(gates[c / 4], attacking[c / 4], float_4::mask());
        
        simd_assertMask(attacking[c / 4]);
    }
}

inline void ADSR16::set(float_4&output, float input)
{
    assert(input >=0);
    assert(input <= 1);
    float_4 x = rack::simd::clamp(input, 0.f, 1.f);
    output  = rack::simd::pow(LAMBDA_BASE, -x) / MIN_TIME;  
}

inline void ADSR16::setA(float attack)
{
    set(attackLambda, attack);
}

inline void ADSR16::setD(float decay)
{
    set(decayLambda, decay);
}

inline void ADSR16::setS(float sust)
{
    assert(sust >=0);
    assert(sust <= 1);
    float_4 x = rack::simd::clamp(sust, 0.f, 1.f);
    sustain = x;
}

inline void ADSR16::setR(float release)
{
    set(releaseLambda, release);
}
