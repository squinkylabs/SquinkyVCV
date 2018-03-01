#pragma once

#include "SawOscillator.h"

template <class TBase>
class VocalAnimator : public TBase
{
public:
    typedef float T;

    //using Animator = VocalAnima

    VocalAnimator(struct Module * module) : TBase(module)
    {
    }
    VocalAnimator() : TBase()
    {
    }

    void setSampleRate(float rate)
    {
        reciprocolSampleRate = 1 / rate;
        SawOscillator<T, false>::setFrequency(lfoParams, reciprocolSampleRate);
    }

    enum ParamIds
    {
        PITCH_PARAM,      // the big pitch knob
        NUM_PARAMS
    };

    enum InputIds
    {
        AUDIO_INPUT,
        NUM_INPUTS
    };

    enum OutputIds
    {
        MAIN_OUTPUT,
        NUM_OUTPUTS
    };

    enum LightIds
    {
        NUM_LIGHTS
    };

    void init()
    {

    }
    void step()
    {
        TBase::outputs[MAIN_OUTPUT].value = SawOscillator<T, false>::runTri(lfoState, lfoParams);
    }
private:
    float reciprocolSampleRate;
    SawOscillatorParams<T> lfoParams;
    SawOscillatorState<T> lfoState;

};
