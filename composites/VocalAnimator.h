#pragma once
#include <algorithm>

#include "MultiModOsc.h"
#include "StateVariableFilter.h"




/**
 * Version 2 - make the math sane.
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
        reciprocalSampleRate = 1 / rate;
        modulatorParams.setRateAndSpread(.5, .5, reciprocalSampleRate);
    }

    enum ParamIds
    {
        LFO_RATE_PARAM,
        LFO_SPREAD_PARAM,
        FILTER_Q_PARAM,
        FILTER_FC_PARAM,
        FILTER_MOD_DEPTH_PARAM,
        LFO_RATE_TRIM_PARAM,
        FILTER_Q_TRIM_PARAM,
        FILTER_FC_TRIM_PARAM,
        FILTER_MOD_DEPTH_TRIM_PARAM,
        BASS_EXP_PARAM,

        // tracking:
        //  0 = all 1v/oct, mod scaled, no on top
        //  1 = mod and cv scaled
        //  2 = 1, + top filter gets some mod
        TRACK_EXP_PARAM,

        // LFO mixing options
        // 0 = classic
        // 1 = option
        // 2 = lf sub
        LFO_MIX_PARAM,



        NUM_PARAMS
    };

    enum InputIds
    {
        AUDIO_INPUT,
        LFO_RATE_CV_INPUT,
        FILTER_Q_CV_INPUT,
        FILTER_FC_CV_INPUT,
        FILTER_MOD_DEPTH_CV_INPUT,
        NUM_INPUTS
    };

    enum OutputIds
    {
        AUDIO_OUTPUT,
        LFO0_OUTPUT,
        LFO1_OUTPUT,
        LFO2_OUTPUT,
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
    T modulatorOutput[numModOutputs];

    // The frequency inputs to the filters, exposed for testing.
 
    T filterFrequencyLog[numFilters];

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
private:
    float reciprocalSampleRate;

    using osc = MultiModOsc<T, numTriangle, numModOutputs>;
    typename osc::State modulatorState;
    typename osc::Params modulatorParams;

    StateVariableFilterState<T> filterStates[numFilters];
    StateVariableFilterParams<T> filterParams[numFilters];

    /**
    * Normalize =-5..5 => 0..maxY
    * Clip values outside
    */
#if 1
    T norm0_maxY(T x, T maxY)
    {
        x += 5;
        if (x < 0) x = -0;
        if (x > 10) x = 10;
        return x * maxY / 10;
    }

    // -5 .. 5 -> -y...y
    T norm_pmY(T x, T maxY)
    {
        if (x < -5) x = -5;
        if (x > 5) x = 5;
        return x * maxY / 5;
    }
#endif
    
    T getAddAndScale(
        InputIds,
        ParamIds,
        ParamIds,
        T minRange, T maxRange);
};

template <class TBase>
inline float VocalAnimator<TBase>::getAddAndScale(
    InputIds inputId,
    ParamIds trimId,
    ParamIds paramId,
    T minRange, T maxRange)
{
    // results = knob + VC * trim;
    T result = TBase::inputs[inputId].value * TBase::params[trimId].value;
    result += TBase::params[paramId].value;
    result = std::max(-5.0f, result);
    result = std::min(5.0f, result);
    // not it's -5 to 5
    // TODO: precalculate A, b (use my old interp)
    const float a = (maxRange - minRange) / 10.0f;
    const float b = maxRange - 5 * a;
    
    const T scaled = a * result + b;
    assert(scaled >= minRange);
    assert(scaled <= maxRange);
    return scaled;
}

template <class TBase>
inline void VocalAnimator<TBase>::init()
{
    for (int i = 0; i < numFilters; ++i) {
        filterParams[i].setMode(StateVariableFilterParams<T>::Mode::BandPass);
        filterParams[i].setQ(15);           // or should it be 5?

        filterParams[i].setFreq(nominalFilterCenterHz[i] * reciprocalSampleRate);
        filterFrequencyLog[i] = nominalFilterCenterLog2[i];

        normalizedFilterFreq[i] = nominalFilterCenterHz[i] * reciprocalSampleRate;
    }
}

template <class TBase>
inline void VocalAnimator<TBase>::step()
{
    // Run the modulators, hold onto their output.
    // Raw Modulator outputs put in modulatorOutputs[].
    osc::run(modulatorOutput, modulatorState, modulatorParams);

    static const OutputIds LEDOutputs[] = {
        LFO0_OUTPUT,
        LFO1_OUTPUT,
        LFO2_OUTPUT,
    };
    // Light up the LEDs with the unscaled Modulator outputs.
    for (int i = 0; i < NUM_LIGHTS; ++i) {
        TBase::outputs[LEDOutputs[i]].value = modulatorOutput[i];
        TBase::lights[i].value = modulatorOutput[i] > 0 ? T(1.0) : 0;
        TBase::outputs[LEDOutputs[i]].value = modulatorOutput[i];
    }

    // norm all the params out here
    T q = TBase::params[FILTER_Q_PARAM].value + 5;          // 0..10
    q *= 2;                                                // 0..20
    q += T(.71);

    if (q < .7) {
        fprintf(stderr, "q = %f, param = %f\n", q, TBase::params[FILTER_Q_PARAM].value);
        q = 1;
    }
    assert(q >= .7);


    // put together a mod depth param from all the inputs
    // range is 0..1
    const T baseModDepth = getAddAndScale(
        FILTER_MOD_DEPTH_CV_INPUT,
        FILTER_MOD_DEPTH_TRIM_PARAM,
        FILTER_MOD_DEPTH_PARAM,
        0, 1);

    const T input = TBase::inputs[AUDIO_INPUT].value;
    T filterMix = 0;                // Sum the folder outputs here

    for (int i = 0; i < numFilters; ++i) {
        T logFreq = nominalFilterCenterLog2[i];

        // first version - everyone track straight
        // replace this with switch
#if 0
        logFreq += TBase::params[FILTER_FC_PARAM].value;    // add without attenuation for 1V/octave
#else
        // classic version - respect the mod depth scaling
        logFreq += TBase::params[FILTER_FC_PARAM].value * nominalModSensitivity[i];
#endif
        logFreq += ((i < 3) ? modulatorOutput[i] : 0) *
            baseModDepth *
            nominalModSensitivity[i];

       // fprintf(stderr, "logFreq = %f\n", logFreq); fflush(stderr);

        filterFrequencyLog[i] = logFreq;

        T normFreq = std::pow(T(2), logFreq) * reciprocalSampleRate;
       // fprintf(stderr, "nromFreq = %f\n", normFreq); fflush(stderr);
        if (normFreq > .2) {
            normFreq = T(.2);
        }
      
        normalizedFilterFreq[i] = normFreq;
       // fprintf(stderr, "nromFreq2 = %f\n", normFreq); fflush(stderr);
        filterParams[i].setFreq(normFreq);
        filterParams[i].setQ(q);
       // fprintf(stderr, "setFreq = %f\n", normFreq); fflush(stderr);

        filterMix += StateVariableFilter<T>::run(input, filterStates[i], filterParams[i]);
       // fprintf(stderr, "fran filter = %f\n", normFreq); fflush(stderr);
    }

    filterMix *= T(.3);            // attenuate to avoid clip
    TBase::outputs[AUDIO_OUTPUT].value = filterMix;

    modulatorParams.setRateAndSpread(
        norm_pmY(TBase::params[LFO_RATE_PARAM].value, 2),
        norm_pmY(TBase::params[LFO_SPREAD_PARAM].value, 1),
        reciprocalSampleRate);

}