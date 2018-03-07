#pragma once

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
}

template <class TBase>
inline void VocalAnimator<TBase>::step()
{
    // Run the modulators, hold onto their output.
    // Raw Modulator outputs put in modulatorOutputs[].
    osc::run(modulatorOutput, modulatorState, modulatorParams);

    // Light up the LEDs with the unscaled Modulator outputs.
    for (int i = 0; i < NUM_LIGHTS; ++i) {
        TBase::lights[i].value = modulatorOutput[i] > 0 ? T(1.0) : 0;
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


    const T input = TBase::inputs[AUDIO_INPUT].value;
    T filterMix = 0;

    for (int i = 0; i < numFilters; ++i) {
        T logFreq = nominalFilterCenterLog2[i];

        // first version - everyone track straight
        // replace this with switch
#if 0
        logFreq += TBase::params[FILTER_FC_PARAM].value;    // add without attenuation for 1V/octave
#else
        logFreq += TBase::params[FILTER_FC_PARAM].value * nominalModSensitivity[i];
#endif
        logFreq += ((i < 3) ? modulatorOutput[i] : 0) *
            norm0_maxY(TBase::params[FILTER_MOD_DEPTH_PARAM].value, 1) *
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


#if 0 // port of the old spacelab version
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
    T filterFrequency[numFilters];
    const T nominalFilterCenter[numFilters] = {522, 1340, 2570, 3700};
    const T nominalModSensitivity[numFilters] = {T(.8), T(.75), T(.25), 0};
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
    T norm0_maxY(T x, T maxY)
    {
        x += 5;
        if (x < 0) x = -0;
        if (x > 10) x = 10;
        return x * maxY / 10;
    }
};

template <class TBase>
inline void VocalAnimator<TBase>::init()
{
    for (int i = 0; i < numFilters; ++i) {
        filterParams[i].setMode(StateVariableFilterParams<T>::Mode::BandPass);
        filterParams[i].setQ(15);           // or should it be 5?
 
        filterParams[i].setFreq(nominalFilterCenter[i] * reciprocalSampleRate);
        filterFrequency[i] = nominalFilterCenter[i];
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

    /** Compute the base filter frequency.
     * Filter cutoff parameter is the variable input.
     * After this filterFrequencies will hold the base frequencies.
     */
    const T fcInput = TBase::params[FILTER_FC_PARAM].value * T(.2); // normalize to +/-1
    for (int i = 0; i < numFilters; ++i) {
        // shift will multiply base freq to apply the FC parameter
        const T shift = (T) std::pow( 2, nominalModSensitivity[i] * 2.5 * fcInput);
        filterFrequency[i] = shift * nominalFilterCenter[i];
    }

    /** Compute the modulation depths.
     * Mod Depth parameter is the variable input.
     * normalize it to 0..1 (for now)
    */
    T depthParam = (TBase::params[FILTER_MOD_DEPTH_PARAM].value + 5) / 10;
    //printf("depth param raw %f, normalized %f\n", TBase::params[FILTER_MOD_DEPTH_PARAM].value, depthParam);
    T modDepth[numFilters];
    for (int i = 0; i < numFilters; ++i) {
        modDepth[i] = T(depthParam * (3. / 25.)
            * nominalModSensitivity[i]);
    }

    // Compute the filter frequencies, using all input parameters.
    // The last one does not get modulated
    for (int i = 0; i < numFilters-1; ++i) {
        const T mod = jamModForTest ? modValueForTest : modulatorOutputs[i];
       // const T mod = modulatorOutputs[i];
        const T mod2 = T(std::pow(2, modDepth[i] * mod ));
        //printf("i=%d lfo=%f scaled2=%f origfc=%f ", i, mod, mod2, filterFrequency[i]);
        filterFrequency[i] *= mod2;
       // printf("finalf=%f\n", filterFrequency[i]);
    }

    // TODO: Q
    T q = TBase::params[FILTER_Q_PARAM].value + 5;          // 0..10
    q *= 2;                                                // 0..20
    q += T(.7);                                            // minimum Q

    // Update the filters
    for (int i = 0; i < numFilters; ++i) {
        assert(filterFrequency[i] > 10);
        assert(filterFrequency[i] < 22000);
        filterParams[i].setFreq(filterFrequency[i] * reciprocalSampleRate);
        filterParams[i].setQ(q);
    }


    // Run the filters. Output summed to filterMix.
    const T input = TBase::inputs[AUDIO_INPUT].value;
    T filterMix = 0;
    for (int i = 0; i < numFilters; ++i) {
        filterMix += StateVariableFilter<T>::run(input, filterStates[i], filterParams[i]);
    }
    filterMix *= T(.3);            // attenuate to avoid clip
    TBase::outputs[AUDIO_OUTPUT].value = filterMix;

    modulatorParams.setRateAndSpread(
        norm0_maxY(TBase::params[LFO_RATE_PARAM].value, 1),
        norm0_maxY(TBase::params[LFO_SPREAD_PARAM].value, 1),
             reciprocalSampleRate);
}
#endif
