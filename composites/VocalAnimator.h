#pragma once

#include "MultiModOsc.h"
#include "StateVariableFilter.h"

/*
mdInitFreq[0] = 522.0;
mdModSens[0] = .8;

mdInitFreq[1] = 1340.0;
mdModSens[1] = .75;

mdInitFreq[2] = 2570.0;
mdModSens[2] = .25;

mdInitFreq[3] = 3700.0;
mdModSens[3] = 0;
*/
template <class TBase>
class VocalAnimator : public TBase
{
public:
    typedef float T;
    static const int numTriangle = 4;
    static const int numModOutputs = 3;
    static const int numFilters = 4;

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
        for (int i = 0; i < numFilters; ++i) {
            filterParams[i].setQ(15);
            filterParams[i].setFreq(nominalFilterCenters[i] * reciprocolSampleRate);
        }
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

    StateVariableFilterState<T> filterStates[numFilters];
    StateVariableFilterParams<T> filterParams[numFilters];

    T nominalFilterCenters[numFilters] = {522, 1340, 2570, 3700};
};
