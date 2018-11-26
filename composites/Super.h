
#pragma once


// Enable the smoothed HPF to reduce pops
#define _SV

#ifdef _SV
#include "StateVariable4PHP.h"
#else
#include "ButterworthLookup.h"
#include "BiquadState.h"
#include "BiquadFilter.h"
#endif

#include "GateTrigger.h"
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


/**
 * orig CPU = 39
 * sub sample => 16
 * beta1 => 16.1
        17.7 if change pitch every 16 samples.


 * glitching: old way, filters popped a ton.
                turning off filters made it go away
 * new lag: .001 : less popping, but still some
 */
template <class TBase>
class Super : public TBase
{
public:

    Super(struct Module * module) : TBase(module),  gateTrigger(true)
    {
        init();
    }
    Super() : TBase(),  gateTrigger(true)
    {
        init();
    }

    /**
    * re-calculate everything that changes with sample
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
        TRIGGER_INPUT,
       // DEBUG_INPUT,
        DETUNE_INPUT,
        MIX_INPUT,
        NUM_INPUTS
    };

    enum OutputIds
    {
        MAIN_OUTPUT,
       // DEBUG_OUTPUT,
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
    float phaseInc[numSaws] = {0};
    float globalPhaseInc = 0;

    std::function<float(float)> expLookup =
        ObjectCache<float>::getExp2Ex();
    std::shared_ptr<LookupTableParams<float>> audioTaper =
        ObjectCache<float>::getAudioTaper();

    // knob, cv, trim -> 0..1
    AudioMath::ScaleFun<float> scaleDetune;

    void updatePhaseInc();
    void updateAudio();
    void updateTrigger();
    void updateMix();

    AudioMath::RandomUniformFunc random =  AudioMath::random();

    int inputSubSampleCounter = 1;
    const static int inputSubSample = 4;    // only look at knob/cv every 4

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

  
    void updateHPFilters();
   
    SawtoothDetuneCurve detuneCurve;
    GateTrigger gateTrigger; 
    float gainCenter = 0;
    float gainSides = 0;

#ifdef _SV
    StateVariable4PHP hpf;
#else
    ButterworthLookup4PHP filterLookup;
    BiquadState<float, 2> filterState;
    BiquadParams<float, 2> filterParams;
#endif
};

template <class TBase>
inline void Super<TBase>::init()
{
    scaleDetune = AudioMath::makeLinearScaler<float>(0, 1);
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

    const float q = float(log2(261.626));       // move up to pitch range of EvenVCO
    pitch += q;
    const float freq = expLookup(pitch);
    globalPhaseInc = TBase::engineGetSampleTime() * freq;

    const float rawDetuneValue = scaleDetune(
        TBase::inputs[DETUNE_INPUT].value,
        TBase::params[DETUNE_PARAM].value,
        TBase::params[DETUNE_TRIM_PARAM].value);

    const float detuneInput = detuneCurve.getDetuneFactor(rawDetuneValue);

    for (int i = 0; i < numSaws; ++i) {
        float detune = (detuneFactors[i] - 1) * detuneInput;
        detune += 1;
        phaseInc[i] = globalPhaseInc * detune;
        //phaseInc[i] = globalPhaseInc;       // JUST FOR TEST. DON"T CHCKE IN
    }
}

template <class TBase>
inline void Super<TBase>::updateAudio()
{
    float mix = 0;
    for (int i = 0; i < numSaws; ++i) {
        phase[i] += phaseInc[i];
        if (phase[i] > 1) {
            phase[i] -= 1;
        }
        if (phase[i] > 1) {
            printf("hey, phase too big %f\n", phase[i]); fflush(stdout);
        }
        if (phase[i] < 0) {
            printf("hey, phase too small %f\n", phase[i]); fflush(stdout);
        }
       // gainSides = 0;  // JUST TO FIND GLITCH. DON"T CHECK IN.
        const float gain = (i == numSaws/2) ? gainCenter : gainSides;
      //  mix += phase[i] * gain;
        mix += (phase[i] - .5f) * gain;        // experiment
    }

    mix *= 2;

#ifdef _SV
    const float output = hpf.run(mix);
#else
   const float output = BiquadFilter<float>::run(mix, filterState, filterParams);
   //const float output = mix;     // try without filters. don't check in.
#endif

#ifdef _LOG
   static float lastOutput = -100;
   static float maxDelta = 0;

   if (lastOutput < -90) {
       lastOutput = output;
   }
   const float delta = std::abs(output - lastOutput);
   if ((delta > maxDelta) || 0) {
       maxDelta = delta;
       printf("** MAX POP %.2f\n", maxDelta);
   }

  printf("mix = %.2f output=%.2f delta = %.2f\n", mix, output, delta);

  // if (delta > 4) printf("** POP %.2f **\n", delta);
   lastOutput = output;
   fflush(stdout);
   if (maxDelta > 4) abort();
#endif


    TBase::outputs[MAIN_OUTPUT].value = output;
 

}

template <class TBase>
inline void Super<TBase>::updateHPFilters()
{
#ifdef _SV
    hpf.setCutoff(globalPhaseInc);
#else
    filterLookup.get(filterParams, globalPhaseInc);
#endif
}

template <class TBase>
inline void Super<TBase>::step()
{
    updateTrigger();
    if (--inputSubSampleCounter <= 0) {
        inputSubSampleCounter = inputSubSample;
        updatePhaseInc();
        updateHPFilters();
        updateMix();
    }
    updateAudio();
}

template <class TBase>
inline void Super<TBase>::updateTrigger()
{
    gateTrigger.go(TBase::inputs[TRIGGER_INPUT].value);
    if (gateTrigger.trigger()) {
        for (int i = 0; i < numSaws; ++i) {
            phase[i] = this->random();
        }
    }
}

template <class TBase>
inline void Super<TBase>::updateMix()
{
    const float rawMixValue = scaleDetune(
        TBase::inputs[MIX_INPUT].value,
        TBase::params[MIX_PARAM].value,
        TBase::params[MIX_TRIM_PARAM].value);

    gainCenter = -0.55366f * rawMixValue + 0.99785f;

    gainSides = -0.73764f * rawMixValue * rawMixValue +
        1.2841f * rawMixValue + 0.044372f;
}