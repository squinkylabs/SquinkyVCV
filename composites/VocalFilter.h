#pragma once
#include <algorithm>
#include <cmath>
#include "AudioMath.h"
#include "StateVariableFilter.h"
#include "LookupTable.h"
#include "FormantTables2.h"


#if 0
/**
 * Helper class to hold all the formant data
 * (lookup tables, interpolators)
 * TODO: move this to another file and make efficient.
 */
template <typename T>
class FormantTables
{
public:
    FormantTables();
    static const int numFormants = 6;
    /**
     * Interpolates the frequency using lookups
     * @param sex = 0 (male) 1 (female) 2 (child)
     * @param index = 0..2 (formant F1..F3)
     * @param vowel is the continuous index into the per/vowel lookup tables
     */
    T getFrequency(int sex, int index, T vowel);
private:
    // formant data. we are using three formant frequencies, size vowel sounds,
    // and three models (male, female, child)
  
    const T maleF1Formants[numFormants] = {270, 530, 660, 730, 570, 300};
    const T maleF2Formants[numFormants] = {2290, 1840, 1720, 1090,  840, 870};
    const T maleF3Formants[numFormants] = {3010, 2480, 2410, 2440,  2410,2240};

    const T femaleF1Formants[numFormants] = {310, 610, 860, 850, 590, 370};
    const T femaleF2Formants[numFormants] = {2790, 2330, 2050, 1220, 920, 950};
    const T femaleF3Formants[numFormants] = {3310, 2990, 2850, 2810, 2710, 2670};

    const T childF1Formants[numFormants] = {370, 690, 1010, 1030, 680, 430};
    const T childF2Formants[numFormants] = {3200, 2610, 2320, 1370, 1060, 1170};

    const T childF3Formants[numFormants] = {3730, 3570, 3320, 3170, 3180, 3260};

    const T* maleFormants[3] = {maleF1Formants, maleF2Formants, maleF3Formants};
    const T* femaleFormants[3] = {femaleF1Formants, femaleF2Formants, femaleF3Formants};
    const T* childFormants[3] = {childF1Formants, childF2Formants, childF3Formants};

    const T** allFormants[3] = {maleFormants, femaleFormants, childFormants};

    LookupTableParams<T> maleF1lookup;
    LookupTableParams<T> maleF2lookup;
    LookupTableParams<T> maleF3lookup;
    LookupTableParams<T> femaleF1lookup;
    LookupTableParams<T> femaleF2lookup;
    LookupTableParams<T> femaleF3lookup;
    LookupTableParams<T> childF1lookup;
    LookupTableParams<T> childF2lookup;
    LookupTableParams<T> childF3lookup;

    LookupTableParams<T>* maleLookups[3] = {&maleF1lookup, &maleF2lookup, &maleF3lookup};
    LookupTableParams<T>* femaleLookups[3] = {&femaleF1lookup, &femaleF2lookup, &femaleF3lookup};
    LookupTableParams<T>* childLookups[3] = {&childF1lookup, &childF2lookup, &childF3lookup};
    LookupTableParams<T>** allLookups[3] = {maleLookups, femaleLookups, childLookups };
};

template <typename T>
FormantTables<T>::FormantTables()
{
   //initDiscrete(LookupTableParams<T>& params, int numEntries, const T * entries)
 
    for (int model = 0; model < 3; ++model) {
        // get array of all three formant lookups for this model
        const T** f1 = allFormants[model];
        LookupTableParams<T>** l1 = allLookups[model];

        for (int formantFreq = 0; formantFreq < 3; ++formantFreq) {
            // get the lookup for this model/freq
            const T* f2 = f1[formantFreq];
            LookupTableParams<T>* l2 = l1[formantFreq];

            LookupTable<T>::initDiscrete(*l2, numFormants, f2);
        }
    }
}

template <typename T>
T FormantTables<T>::getFrequency(int model, int index, T vowel)
{
    assert(model >= 0 && model <= 2);
    assert(index >= 0 && index <= 2);
    assert(vowel >= 0 && vowel < numFormants);

    LookupTableParams<T>* params = allLookups[model][index];
    return LookupTable<T>::lookup(*params, vowel);
}
#endif

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

        NUM_PARAMS
    };

    enum InputIds
    {
        AUDIO_INPUT,
        FILTER_Q_CV_INPUT,
        FILTER_FC_CV_INPUT,
        FILTER_VOWEL_CV_INPUT,
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


    StateVariableFilterState<T> filterStates[numFilters];
    StateVariableFilterParams<T> filterParams[numFilters];

    FormantTables2 formantTables;

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

        // TODO
        filterParams[i].setFreq(T(.1));
        //filterParams[i].setFreq(nominalFilterCenterHz[i] * reciprocalSampleRate);
        //filterFrequencyLog[i] = nominalFilterCenterLog2[i];

       // normalizedFilterFreq[i] = nominalFilterCenterHz[i] * reciprocalSampleRate;
    }
    scaleCV_to_formant = AudioMath::makeScaler<T>(0, formantTables.numVowels - 1);
    scaleQ = AudioMath::makeScaler<T>(.71f, 21);
    scaleFc = AudioMath::makeScaler<T>(-2, 2);

}

template <class TBase>
inline void VocalFilter<TBase>::step()
{
    // todo - need 4 position switch for all models
    int model = 0;
    const T switchVal = TBase::params[FILTER_MODEL_SELECT_PARAM].value;
    if (switchVal < .5) {
        model = 0;
        assert(switchVal > -.5);
    } else if (switchVal < 1.5) {
        model = 1;
    } else {
        model = 2;
        assert(switchVal < 2.5);
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


    /* TODO: put Q back
    const T q = scaleQ(
        TBase::inputs[FILTER_Q_CV_INPUT].value,
        TBase::params[FILTER_Q_PARAM].value,
        TBase::params[FILTER_Q_TRIM_PARAM].value);
        */

    const T fPara = scaleFc(
        TBase::inputs[FILTER_FC_CV_INPUT].value,
        TBase::params[FILTER_FC_PARAM].value,
        TBase::params[FILTER_FC_TRIM_PARAM].value);
    // fNow -5..5, log

    T input = TBase::inputs[AUDIO_INPUT].value;
    T filterMix = 0;
    for (int i = 0; i < numFilters; ++i) {
        const T fcLog = formantTables.getLogFrequency(model, i, fVowel);
        const T normalizedBw = formantTables.getNormalizedBandwidth(model, i, fVowel);

        // Get the filter gain from the table, but scale by BW to counteract the filters 
        // gain that tracks Q
        const T gain = formantTables.getGain(model, i, fVowel) * normalizedBw;

        T fcFinalLog = fcLog + fPara;
        T fcFinal = T(std::pow(2, fcFinalLog));
        filterParams[i].setFreq(fcFinal * reciprocalSampleRate);
        filterParams[i].setNormalizedBandwidth(normalizedBw);
        filterMix += gain * StateVariableFilter<T>::run(input, filterStates[i], filterParams[i]);
    }
    TBase::outputs[AUDIO_OUTPUT].value = 3 * filterMix;
}