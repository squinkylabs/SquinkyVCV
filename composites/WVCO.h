
#pragma once

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
  //  static const __m128 twoPi = {_mm_set_ps1(2 * 3.141592653589793238)};

 //   static const double twop = 2.0 * 3.141592653589793238;
  //  static float_4 twoPi(twop);
    float_4 buffer[oversampleRate];
    WVCODsp() {
        downsampler.setup(oversampleRate);
    }

    float_4 step() {
         __m128 twoPi = {_mm_set_ps1(2 * 3.141592653589793238)};

        float_4 phase = phaseAcc + (feedback * rack::simd::sin(phaseAcc * twoPi));
        phase = SimdBlocks::wrapPhase01(phase);
        phaseAcc = phase;

        for (int i=0; i< oversampleRate; ++i) {
            stepOversampled(i);
        }
        if (oversampleRate == 1) {
            return buffer[0];
        } else {
            float_4 finalSample = downsampler.process(buffer);
            return finalSample;
        }
    }

    void stepOversampled(int bufferIndex)
    {
        float_4 s;
        phaseAcc += normalizedFreq;
        // Wrap phase
        //phaseAcc -= rack::simd::floor(phaseAcc);
        phaseAcc = SimdBlocks::wrapPhase01(phaseAcc);

        __m128 twoPi = {_mm_set_ps1(2 * 3.141592653589793238)};

        // this should go outside the loop.
     //   float_4 phase = phaseAcc + (feedback * rack::simd::sin(phaseAcc * twoPi));

        if (waveform == WaveForm::Fold) {
            s = rack::simd::sin(phaseAcc * twoPi);
            s *= (shapeAdjust * 10);
            s = SimdBlocks::fold(s);
            s *= 5;
        } else if (waveform == WaveForm::SawTri) {
            // TODO: move this into helper
            float_4 k = .5 + shapeAdjust / 2;
            float_4 x = phaseAcc;
            simd_assertGE(x, float_4(0));
            simd_assertLE(x, float_4(1));
            s = ifelse( x < k, x * aLeft,  aRight * x + bRight);
            // printf("k = %s\n  x = %s\n s = %s\n",  toStr(k).c_str(), toStr(x).c_str(), toStr(s).c_str());
        } else if (waveform == WaveForm::Sine) {
            s = rack::simd::sin(phaseAcc * twoPi);
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

        ATTACK_PARAM,
        DECAY_PARAM,
        SUSTAIN_PARAM,
        RELEASE_PARAM,
        
        NUM_PARAMS
    };

    enum InputIds
    {
        VOCT_INPUT,
        FM_INPUT,
        LINEAR_FM_INPUT,
        GATE_INPUT,
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
    float basePitch;        // all the knobs, no cv. units are volts
    int numChannels = 1;      // 1..16
    int freqMultiplier = 1;
    float baseShapeGain = 0;    // 0..1 -> re-do this!
    float baseFeedback = 0;

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

    freqMultiplier = int(std::round(TBase::params[FREQUENCY_MULTIPLIER_PARAM].value)); 

    int wfFromUI = (int) std::round(TBase::params[WAVE_SHAPE_PARAM].value);
    WVCODsp::WaveForm wf = WVCODsp::WaveForm(wfFromUI);

    baseShapeGain = TBase::params[WAVESHAPE_GAIN_PARAM].value / 100;

    int numBanks = numChannels / 4;
    if (numChannels > numBanks * 4) {
        numBanks++;
    }
    for (int bank=0; bank < numBanks; ++bank) {
        dsp[bank].waveform = wf;
    }

    baseFeedback =  TBase::params[FEEDBACK_PARAM].value / 100.f;
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

            float _freq = expLookup(pitch);
            _freq *= freqMultiplier;

            const float time = std::clamp(_freq * TBase::engineGetSampleTime(), -.5f, 0.5f);
            freq[i] = time;
        }

        dsp[bank].normalizedFreq = freq / WVCODsp::oversampleRate;
        dsp[bank].shapeAdjust = baseShapeGain;

        // printf("base shape gain = %f\n", baseShapeGain); fflush(stdout);
        baseShapeGain = std::clamp(baseShapeGain, .01, .99);
        assert(baseShapeGain < 1);
        assert(baseShapeGain > 0);

        float k = .5 + baseShapeGain / 2;
        float a, b;
        TriFormula::getLeftA(a, k);
        dsp[bank].aLeft = a;
        TriFormula::getRightAandB(a, b, k);

        dsp[bank].aRight = a;
        dsp[bank].bRight = b;
        dsp[bank].feedback = baseFeedback * 4;
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
        dsp[bank].fmInput = fmInput;
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
            ret = {0, 100, 0, "Shape Mod"};
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
        default:
            assert(false);
    }
    return ret;
}
#endif


