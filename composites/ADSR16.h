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
    void step(const float_4* gates, float sampleTime);
    void setNumChannels(int ch) {
        channels = ch;
    }

    // 0..1
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

    int channels = 0;

};

inline void  ADSR16::step(const float_4* gates, float sampleTime)
{ 
    //float_4 gate[4];
    for (int c = 0; c < channels; c += 4) {
        simd_assertMask(gates[c/4]);
       // gate[c / 4] = inputs[GATE_INPUT].getVoltageSimd<float_4>(c) >= 1.f;

        // we don't have trigger inputRetrigger
        // float_4 triggered = trigger[c / 4].process(inputs[TRIG_INPUT].getPolyVoltageSimd<float_4>(c));
        // attacking[c / 4] = simd::ifelse(triggered, float_4::mask(), attacking[c / 4]);

		// Get target and lambda for exponential decay
		const float attackTarget = 1.2f;
		float_4 target = rack::simd::ifelse(gates[c / 4], rack::simd::ifelse(attacking[c / 4], attackTarget, sustain[c / 4]), 0.f);
		float_4 lambda = rack::simd::ifelse(gates[c / 4], rack::simd::ifelse(attacking[c / 4], attackLambda[c / 4], decayLambda[c / 4]), releaseLambda[c / 4]);

       
    #if 0
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

		// Turn off attacking state if envelope is HIGH
		attacking[c / 4] = rack::simd::ifelse(env[c / 4] >= 1.f, float_4::zero(), attacking[c / 4]);

		// Turn on attacking state if gate is LOW
		attacking[c / 4] = rack::simd::ifelse(gates[c / 4], attacking[c / 4], float_4::mask());

		// Set output
		//outputs[ENVELOPE_OUTPUT].setVoltageSimd(10.f * env[c / 4], c);
    }
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

inline void ADSR16::setS(float sust)
{
    float_4 x = rack::simd::clamp(sust, 0.f, 1.f);
    sustain = x;
}

inline void ADSR16::setR(float release)
{
    float_4 x = rack::simd::clamp(release, 0.f, 1.f);
    releaseLambda  = rack::simd::pow(LAMBDA_BASE, -x) / MIN_TIME;
}

