#pragma once

#include "MultiModOsc.h"

template <class TBase>
class VocalAnimator : public TBase
{
public:
    typedef float T;
    static const int numTriangle = 4;
    static const int numModOutputs = 3;

    VocalAnimator(struct Module * module) : TBase(module)
    {
    }
    VocalAnimator() : TBase()
    {
    }

    void setSampleRate(float rate)
    {
        reciprocolSampleRate = 1 / rate;
        modulatorParams.setRateAndSpread(.5, .5, reciprocolSampleRate);
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
        T output[numModOutputs] = {0, 0, 0};

        osc::run(output, modulatorState , modulatorParams);
        TBase::outputs[MAIN_OUTPUT].value = output[0];
    }
private:
    float reciprocolSampleRate;
    using osc = MultiModOsc<T, numTriangle, numModOutputs>;
    typename osc::State modulatorState;
    typename osc::Params modulatorParams;
};
