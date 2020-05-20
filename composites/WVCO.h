
#pragma once

/**
 * 
 * 5/17 stock : 123.5
 * -march=native: 100 
 * -finline-limit=n: 103.5
 * -finline-limit=500000 -finline-functions-called-once: 103.6
 *  flatten on step() -finline-limit=500000 -finline-functions-called-once: 102
 *  -flto and all linlines
 * all the above: 77
 * 
 * gcc options to try:
 * 
 * -flto
 * (nc with ?) -fwhole-program
 * -finline-funciton
 * -march=native or skylake

 * static int max(int x, int y) __attribute__((always_inline));
static int max(int x, int y)
{

-finline-functions-called-once

--forceinline

__forceinline static int max(int x, int y)

-finline-limit=n

__attribute__ ((flatten))

conclusions, round 1:
march native is significant, but why? It's not generating avx, is it? (more to do)
inline options make a difference. unclear which to use, although inline-limit is easy
float -> float_4 isn't free.

 */

#include <assert.h>
#include <memory>
#include <vector>
//#include "functions.hpp"
#include <simd/vector.hpp>
#include <simd/functions.hpp>



#ifndef _MSC_VER
#include "ObjectCache.h"
#include "SimdBlocks.h"
#include "IComposite.h"
#include "IIRDecimator.h"
#include "Divider.h"

using float_4 = rack::simd::float_4;


#ifndef _CLAMP
#define _CLAMP
namespace std {
    inline float clamp(float v, float lo, float hi)
    {
        assert(lo < hi);
        return std::min(hi, std::max(v, lo));
    }
}
#endif

namespace rack {
    namespace engine {
        struct Module;
    }
}

using Module = ::rack::engine::Module;

template <class TBase>
class WVCODescription : public IComposite
{
public:
    Config getParam(int i) override;
    int getNumParams() override;
};


class TriFormula
{
public:
    static void getLeftA(float& outA, float k)
    {
        assert(k < 1);
        assert(k > 0);
        outA = 1 / k;
    }
    static void getRightAandB(float& outA, float& outB, float k)
    {
        outA = 1 / (k - 1);
        outB = -outA;
    }
};

class WVCODsp
{
public:
    enum class WaveForm {Sine, Fold, SawTri};
    
    static const int oversampleRate = 4;
    float_4 buffer[oversampleRate];
    WVCODsp() {
        downsampler.setup(oversampleRate);
    }

    float_4 step() {
         __m128 twoPi = {_mm_set_ps1(2 * 3.141592653589793238)};
        float_4 phaseMod = (feedback * rack::simd::sin(phaseAcc * twoPi));
        phaseMod += fmInput;

        for (int i=0; i< oversampleRate; ++i) {
            stepOversampled(i, phaseMod);
        }
        if (oversampleRate == 1) {
            return buffer[0] * outputLevel;
        } else {
            float_4 finalSample = downsampler.process(buffer);
            return finalSample * outputLevel;
        }
    }

    void stepOversampled(int bufferIndex, float_4 phaseModulation)
    {
        float_4 s;
        phaseAcc += normalizedFreq;
        phaseAcc = SimdBlocks::wrapPhase01(phaseAcc);

        __m128 twoPi = {_mm_set_ps1(2 * 3.141592653589793238)};

        float_4 phase = SimdBlocks::wrapPhase01(phaseAcc + phaseModulation);

        if (waveform == WaveForm::Fold) {
            s = rack::simd::sin(phase * twoPi);
            s *= (shapeAdjust * 10);
            s = SimdBlocks::fold(s);
            s *= (5.f * 5.f / 5.6f);        // why do we need this correction?
        } else if (waveform == WaveForm::SawTri) {
            // TODO: move this into helper
            float_4 k = .5 + shapeAdjust / 2;
            float_4 x = phase;
            simd_assertGE(x, float_4(0));
            simd_assertLE(x, float_4(1));
            s = ifelse( x < k, x * aLeft,  aRight * x + bRight);
            s -= .5f;           // center it
            s *= 10;            // andfix range
        } else if (waveform == WaveForm::Sine) {
            s = rack::simd::sin(phase * twoPi);
            s *= 5;
        }
        buffer[bufferIndex] = s;
    }

    // public variables. The composite will set these on us,
    // and we will use them to generate audio.

    // f / fs
    float_4 normalizedFreq = float_4::zero();
    float_4 fmInput = float_4::zero();

    WaveForm waveform;
    float_4 shapeAdjust = 1;    // 0..1
    float_4 aRight = 0;            // y = ax + b for second half of tri
    float_4 bRight = 0;
    float_4 aLeft = 0;
    float_4 feedback = 0;
    float_4 outputLevel = 1;

private:
    float_4 phaseAcc = float_4::zero();
    IIRDecimator<float_4> downsampler;
};

template <class TBase>
class WVCO : public TBase
{
public:

    WVCO(Module * module) : TBase(module)
    {
    }
    WVCO() : TBase()
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
        FREQUENCY_MULTIPLIER_PARAM,
        FINE_TUNE_PARAM,
        FM_DEPTH_PARAM,
        LINEAR_FM_DEPTH_PARAM,
        WAVESHAPE_GAIN_PARAM,
        WAVE_SHAPE_PARAM,
        FEEDBACK_PARAM,
        OUTPUT_LEVEL_PARAM,


        ATTACK_PARAM,
        DECAY_PARAM,
        SUSTAIN_PARAM,
        RELEASE_PARAM,

        ADSR_SHAPE_PARAM,
        ADSR_FBCK_PARAM,
        ADSR_OUTPUT_LEVEL_PARAM,
        ADSR_LFM_DEPTH_PARAM,
        
        NUM_PARAMS
    };

    enum InputIds
    {
        VOCT_INPUT,
        FM_INPUT,
        LINEAR_FM_INPUT,
        GATE_INPUT,
        SYNC_INPUT,
        NUM_INPUTS
    };

    enum OutputIds
    {
        MAIN_OUTPUT,
        NUM_OUTPUTS
    };

    enum LightIds
    {
        NUM_LIGHTS
    };

    /** Implement IComposite
     */
    static std::shared_ptr<IComposite> getDescription()
    {
        return std::make_shared<WVCODescription<TBase>>();
    }

    /**
     * Main processing entry point. Called every sample
     */
    void step() override;

    static std::vector<std::string> getWaveformNames()
    {
        return {"Sine", "Folder", "Saw/T"};
    }

private:
    Divider divn;
    Divider divm;
    WVCODsp dsp[4];
    std::function<float(float)> expLookup = ObjectCache<float>::getExp2Ex();

    // variables to stash processed knobs and other input
    float basePitch = 0;        // all the knobs, no cv. units are volts
    int numChannels = 1;      // 1..16
    int freqMultiplier = 1;
    float baseShapeGain = 0;    // 0..1 -> re-do this!
    float baseFeedback = 0;
    float baseOutputLevel = 1;  // 0..1

    float_4 getOscFreq(int bank);

    void stepn();
    void stepm();
};


template <class TBase>
inline void WVCO<TBase>::init()
{
    divn.setup(4, [this]() {
        stepn();
    });
     divm.setup(16, [this]() {
        stepm();
    });
}

template <class TBase>
inline void WVCO<TBase>::stepm()
{
    numChannels = std::max<int>(1, TBase::inputs[VOCT_INPUT].channels);
    WVCO<TBase>::outputs[ WVCO<TBase>::MAIN_OUTPUT].setChannels(numChannels);

    basePitch = -4.0f + roundf(TBase::params[OCTAVE_PARAM].value) +      
        TBase::params[FINE_TUNE_PARAM].value / 12.0f;

    const float q = float(log2(261.626));       // move up to pitch range of EvenVCO
    basePitch += q;
    if (basePitch > 10) {
        printf("base=%f, oct = %f, fine=%f q = %f\n",
            basePitch,
            TBase::params[OCTAVE_PARAM].value,
            TBase::params[FINE_TUNE_PARAM].value,
            q);
        fflush(stdout);

    }
    assert(basePitch < 10);

    freqMultiplier = int(std::round(TBase::params[FREQUENCY_MULTIPLIER_PARAM].value)); 

    int wfFromUI = (int) std::round(TBase::params[WAVE_SHAPE_PARAM].value);
    WVCODsp::WaveForm wf = WVCODsp::WaveForm(wfFromUI);

    baseShapeGain = TBase::params[WAVESHAPE_GAIN_PARAM].value / 100;
    //printf("baseShapegain set to %f from param %f\n", baseShapeGain,TBase::params[WAVESHAPE_GAIN_PARAM].value);

    int numBanks = numChannels / 4;
    if (numChannels > numBanks * 4) {
        numBanks++;
    }
    for (int bank=0; bank < numBanks; ++bank) {
        dsp[bank].waveform = wf;
    }

    baseFeedback =  TBase::params[FEEDBACK_PARAM].value / 100.f;
    baseOutputLevel =  TBase::params[OUTPUT_LEVEL_PARAM].value / 100.f;
}

template <class TBase>
inline void WVCO<TBase>::stepn()
{
    // update the pitch of every vco
    int numBanks = numChannels / 4;
    if (numChannels > numBanks * 4) {
        numBanks++;
    }
    for (int bank=0; bank < numBanks; ++bank) {

        float_4 freq;
        for (int i=0; i<4; ++i) {
            const int channel = bank * 4 + i;

            float pitch = basePitch;
            pitch += TBase::inputs[VOCT_INPUT].getVoltage(channel);

            if (pitch > 10) {
                printf("looking up pitch %f\n", pitch);
                printf("base pitch = %f, cv = %f\n", basePitch, TBase::inputs[VOCT_INPUT].getVoltage(channel));
                fflush(stdout);
                assert(false);
                // we should actually clip this, not assert
            }

            float _freq = expLookup(pitch);
            _freq *= freqMultiplier;

            const float time = std::clamp(_freq * TBase::engineGetSampleTime(), -.5f, 0.5f);
            freq[i] = time;
        }

        dsp[bank].normalizedFreq = freq / WVCODsp::oversampleRate;
        dsp[bank].shapeAdjust = baseShapeGain;    
       // printf("shapeAdjust set from base out= %s\n", toStr(dsp[bank].shapeAdjust).c_str());

        // now let's compute triangle params
        const float shapeGain =  std::clamp(baseShapeGain, .01, .99); 
        assert(shapeGain < 1);
        assert(shapeGain > 0);

        float k = .5 + shapeGain / 2;
        float a, b;
        TriFormula::getLeftA(a, k);
        dsp[bank].aLeft = a;
        TriFormula::getRightAandB(a, b, k);

        dsp[bank].aRight = a;
        dsp[bank].bRight = b;
        dsp[bank].feedback = baseFeedback * 4;

        dsp[bank].outputLevel = baseOutputLevel;
    }
}

template <class TBase>
inline void WVCO<TBase>::step()
{
    // clock the sub-sample rate tasks
    divn.step();
    divm.step();

    // run the sample loop and write audi
    int numBanks = numChannels / 4;
    if (numChannels > numBanks * 4) {
        numBanks++;
    }
    for (int bank=0; bank < numBanks; ++bank) {
        const int baseChannel = 4 * bank;
       // rack::engine::Port& port = WVCO<TBase>::inputs[LINEAR_FM_INPUT];
        Port& port = WVCO<TBase>::inputs[LINEAR_FM_INPUT];
        float_4 fmInput = port.getPolyVoltageSimd<float_4>(baseChannel);
        auto fmInputScaling = WVCO<TBase>::params[LINEAR_FM_DEPTH_PARAM].value * .01;
        dsp[bank].fmInput = fmInput * fmInputScaling;
        float_4 v = dsp[bank].step(); 
        WVCO<TBase>::outputs[MAIN_OUTPUT].setVoltageSimd(v, baseChannel);
    }
}


template <class TBase>
int WVCODescription<TBase>::getNumParams()
{
    return WVCO<TBase>::NUM_PARAMS;
}

template <class TBase>
inline IComposite::Config WVCODescription<TBase>::getParam(int i)
{
    Config ret(0, 1, 0, "");
    switch (i) {
        case WVCO<TBase>::OCTAVE_PARAM:
            ret = {0.f, 10.0f, 4, "Octave"};
            break;
        case WVCO<TBase>::FREQUENCY_MULTIPLIER_PARAM:
            ret = {1.f, 16.0f, 1, "Freq Ratio"};
            break;
        case WVCO<TBase>::FINE_TUNE_PARAM:
            ret = {-12.0f, 12.0f, 0, "Fine Tune"};
            break;
        case WVCO<TBase>::FM_DEPTH_PARAM:
         ret = {.0f, 100.0f, 0, "[nimp] Freq Mod"};
            break;
        case WVCO<TBase>::LINEAR_FM_DEPTH_PARAM:
            ret = {0, 100, 0, "[nimp] Through-zero FM depth"};
            break;
        case WVCO<TBase>::WAVESHAPE_GAIN_PARAM:
            ret = {0, 100, 20, "Shape Mod"};
            break;
        case WVCO<TBase>::WAVE_SHAPE_PARAM:
            ret = {0, 2, 0, "Wave Shape"};
            break;
        case WVCO<TBase>::FEEDBACK_PARAM:
            ret = {0, 100, 0, "FM feedback depth"};
            break;
        case WVCO<TBase>::ATTACK_PARAM:
            ret = {0, 100, 0, "[nimp] Attck"};
            break;
        case WVCO<TBase>::DECAY_PARAM:
            ret = {-1.0f, 1.0f, 0, "[nimp] Decay"};
            break;
        case WVCO<TBase>::SUSTAIN_PARAM:
            ret = {-1.0f, 1.0f, 0, "[nimp] Sustain"};
            break;
        case WVCO<TBase>::RELEASE_PARAM:
            ret = {-1.0f, 1.0f, 0, "[nimp] Release"};
            break;
        case WVCO<TBase>::OUTPUT_LEVEL_PARAM:
            ret = {0, 100, 50, "[nimp] Level"};
            break;
        case WVCO<TBase>::ADSR_SHAPE_PARAM:
            ret = {0, 100, 0, "[nimp] ADSR->shape"};
            break;
        case WVCO<TBase>::ADSR_FBCK_PARAM:
         ret = {0, 1, 00, "[nimp] ADSR->Feedback"};
            break;
        case WVCO<TBase>::ADSR_OUTPUT_LEVEL_PARAM:
         ret = {0, 1, 0, "[nimp] ADRS->Output Level"};
            break;
        case WVCO<TBase>::ADSR_LFM_DEPTH_PARAM:
         ret = {0, 1, 0, "[nimp] ARSR->FM Depth"};
            break;
        default:
            assert(false);
    }
    return ret;
}
#endif


