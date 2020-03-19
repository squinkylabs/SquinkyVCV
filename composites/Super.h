
#pragma once

#include "SuperDsp.h"

#include "Divider.h"
#include "GateTrigger.h"
#include "IComposite.h"

#include "ObjectCache.h"



namespace rack {
    namespace engine {
        struct Module;
    }
}
using Module = ::rack::engine::Module;

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
    //static const unsigned int MAX_OVERSAMPLE = 16;
 //   static const int numSaws = 7;

 //   float phase[numSaws] = {0};
 //   float phaseInc[numSaws] = {0};
 //   float globalPhaseInc = 0;
    bool isStereo = false;

#if 0
    // current left and right gains
    float sawGainsStereo[2][numSaws] = {
        {.2f},
        {.2f}
    };

    float sawGainsNorm[2][numSaws] = {
        {1.f, .26f, .87f, .71f, .5f, .97f, 0.f},
        {0.f, .97f, .5f,  .71f, .87f,.26f, 1.f}
    };

    float sawGainsHardPan[2][numSaws] = {
        {1.1f, 0.f, 1.1f, 1.f, 0.f, 1.1f, 0.f},
        {0.f, 1.1f, 0.f,  1.f, 1.1f, 0.f, 1.1f}
    };
#endif
    Divider div;

#if 0
    std::function<float(float)> expLookup =
        ObjectCache<float>::getExp2Ex();
    std::shared_ptr<LookupTableParams<float>> audioTaper =
        ObjectCache<float>::getAudioTaper();
        #endif

    // knob, cv, trim -> 0..1
  //  AudioMath::ScaleFun<float> scaleDetune;

    void runSaws(float& left);
    void runSawsStereo(float& left, float& right);
  //  void updatePhaseInc();

 //   void updateAudioClassic();
 //   void updateAudioClean();
  //  void updateAudioClassicStereo();
 //   void updateAudioCleanStereo();
 //   void updateTrigger();
 //   void updateMix();
    void updateStereo();
    void updateStereoGains();
    void stepn(int);

    int getOversampleRate();

    AudioMath::RandomUniformFunc random = AudioMath::random();

   // int inputSubSampleCounter = 1;
    const static int inputSubSample = 4;    // only look at knob/cv every 4


    void updateHPFilters();

 //   SawtoothDetuneCurve detuneCurve;
    GateTrigger gateTrigger;
  //  float gainCenter = 0;
 //   float gainSides = 0;

  //  StateVariable4PHP hpfLeft;
  //  StateVariable4PHP hpfRight;

   // float bufferLeft[MAX_OVERSAMPLE] = {0};
 //   float bufferRight[MAX_OVERSAMPLE] = {0};
 //   IIRDecimator decimatorLeft;
  //  IIRDecimator decimatorRight;

 //   SuperDsp dsp[16];   // maximum 16 channels
    SuperDspCommon dspCommon;
};

template <class TBase>
inline void Super<TBase>::init()
{
    div.setup(inputSubSample, [this] {
        this->stepn(div.getDiv());
     });

    dspCommon.init();
 //   scaleDetune = AudioMath::makeLinearScaler<float>(0, 1);

    const int rate = getOversampleRate();
    const int decimateDiv = std::max(rate, (int) SuperDspCommon::MAX_OVERSAMPLE);

    dspCommon.setupDecimationRatio(decimateDiv);
   // decimatorLeft.setup(decimateDiv);
  //  decimatorRight.setup(decimateDiv);
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
    assert(rate <= (int)SuperDspCommon::MAX_OVERSAMPLE);
    // printf("setting = %d, rate = %d\n", setting, rate);
    return rate;
}




#if 0
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
#endif

#if 0
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
#endif

#if 0 // move to dsp
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
#endif

template <class TBase>
inline void Super<TBase>::updateStereo()
{
    isStereo = TBase::outputs[MAIN_OUTPUT_RIGHT].isConnected() && TBase::outputs[MAIN_OUTPUT_LEFT].isConnected(); 
}

/*

regular:

g[0] = 2.26,0.00 panL=1.00 panR=0.00
g[1] = 0.58,2.18 panL=0.97 panR=0.26
g[2] = 1.96,1.13 panL=0.87 panR=0.50
g[3] = 2.29,2.29 panL=0.71 panR=0.71
g[4] = 1.13,1.96 panL=0.50 panR=0.87
g[5] = 2.18,0.58 panL=0.26 panR=0.97
g[6] = -0.00,2.26 panL=-0.00 panR=1.00

hard:

g[0] = 2.48,0.00 panL=1.00 panR=0.00
g[1] = 0.00,2.40 panL=0.97 panR=0.26
g[2] = 2.15,0.00 panL=0.87 panR=0.50
g[3] = 2.29,2.29 panL=0.71 panR=0.71
g[4] = 0.00,2.15 panL=0.50 panR=0.87
g[5] = 2.40,0.00 panL=0.26 panR=0.97
g[6] = 0.00,2.48 panL=-0.00 panR=1.00


*/

#if 0
template <class TBase>
inline void Super<TBase>::updateStereoGains()
{
    const bool hardPan = TBase::params[HARD_PAN_PARAM].value > .5;
    for (int i=0; i< numSaws; ++i) 
    {
        const float monoGain = 4.5f * ((i == numSaws / 2) ? gainCenter : gainSides);
       
        float l = monoGain;
        float r = monoGain;

        if (!hardPan) {
            l *=  sawGainsNorm[0][i];
            r *=  sawGainsNorm[1][i];
        } else {
            l *=  sawGainsHardPan[0][i];
            r *=  sawGainsHardPan[1][i];
        }

        sawGainsStereo[0][i] = l;
        sawGainsStereo[1][i] = r;

        #if 0
        if (i == 0) printf("\n");
        printf("g[%d] = %.2f,%.2f\n", 
            i, sawGainsStereo[0][i], sawGainsStereo[1][i]);
        #endif
    }
}
#endif

template <class TBase>
inline void Super<TBase>::stepn(int n)
{
    updateStereo();

    float fineTuneParam =  TBase::params[FINE_PARAM].value;
    float semiParam =  TBase::params[SEMI_PARAM].value;
    float octaveParam =  TBase::params[OCTAVE_PARAM].value;

    // TODO: this should be poly
    float fmInput =  TBase::inputs[FM_INPUT].getVoltage(0);
    float fmParam =  TBase::params[FM_PARAM].value;
    float detuneInput = TBase::inputs[FM_INPUT].getVoltage(0);
    float detuneParam = TBase::params[DETUNE_PARAM].value;
    float detuneTrimParam = TBase::params[DETUNE_TRIM_PARAM].value;
    int oversampleRate = getOversampleRate();
    float sampleTime = TBase::engineGetSampleTime();
   
    // TODO: this toally should be poly
    float cv = TBase::inputs[CV_INPUT].getVoltage(0);
    float mixInput = TBase::inputs[MIX_INPUT].getVoltage(0);
    float mixParam = TBase::params[MIX_PARAM].value;
    float mixTrimParam = TBase::params[MIX_TRIM_PARAM].value;
    const bool hardPan = TBase::params[HARD_PAN_PARAM].value > .5;


    for (int i=0; i< dspCommon.numChannels; ++i) {
        dspCommon.stepn(n, i,  oversampleRate, sampleTime, cv, fineTuneParam, semiParam, octaveParam, fmInput,
            fmParam, detuneInput, detuneParam, detuneTrimParam,
            mixInput, mixParam, mixTrimParam,
            isStereo,
            hardPan);
    }


#if 0
    updatePhaseInc();
    updateHPFilters();
    updateMix();
    updateStereo(); 
    updateStereoGains();  
#endif
  //  dspCommon.stepn(n);
}

template <class TBase>
inline void Super<TBase>::step()
{
    div.step();

    int rate = getOversampleRate();
    dspCommon.step(isStereo, TBase::outputs[MAIN_OUTPUT_LEFT], TBase::outputs[MAIN_OUTPUT_RIGHT], rate);
#ifdef _DEBUG
    printf("we need update trigger back in comp\n");
    #endif
    
#if 0
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
    #endif
}

#if 0
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
#endif

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
        default:
            assert(false);
    }
    return ret;
}

