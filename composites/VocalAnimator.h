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


    /*
        LFO_RATE_PARAM,
        LFO_SPREAD_PARAM,
        */
    modulatorParams.setRateAndSpread(
        norm0_maxY(TBase::params[LFO_RATE_PARAM].value, 1),
        norm0_maxY(TBase::params[LFO_SPREAD_PARAM].value, 1),
             reciprocalSampleRate);
}
