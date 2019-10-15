
#pragma once

#include "Divider.h"
#include "GateTrigger.h"
#include "IComposite.h"
#include "IIRDecimator.h"
#include "NonUniformLookupTable.h"
#include "ObjectCache.h"
#include "StateVariable4PHP.h"

namespace rack {
    namespace engine {
        struct Module;
    }
}
using Module = ::rack::engine::Module;

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
class SuperDescription : public IComposite
{
public:
    Config getParam(int i) override;
    int getNumParams() override;
};

/**
 * classic cpu: 14.4
 * clean: 29.6
 * clean2: 85.3
 */
template <class TBase>
class Super : public TBase
{
public:

    Super(Module * module) : TBase(module), gateTrigger(true)
    {
        init();
    }
    Super() : TBase(), gateTrigger(true)
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

    /** Implement IComposite
     */
    static std::shared_ptr<IComposite> getDescription()
    {
        return std::make_shared<SuperDescription<TBase>>();
    }

    enum ParamIds
    {
        OCTAVE_PARAM,
        SEMI_PARAM,
        FINE_PARAM,
        DETUNE_PARAM,
        DETUNE_TRIM_PARAM,
        MIX_PARAM,
        MIX_TRIM_PARAM,
        FM_PARAM,
        CLEAN_PARAM,
        HARD_PAN_PARAM,
        ALTERNATE_PAN_PARAM,
        NUM_PARAMS
    };

    enum InputIds
    {
        CV_INPUT,
        TRIGGER_INPUT,
        DETUNE_INPUT,
        MIX_INPUT,
        FM_INPUT,
        NUM_INPUTS
    };

    enum OutputIds
    {
        MAIN_OUTPUT_LEFT,
        MAIN_OUTPUT_RIGHT,
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
    static const unsigned int MAX_OVERSAMPLE = 16;
    static const int numSaws = 7;

    float phase[numSaws] = {0};
    float phaseInc[numSaws] = {0};
    float globalPhaseInc = 0;
    bool isStereo = false;

    // current left and right gains
    float sawGainsStereo[2][numSaws] = {
        {.2f},
        {.2f}
    };

    Divider div;

    std::function<float(float)> expLookup =
        ObjectCache<float>::getExp2Ex();
    std::shared_ptr<LookupTableParams<float>> audioTaper =
        ObjectCache<float>::getAudioTaper();

    // knob, cv, trim -> 0..1
    AudioMath::ScaleFun<float> scaleDetune;

    void runSaws(float& left);
    void runSawsStereo(float& left, float& right);
    void updatePhaseInc();

    void updateAudioClassic();
    void updateAudioClean();
    void updateAudioClassicStereo();
    void updateAudioCleanStereo();
    void updateTrigger();
    void updateMix();
    void updateStereo();
    void updateStereoGains();
    void stepn(int);

    int getOversampleRate();

    AudioMath::RandomUniformFunc random = AudioMath::random();

   // int inputSubSampleCounter = 1;
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

    StateVariable4PHP hpfLeft;
    StateVariable4PHP hpfRight;

    float bufferLeft[MAX_OVERSAMPLE] = {0};
    float bufferRight[MAX_OVERSAMPLE] = {0};
    IIRDecimator decimatorLeft;
    IIRDecimator decimatorRight;
};

template <class TBase>
inline void Super<TBase>::init()
{
    div.setup(inputSubSample, [this] {
        this->stepn(div.getDiv());
     });

    scaleDetune = AudioMath::makeLinearScaler<float>(0, 1);

    const int rate = getOversampleRate();
    const int decimateDiv = std::max(rate, (int) MAX_OVERSAMPLE);
    decimatorLeft.setup(decimateDiv);
    decimatorRight.setup(decimateDiv);
}

template <class TBase>
inline int Super<TBase>::getOversampleRate()
{
    int rate = 1;
    const int setting = (int) std::round(TBase::params[CLEAN_PARAM].value);
    switch (setting) {
        case 0:
            rate = 1;
            break;
        case 1:
            rate = 4;
            break;
        case 2:
            rate = 16;
            break;
        default:
            assert(false);
    }
    assert(rate <= (int) MAX_OVERSAMPLE);
    return rate;
}

template <class TBase>
inline void Super<TBase>::updatePhaseInc()
{
    const float cv = TBase::inputs[CV_INPUT].getVoltage(0);

    const float finePitch = TBase::params[FINE_PARAM].value / 12.0f;
    const float semiPitch = TBase::params[SEMI_PARAM].value / 12.0f;

    float pitch = 1.0f + roundf(TBase::params[OCTAVE_PARAM].value) +
        semiPitch +
        finePitch;

    pitch += cv;

    const float fm = TBase::inputs[FM_INPUT].getVoltage(0);
    const float fmDepth = AudioMath::quadraticBipolar(TBase::params[FM_PARAM].value);

    pitch += (fmDepth * fm);

    const float q = float(log2(261.626));       // move up to pitch range of EvenVCO
    pitch += q;
    const float freq = expLookup(pitch);
    globalPhaseInc = TBase::engineGetSampleTime() * freq;

    const float rawDetuneValue = scaleDetune(
        TBase::inputs[DETUNE_INPUT].getVoltage(0),
        TBase::params[DETUNE_PARAM].value,
        TBase::params[DETUNE_TRIM_PARAM].value);

    const float detuneInput = detuneCurve.getDetuneFactor(rawDetuneValue);


   // const bool classic = TBase::params[CLEAN_PARAM].value < .5f;
    const int oversampleRate = getOversampleRate();

    for (int i = 0; i < numSaws; ++i) {
        float detune = (detuneFactors[i] - 1) * detuneInput;
        detune += 1;
        float phaseIncI = globalPhaseInc * detune;
        phaseIncI = std::min(phaseIncI, .4f);         // limit so saws don't go crazy
        if (oversampleRate > 1) {
            phaseIncI /= oversampleRate;
        }
        phaseInc[i] = phaseIncI;
    }
}


template <class TBase>
inline void Super<TBase>::runSaws(float& left)
{
    float mix = 0;
    for (int i = 0; i < numSaws; ++i) {
        phase[i] += phaseInc[i];
        if (phase[i] > 1) {
            phase[i] -= 1;
        }
        assert(phase[i] <= 1);
        assert(phase[i] >= 0);

        const float gain = (i == numSaws / 2) ? gainCenter : gainSides;
        mix += (phase[i] - .5f) * gain;        // experiment to get rid of DC

    }

    mix *= 4.5;       // too low 2 too high 10
    left = mix;
}

template <class TBase>
inline void Super<TBase>::runSawsStereo(float& left, float& right)
{
    left = right = 0;
    for (int i = 0; i < numSaws; ++i) {
        phase[i] += phaseInc[i];
        if (phase[i] > 1) {
            phase[i] -= 1;
        }
        assert(phase[i] <= 1);
        assert(phase[i] >= 0);

        left +=  (phase[i] - .5f) *sawGainsStereo[0][i];
        right +=  (phase[i] - .5f) *sawGainsStereo[1][i];
    }
}

template <class TBase>
inline void Super<TBase>::updateAudioClassic()
{
    float left;
    runSaws(left);

    const float outputLeft = hpfLeft.run(left);
    TBase::outputs[MAIN_OUTPUT_LEFT].setVoltage(outputLeft, 0);
    TBase::outputs[MAIN_OUTPUT_RIGHT].setVoltage(outputLeft, 0);
}

template <class TBase>
inline void Super<TBase>::updateAudioClassicStereo()
{
    float left, right;
    runSawsStereo(left, right);

    const float outputLeft = hpfLeft.run(left);
    const float outputRight = hpfRight.run(right);
    TBase::outputs[MAIN_OUTPUT_LEFT].setVoltage(outputLeft, 0);  
    TBase::outputs[MAIN_OUTPUT_RIGHT].setVoltage(outputRight, 0);
}

template <class TBase>
inline void Super<TBase>::updateAudioClean()
{
    const int bufferSize = getOversampleRate();
    decimatorLeft.setup(bufferSize);
    for (int i = 0; i < bufferSize; ++i) {
        float left;
        runSaws(left);
        bufferLeft[i] = left;
    }

    const float output = decimatorLeft.process(bufferLeft);
    TBase::outputs[MAIN_OUTPUT_LEFT].setVoltage(output, 0);
    TBase::outputs[MAIN_OUTPUT_RIGHT].setVoltage(output, 0);
}

template <class TBase>
inline void Super<TBase>::updateAudioCleanStereo()
{
    const int bufferSize = getOversampleRate();
    decimatorLeft.setup(bufferSize);
    decimatorRight.setup(bufferSize);
    for (int i = 0; i < bufferSize; ++i) {
        float left, right;
        runSawsStereo(left, right);
        bufferLeft[i] = left;
        bufferRight[i] = right;
    }

    const float outputLeft = decimatorLeft.process(bufferLeft);
    const float outputRight = decimatorRight.process(bufferRight);
    TBase::outputs[MAIN_OUTPUT_LEFT].setVoltage(outputLeft, 0);
    TBase::outputs[MAIN_OUTPUT_RIGHT].setVoltage(outputRight, 0);
}

template <class TBase>
inline void Super<TBase>::updateHPFilters()
{
    const float filterCutoff = std::min(globalPhaseInc, .1f);
    hpfLeft.setCutoff(filterCutoff);
    if (isStereo) {
        hpfRight.setCutoff(filterCutoff);
    }
}

template <class TBase>
inline void Super<TBase>::updateStereo()
{
    isStereo = TBase::outputs[MAIN_OUTPUT_RIGHT].isConnected() && TBase::outputs[MAIN_OUTPUT_LEFT].isConnected(); 
}

// balance from -1, 1
static inline float panL(float balance)
{ // -1...+1
    float p, inp;
    inp = balance;
    p = M_PI * (inp + 1) / 4;
    return std::cos(p);
}

static inline float panR(float balance)
{
    float p, inp;
    inp = balance;
    p = M_PI * (inp + 1) / 4;
    return std::sin(p);
}

template <class TBase>
inline void Super<TBase>::updateStereoGains()
{
    const bool hardPan = TBase::params[HARD_PAN_PARAM].value > .5;
    const bool alternatePan = TBase::params[ALTERNATE_PAN_PARAM].value > .5;
    for (int i=0; i< numSaws; ++i) 
    {
        float position = -1.f + 2.f * (float) i / (float) (numSaws-1); 

        const float monoGain = 4.5f * ((i == numSaws / 2) ? gainCenter : gainSides);
       

        float l = monoGain * panL(position);
        float r = monoGain * panR(position);

        if (alternatePan) {
            if ((i == 1) || (i == 5)) {
                std::swap(l, r);                
            }
        }

        if (hardPan) {
            if (i != numSaws / 2) {
                if (l > r) {
                    l *= 1.1f;
                    r = 0;
                } else {
                    r *= 1.1f;
                    l = 0;
                }
            }
        }

        sawGainsStereo[0][i] = l;
        sawGainsStereo[1][i] = r;

        #if 0
        if (i == 0) printf("\n");
        printf("g[%d] = %.2f,%.2f\n", i, sawGainsStereo[0][i], sawGainsStereo[1][i]);
        #endif
    }
}

template <class TBase>
inline void Super<TBase>::stepn(int n)
{
    updatePhaseInc();
    updateHPFilters();
    updateMix();
    updateStereo(); 
    updateStereoGains();  
}

template <class TBase>
inline void Super<TBase>::step()
{
    div.step();
    updateTrigger();
    
    int rate = getOversampleRate();
    if ((rate == 1) && !isStereo) {
        updateAudioClassic();
    } else if ((rate == 1) && isStereo) {
        updateAudioClassicStereo();
    } else if ((rate != 1) && !isStereo) {
        updateAudioClean();
    } else {
        updateAudioCleanStereo();
    }
}

template <class TBase>
inline void Super<TBase>::updateTrigger()
{
    gateTrigger.go(TBase::inputs[TRIGGER_INPUT].getVoltage(0));
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
        TBase::inputs[MIX_INPUT].getVoltage(0),
        TBase::params[MIX_PARAM].value,
        TBase::params[MIX_TRIM_PARAM].value);

    gainCenter = -0.55366f * rawMixValue + 0.99785f;

    gainSides = -0.73764f * rawMixValue * rawMixValue +
        1.2841f * rawMixValue + 0.044372f;
}

template <class TBase>
int SuperDescription<TBase>::getNumParams()
{
    return Super<TBase>::NUM_PARAMS;
}

template <class TBase>
inline IComposite::Config SuperDescription<TBase>::getParam(int i)
{
    Config ret(0, 1, 0, "");
    switch (i) {
        case Super<TBase>::OCTAVE_PARAM:
            ret = {-5, 4, 0, "Octave transpose"};
            break;
        case Super<TBase>::SEMI_PARAM:
            ret = {-11, 11, 0, "Semitone transpose"};
            break;
        case Super<TBase>::FINE_PARAM:
            ret = {-1, 1, 0, "Fine tune"};
            break;
        case Super<TBase>::DETUNE_PARAM:
            ret = {-5, 5, 0, "Detune"};
            break;
        case Super<TBase>::DETUNE_TRIM_PARAM:
            ret = {-1, 1, 0, "Detune CV trim"};
            break;
        case Super<TBase>::MIX_PARAM:
            ret = {-5, 5, 0, "Detuned saw level"};
            break;
        case Super<TBase>::MIX_TRIM_PARAM:
            ret = {-1, 1, 0, "Detuned saw CV trim"};
            break;
        case Super<TBase>::FM_PARAM:
            ret = {0, 1, 0, "Pitch modulation depth"};
            break;
        case Super<TBase>::CLEAN_PARAM:
            ret = {0.0f, 2, 0, "Alias suppression amount"};
            break;
        case Super<TBase>::HARD_PAN_PARAM:
            ret =  {0.0f, 1.0f, 0.0f, "Hard Pan"};
            break;
        case Super<TBase>::ALTERNATE_PAN_PARAM:
            ret =  {0.0f, 1.0f, 0.0f, "Alternate Pan"};
            break;
        
        default:
            assert(false);
    }
    return ret;
}

