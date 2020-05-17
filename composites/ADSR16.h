/**
 * A simple 4 channel ADSR, based on the VCV Fundamental ADSR
 */

#pragma once

#include "simd.h"

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
    //setGate(float_4 gates);
    void step(float_4 gates0, float_4 gates1, float_4 gates2, float_4 gates3);
    float_4 env[4] = {0.f};
private:
	float_4 attacking [4] = {float_4::zero()};
	
    // for ADSR16, there is no CV input, so don't need separate lambdas
	float_4 attackLambda = {0.f};
	float_4 decayLambda = {0.f};
	float_4 releaseLambda = {0.f};
	float_4 sustain = {0.f};

    const float MIN_TIME = 1e-3f;
    const float MAX_TIME = 10.f;
    const float LAMBDA_BASE = MAX_TIME / MIN_TIME;

};

inline void  ADSR16::step(float_4 gates0, float_4 gates1, float_4 gates2, float_4 gates3)
{ 
}

/*
	// CV
				float_4 attack = attackParam + inputs[ATTACK_INPUT].getPolyVoltageSimd<float_4>(c) / 10.f;
				float_4 decay = decayParam + inputs[DECAY_INPUT].getPolyVoltageSimd<float_4>(c) / 10.f;
				float_4 sustain = sustainParam + inputs[SUSTAIN_INPUT].getPolyVoltageSimd<float_4>(c) / 10.f;
				float_4 release = releaseParam + inputs[RELEASE_INPUT].getPolyVoltageSimd<float_4>(c) / 10.f;

				attack = simd::clamp(attack, 0.f, 1.f);
				decay = simd::clamp(decay, 0.f, 1.f);
				sustain = simd::clamp(sustain, 0.f, 1.f);
				release = simd::clamp(release, 0.f, 1.f);

				attackLambda[c / 4] = simd::pow(LAMBDA_BASE, -attack) / MIN_TIME;
				decayLambda[c / 4] = simd::pow(LAMBDA_BASE, -decay) / MIN_TIME;
				releaseLambda[c / 4] = simd::pow(LAMBDA_BASE, -release) / MIN_TIME;
				this->sustain[c / 4] = sustain;
            */


inline void ADSR16::setA(float attack)
{
    float_4 x = rack::simd::clamp(attack, 0.f, 1.f);
    attackLambda  = rack::simd::pow(LAMBDA_BASE, -x) / MIN_TIME;
}

inline void ADSR16::setD(float decay)
{
    float_4 x = rack::simd::clamp(decay, 0.f, 1.f);
    decayLambda  = rack::simd::pow(LAMBDA_BASE, -x) / MIN_TIME;
}

inline void ADSR16::setS(float attacksMilliseconds)
{
   
}

inline void ADSR16::setR(float release)
{
    float_4 x = rack::simd::clamp(release, 0.f, 1.f);
    releaseLambda  = rack::simd::pow(LAMBDA_BASE, -x) / MIN_TIME;
}

