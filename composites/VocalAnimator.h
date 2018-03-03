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
        LFO_RATE_PARAM,
        LFO_SPREAD_PARAM,
        FILTER_Q_PARAM,
        FILTER_FC_PARAM,
        FILTER_MOD_DEPTH_PARAM,
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
        LFO0_LIGHT,
        LFO1_LIGHT,
        LFO2_LIGHT,
        NUM_LIGHTS
    };

    void init();
    void step();
    T modulatorOutputs[numModOutputs];

    // The frequency inputs to the filters, exposed for testing.
    // Units here are Hz.
    T filterFrequencies[numFilters];
    const T nominalFilterCenters[numFilters] = {522, 1340, 2570, 3700};
private:
    float reciprocolSampleRate;

    using osc = MultiModOsc<T, numTriangle, numModOutputs>;
    typename osc::State modulatorState;
    typename osc::Params modulatorParams;

    StateVariableFilterState<T> filterStates[numFilters];
    StateVariableFilterParams<T> filterParams[numFilters];
};

template <class TBase>
inline void VocalAnimator<TBase>::init()
{
    for (int i = 0; i < numFilters; ++i) {
        filterParams[i].setMode(StateVariableFilterParams<T>::Mode::BandPass);
        filterParams[i].setQ(15);           // or should it be 5?
 
        filterParams[i].setFreq(nominalFilterCenters[i] * reciprocolSampleRate);
        filterFrequencies[i] = nominalFilterCenters[i];
    }
}

template <class TBase>
inline void VocalAnimator<TBase>::step()
{
    // Run the modulators, hold onto their output.
    // Raw MOdulator outputs put in modulatorOutputs[].
    osc::run(modulatorOutputs, modulatorState, modulatorParams);

    // Light up the LEDs with the unscaled Modulator outputs.
    for (int i = 0; i < NUM_LIGHTS; ++i) {
        TBase::lights[i].value = modulatorOutputs[i] > 0 ? T(1.0) : 0;
    }

    // Run the filters. Output summed to filterMix.
    const T input = TBase::inputs[AUDIO_INPUT].value;
    T filterMix = 0;
    for (int i = 0; i < numFilters; ++i) {
        filterMix += StateVariableFilter<T>::run(input, filterStates[i], filterParams[i]);
    }


    TBase::outputs[AUDIO_OUTPUT].value = filterMix;
}
