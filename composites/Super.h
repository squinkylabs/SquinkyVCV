
#pragma once

#include "ButterworthLookup.h"
#include "BiquadState.h"
#include "BiquadFilter.h"
#include "NonUniformLookupTable.h"
#include "ObjectCache.h"

class SawtoothDetuneCurve
{
public:
    /**
    * @param depth is "detune" knob. 0..1
    * returns a number such that freq = detuneFactor * initialFreq
    */
    float getDetuneFactor(float depth)
    {
        return NonUniformLookupTable<float>::lookup(table, depth);
    }

    SawtoothDetuneCurve()
    {
        // this data is pretty regular - could use uniform table
        using T = NonUniformLookupTable<float>;
        T::addPoint(table, 0, 0);
        T::addPoint(table, .0551f, .00967f);
        T::addPoint(table, .118f, .022f);
        T::addPoint(table, .181f, .04f);
        T::addPoint(table, .244f, .0467f);
        T::addPoint(table, .307f, .059f);

        T::addPoint(table, .37f, .0714f);
        T::addPoint(table, .433f, .0838f);
        T::addPoint(table, .496f, .0967f);
        T::addPoint(table, .559f, .121f);
        T::addPoint(table, .622f, .147f);
        T::addPoint(table, .748f, .243f);
        T::addPoint(table, .811f, .293f);
        T::addPoint(table, .874f, .343f);
        T::addPoint(table, .937f, .392f);
        T::addPoint(table, 1, 1);
        NonUniformLookupTable<float>::finalize(table);
    }
private:
    NonUniformLookupTableParams<float> table;
};


template <class TBase>
class Super : public TBase
{
public:

    Super(struct Module * module) : TBase(module)
    {
    }
    Super() : TBase()
    {
    }

    /**
    * re-calc everything that changes with sample
    * rate. Also everything that depends on baseFrequency.
    *
    * Only needs to be called once.
    */
    void init();

    enum ParamIds
    {
        OCTAVE_PARAM,
        SEMI_PARAM,
        FINE_PARAM,
        DETUNE_PARAM,
        DETUNE_TRIM_PARAM,
        MIX_PARAM,
        MIX_TRIM_PARAM,
        NUM_PARAMS
    };

    enum InputIds
    {
        CV_INPUT,
        GATE_INPUT,
        DEBUG_INPUT,
        DETUNE_INPUT,
        MIX_INPUT,
        NUM_INPUTS
    };

    enum OutputIds
    {
        MAIN_OUTPUT,
        DEBUG_OUTPUT,
        NUM_OUTPUTS
    };

    enum LightIds
    {
        NUM_LIGHTS
    };

    /**
     * Main processing entry point. Called every sample
     */
    void step() override;

private:
    static const int numSaws = 7;

    float phase[numSaws] = {0};
    float phaseInc[numSaws] =  {0};
    float globalPhaseInc = 0;

    std::function<float(float)> expLookup =
        ObjectCache<float>::getExp2Ex();
    std::shared_ptr<LookupTableParams<float>> audioTaper =
        ObjectCache<float>::getAudioTaper();

    void updatePhaseInc();
    void updateAudio();

// TODO: make static
    float const detuneFactors[numSaws] = {
        .89f,
        .94f,
        .98f,
        1.f,
        1.02f,
        1.06f,
        1.107f
    };

    // For debugging filters
    BiquadState<float, 2> filterState;
    BiquadParams<float, 2> filterParams;
    void updateHPFilters();
    ButterworthLookup4PHP filterLookup;

};


template <class TBase>
inline void Super<TBase>::init()
{
}

template <class TBase>
inline void Super<TBase>::updatePhaseInc()
{

    const float cv = TBase::inputs[CV_INPUT].value;
    
    const float finePitch = TBase::params[FINE_PARAM].value / 12.0f;
    const float semiPitch = TBase::params[SEMI_PARAM].value / 12.0f;


    float pitch = 1.0f + roundf(TBase::params[OCTAVE_PARAM].value) +
            semiPitch +
            finePitch;
    
    pitch += cv;

    const float q = float(log2(261.626));       // move up to pitch range of even vco
    pitch += q;
    const float freq = expLookup(pitch);
    globalPhaseInc = TBase::engineGetSampleTime() * freq;

    for (int i=0; i<numSaws; ++i) {
        float detune = (detuneFactors[i] - 1) * .1f;
        detune += 1;
        phaseInc[i] = globalPhaseInc * detune;
       // printf("phaseINc[%d] = %f\n", i, phaseInc[i]); fflush(stdout);
    }
}

template <class TBase>
inline void Super<TBase>::updateAudio()
{
    float mix = 0;
    for (int i=0; i<numSaws; ++i) {
        phase[i] += phaseInc[i];
        if (phase[i] > 1) {
            phase[i] -= 1;
        }
        if (phase[i] > 1) {
             printf("hey, phase too big %f\n", phase[i]); fflush(stdout);
        }
        if (phase[i] < 0) {

             printf("hey, phase too bismallg %f\n", phase[i]); fflush(stdout);
        }
        mix += phase[i];
    }

   // mix = phase[3];     // just for test

    mix *= 2;
    const float output = BiquadFilter<float>::run(mix, filterState, filterParams);
    TBase::outputs[MAIN_OUTPUT].value = output;
}

template <class TBase>
inline void Super<TBase>::updateHPFilters()
{
    filterLookup.get(filterParams, globalPhaseInc);
#if 0
    const float input = TBase::inputs[DEBUG_INPUT].value;
   filterLookup.get(filterParams, globalPhaseInc);
    const float output = BiquadFilter<float>::run(input, filterState, filterParams);
    TBase::outputs[DEBUG_OUTPUT].value = output * 10;
#endif
}

template <class TBase>
inline void Super<TBase>::step()
{
    updatePhaseInc();
    updateHPFilters();
    updateAudio();
}

