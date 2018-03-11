#pragma once
#include <algorithm>

#include "AudioMath.h"
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
        modulatorParams.setRateAndSpread(.5, .5, 0, reciprocalSampleRate);
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

    float reciprocalSampleRate;

    using osc = MultiModOsc<T, numTriangle, numModOutputs>;
    typename osc::State modulatorState;
    typename osc::Params modulatorParams;

    StateVariableFilterState<T> filterStates[numFilters];
    StateVariableFilterParams<T> filterParams[numFilters];

    // We need a bunch of scalers to convert knob, CV, trim into the voltage 
    // range each parameter needs.
    AudioMath::ScaleFun<T> scale0_1;
    AudioMath::ScaleFun<T> scale0_2;
    AudioMath::ScaleFun<T> scaleQ;
    AudioMath::ScaleFun<T> scalen5_5;
};

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
    scale0_1 = AudioMath::makeScaler<T>(0, 1); // full CV range -> 0..1
    scale0_2 = AudioMath::makeScaler<T>(0, 2); // full CV range -> 0..2
    scaleQ = AudioMath::makeScaler<T>(.71f, 21);
    scalen5_5 = AudioMath::makeScaler<T>(-5, 5);
}

template <class TBase>
inline void VocalAnimator<TBase>::step()
{
    const bool bass = TBase::params[BASS_EXP_PARAM].value > .5;
    const auto mode = bass ?
        StateVariableFilterParams<T>::Mode::LowPass :
        StateVariableFilterParams<T>::Mode::BandPass;

    for (int i = 0; i < numFilters - 1; ++i) {
        filterParams[0].setMode(mode);
    }

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
    const T q = scaleQ(
        TBase::inputs[FILTER_Q_CV_INPUT].value,
        TBase::params[FILTER_Q_PARAM].value,
        TBase::params[FILTER_Q_TRIM_PARAM].value);

    const T fc = scalen5_5(
        TBase::inputs[FILTER_FC_CV_INPUT].value,
        TBase::params[FILTER_FC_PARAM].value,
        TBase::params[FILTER_FC_TRIM_PARAM].value);


    // put together a mod depth param from all the inputs
    // range is 0..1

    // cv, knob, trim
    const T baseModDepth = scale0_1(
        TBase::inputs[FILTER_MOD_DEPTH_CV_INPUT].value,
        TBase::params[FILTER_MOD_DEPTH_PARAM].value,
        TBase::params[FILTER_MOD_DEPTH_TRIM_PARAM].value);

    // tracking:
    //  0 = all 1v/oct, mod scaled, no on top
    //  1 = mod and cv scaled
    //  2 = 1, + top filter gets some mod
    int cvScaleMode = 0;
    const float cvScaleParam = TBase::params[TRACK_EXP_PARAM].value;
    if (cvScaleParam < .5) {
        cvScaleMode = 0;
    } else if (cvScaleParam < 1.5) {
        cvScaleMode = 1;
    } else {
        cvScaleMode = 2;
        assert(cvScaleParam < 2.5);
    }
    //printf("cvscale mode = %d\n", cvScaleMode);


    const T input = TBase::inputs[AUDIO_INPUT].value;
    T filterMix = 0;                // Sum the folder outputs here

    for (int i = 0; i < numFilters; ++i) {
        T logFreq = nominalFilterCenterLog2[i];

        switch (cvScaleMode) {
            case 1:
                // In this mode (1) CV comes straight through at 1V/8
                // Even on the top (fixed) filter
                logFreq += fc;    // add without attenuation for 1V/octave
                break;
            case 0:
                // In mode (0) CV gets scaled per filter, as in the original design.
                // Since nominalModSensitivity[3] == 0, top doesn't track
                logFreq += fc * nominalModSensitivity[i];
                break;
            case 2:
                if (fc < 0) {
                    // Normal scaling for Fc less than nominal
                    logFreq += fc * nominalModSensitivity[i];
                } else {
                    // above nominal, they all track the high one (so they don't cross)
                    logFreq += fc * nominalModSensitivity[2];
                }

                break;
            default:
                assert(false);
        }

        // First three filters always modulated,
        // (wanted to try modulating 4, but don't have an LFO (yet
        const bool modulateThisFilter = (i < 3);
        if (modulateThisFilter) {
            logFreq += modulatorOutput[i] *
                baseModDepth *
                nominalModSensitivity[i];
        }

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


    int matrixMode;
    float mmParam = TBase::params[LFO_MIX_PARAM].value;
   // printf("-- mmParam = %f\n", mmParam);
    if (mmParam < .5) {
        matrixMode = 0;
    } else if (mmParam < 1.5) {
        matrixMode = 1;
    } else {
        matrixMode = 2;
        assert(mmParam < 2.5);
    }
    modulatorParams.setRateAndSpread(
        scale0_2(
        TBase::inputs[LFO_RATE_CV_INPUT].value,
        TBase::params[LFO_RATE_PARAM].value,
        TBase::params[LFO_RATE_TRIM_PARAM].value),
        scale0_2(
        0,
        TBase::params[LFO_SPREAD_PARAM].value,
        0),
        matrixMode,
        reciprocalSampleRate);
}