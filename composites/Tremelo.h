#pragma once

#include "ClockMult.h"

/**
 */
template <class TBase>
class Tremelo : public TBase
{
public:
    Tremelo(struct Module * module) : TBase(module)
    {
    }
    Tremelo() : TBase()
    {
    }
    void setSampleRate(float rate)
    {
        reciprocalSampleRate = 1 / rate;
    }

    // must be called after setSampleRate
    void init();


   /* from vst
   		ControlValues() {
			lfoRate=3;		
			lfoShape=.5;
			lfoSkew=0;
			modDepth=0;
			lfoPhase=0;
			beatSync=0;
		}
        */
    enum ParamIds
    {
        LFO_RATE_PARAM,
        LFO_SHAPE_PARAM,
        LFO_SKEW_PARAM,
        MOD_DEPTH_PARAM,
        NUM_PARAMS
    };

    enum InputIds
    {
        AUDIO_INPUT,
        NUM_INPUTS
    };

    enum OutputIds
    {
        AUDIO_OUTPUT,
        NUM_OUTPUTS
    };

    enum LightIds
    {
        NUM_LIGHTS
    };

    /**
     * Main processing entry point. Called every sample
     */
    void step();


private:

    ClockMult clock;

    float reciprocalSampleRate = 0;;
};



template <class TBase>
inline void Tremelo<TBase>::init()
{
    clock.setDivisor(0);
}

template <class TBase>
inline void Tremelo<TBase>::step()
{

    float r = TBase::params[LFO_RATE_PARAM].value;
    // scale crudely, for now.
    r += 5;
    r *= .2;
    r += .1;
    clock.setFreeRunFreq(r * reciprocalSampleRate);
    clock.sampleClock();

    TBase::outputs[AUDIO_OUTPUT].value = clock.getSaw();   // just for now
}
