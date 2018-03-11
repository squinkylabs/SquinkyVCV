#pragma once
#include <algorithm>

#include "AudioMath.h"
#include "StateVariableFilter.h"




/**
 * Version 2 - make the math sane.
 */
template <class TBase>
class VocalFilter : public TBase
{
public:
    typedef float T;
    static const int numFilters = 4;

    VocalFilter(struct Module * module) : TBase(module)
    {
    }
    VocalFilter() : TBase()
    {
    }

    void setSampleRate(float rate)
    {
        reciprocalSampleRate = 1 / rate;
    }

    enum ParamIds
    {
        FILTER_Q_PARAM,
        FILTER_FC_PARAM,
        FILTER_VOWEL_PARAM,
        NUM_PARAMS
    };

    enum InputIds
    {
        AUDIO_INPUT,
        FILTER_Q_CV_INPUT,
        FILTER_FC_CV_INPUT,
        FILTER_VOWEL_INPUT,
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

    void init();
    void step();


    float reciprocalSampleRate;

    // The frequency inputs to the filters, exposed for testing.

    T filterFrequencyLog[numFilters];
#if 0
    const T nominalFilterCenterHz[numFilters] = {522, 1340, 2570, 3700};
    const T nominalFilterCenterLog2[numFilters] = {
        std::log2(T(522)),
        std::log2(T(1340)),
        std::log2(T(2570)),
        std::log2(T(3700))
    };
            // 1, .937 .3125
    const T nominalModSensitivity[numFilters] = {T(1), T(.937), T(.3125), 0};

    // Following are for unit tests.
    T normalizedFilterFreq[numFilters];
    bool jamModForTest = false;
    T   modValueForTest = 0;



    using osc = MultiModOsc<T, numTriangle, numModOutputs>;
    typename osc::State modulatorState;
    typename osc::Params modulatorParams;
#endif

    StateVariableFilterState<T> filterStates[numFilters];
    StateVariableFilterParams<T> filterParams[numFilters];

#if 0
    // We need a bunch of scalers to convert knob, CV, trim into the voltage 
    // range each parameter needs.
    AudioMath::ScaleFun<T> scale0_1;
    AudioMath::ScaleFun<T> scale0_2;
    AudioMath::ScaleFun<T> scaleQ;
    AudioMath::ScaleFun<T> scalen5_5;
#endif
    T maleF1Formants[6] = {270, 530, 660, 730, 570, 300};
    T maleF2Formants[6] = {2290,
        1840,
        1720,
        1090,
        840,
        870};
    T maleF3Formants[6] = {3010,
        2480,
        2410,
        2440,
        2410,
        2240};
    T femaleF1Formants[6] = {310,
        610,
        860,
        850,
        590,
        370};
    T femaleF2Formants[6] = {2790,
        2330,
        2050,
        1220,
        920,
        950};
    T femaleF3Formants[6] = {3310,
        2990,
        2850,
        2810,
        2710,
        2670};

    T childF1Formants[6] = {370,
        690,
        1010,
        1030,
        680,
        430};
    T childF2Formants[6] = {3200,
        2610,
        2320,
        1370,
        1060,
        1170};

    T childF3Formants[6] = {3730,
        3570,
        3320,
        3170,
        3180,
        3260};


};

template <class TBase>
inline void VocalFilter<TBase>::init()
{
    for (int i = 0; i < numFilters; ++i) {
        filterParams[i].setMode(StateVariableFilterParams<T>::Mode::BandPass);
        filterParams[i].setQ(15);           // or should it be 5?

        // TODO
        filterParams[i].setFreq(T(.1));
        //filterParams[i].setFreq(nominalFilterCenterHz[i] * reciprocalSampleRate);
        //filterFrequencyLog[i] = nominalFilterCenterLog2[i];

       // normalizedFilterFreq[i] = nominalFilterCenterHz[i] * reciprocalSampleRate;
    }

}

template <class TBase>
inline void VocalFilter<TBase>::step()
{
    T input = TBase::inputs[AUDIO_INPUT].value;
    T filterMix = 0;
    for (int i = 0; i < numFilters; ++i) {
        filterMix += StateVariableFilter<T>::run(input, filterStates[i], filterParams[i]);
    }
    TBase::outputs[AUDIO_OUTPUT].value = filterMix;
}