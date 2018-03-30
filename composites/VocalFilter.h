#pragma once
#include <algorithm>
#include <cmath>
#include "AudioMath.h"
#include "StateVariableFilter.h"
#include "LookupTable.h"
#include "FormantTables2.h"

/**
 *
 */
template <class TBase>
class VocalFilter : public TBase
{
public:
    typedef float T;
    static const int numFilters = FormantTables2::numFormantBands;

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
        FILTER_Q_TRIM_PARAM,
        FILTER_FC_PARAM,
        FILTER_FC_TRIM_PARAM,
        FILTER_VOWEL_PARAM,
        FILTER_VOWEL_TRIM_PARAM,
        FILTER_MODEL_SELECT_PARAM,
        FILTER_BRIGHTNESS_PARAM,
        FILTER_BRIGHTNESS_TRIM_PARAM,
        NUM_PARAMS
    };

    enum InputIds
    {
        AUDIO_INPUT,
        FILTER_Q_CV_INPUT,
        FILTER_FC_CV_INPUT,
        FILTER_VOWEL_CV_INPUT,
        FILTER_BRIGHTNESS_INPUT,
        NUM_INPUTS
    };

    enum OutputIds
    {
        AUDIO_OUTPUT,
        NUM_OUTPUTS
    };

    enum LightIds
    {
        LED_A,
        LED_E,
        LED_I,
        LED_O,
        LED_U,
        NUM_LIGHTS
    };

    void init();
    void step();


    float reciprocalSampleRate;

    // The frequency inputs to the filters, exposed for testing.

    T filterFrequencyLog[numFilters];

    StateVariableFilterState<T> filterStates[numFilters];
    StateVariableFilterParams<T> filterParams[numFilters];

    FormantTables2 formantTables;
    LookupTableParams<T> expLookup;
 
    AudioMath::ScaleFun<T> scaleCV_to_formant;
    AudioMath::ScaleFun<T> scaleQ;
    AudioMath::ScaleFun<T> scaleFc;
};

template <class TBase>
inline void VocalFilter<TBase>::init()
{
    for (int i = 0; i < numFilters; ++i) {
        filterParams[i].setMode(StateVariableFilterParams<T>::Mode::BandPass);
        filterParams[i].setQ(15);           // or should it be 5?

        filterParams[i].setFreq(T(.1));
    }
    scaleCV_to_formant = AudioMath::makeScaler<T>(0, formantTables.numVowels - 1);
  //  scaleQ = AudioMath::makeScaler<T>(.71f, 21);
    scaleFc = AudioMath::makeScaler<T>(-2, 2);

    AudioMath::ScaleFun<T> rawQKnob = AudioMath::makeScaler<T>(-1, 1);
    scaleQ = [rawQKnob](T cv, T param, T trim) {
        T temp = rawQKnob(cv, param, trim);
        return (temp >= 0) ?
            1 - 3 * temp / 4 :
            1 - temp;
    };

    // make table of 2 ** x
    LookupTable<T>::makeExp2(expLookup);
}

template <class TBase>
inline void VocalFilter<TBase>::step()
{
    // TODO - need 4 position switch for all models
    int model = 0;
    const T switchVal = TBase::params[FILTER_MODEL_SELECT_PARAM].value;
    if (switchVal < .5) {
        model = 0;
        assert(switchVal > -.5);
    } else if (switchVal < 1.5) {
        model = 1;
    } else if (switchVal < 2.5) {
        model = 2;
    } else {
        model = 3;
        assert(switchVal < 3.5);
    }

    const T fVowel = scaleCV_to_formant(
        TBase::inputs[FILTER_VOWEL_CV_INPUT].value,
        TBase::params[FILTER_VOWEL_PARAM].value,
        TBase::params[FILTER_VOWEL_TRIM_PARAM].value);


    int iVowel = int(fVowel);

    assert(iVowel >= 0);
    if (iVowel >= formantTables.numVowels) {
        printf("formant overflow %f\n", fVowel);
        iVowel = formantTables.numVowels - 1;
    }
    for (int i = LED_A; i <= LED_U; ++i) {
        TBase::lights[i].value = (i == iVowel) ? T(10) : T(0);
    }



  
    const T bwMultiplier = scaleQ(
        TBase::inputs[FILTER_Q_CV_INPUT].value,
        TBase::params[FILTER_Q_PARAM].value,
        TBase::params[FILTER_Q_TRIM_PARAM].value);
   // printf("bwMultiplier = %f\n", bwMultiplier);
        

    const T fPara = scaleFc(
        TBase::inputs[FILTER_FC_CV_INPUT].value,
        TBase::params[FILTER_FC_PARAM].value,
        TBase::params[FILTER_FC_TRIM_PARAM].value);
    // fNow -5..5, log

    T input = TBase::inputs[AUDIO_INPUT].value;
    T filterMix = 0;
    for (int i = 0; i < numFilters; ++i) {
        const T fcLog = formantTables.getLogFrequency(model, i, fVowel);
        const T normalizedBw = bwMultiplier * formantTables.getNormalizedBandwidth(model, i, fVowel);

        // Get the filter gain from the table, but scale by BW to counteract the filters 
        // gain that tracks Q
        const T gain = formantTables.getGain(model, i, fVowel) * normalizedBw;

        T fcFinalLog = fcLog + fPara;
       // T fcFinal = T(std::pow(2, fcFinalLog));
        T fcFinal = LookupTable<T>::lookup(expLookup, fcFinalLog);
        //printf("fcFinal=%f, look=%f\n", fcFinal, fcFinalLook);

        filterParams[i].setFreq(fcFinal * reciprocalSampleRate);
        filterParams[i].setNormalizedBandwidth(normalizedBw);
        filterMix += gain * StateVariableFilter<T>::run(input, filterStates[i], filterParams[i]);
    }
    TBase::outputs[AUDIO_OUTPUT].value = 3 * filterMix;
}