
#pragma once

/**
 * 
 * 5/25 : 8 channels = 135%
 * 2,005,168 bytes in reduced plugin
 * 
 * down to 96% with re-written asserts
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

#ifndef _MSC_VER
#include "simd.h"
#include "ADSR16.h"
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
    static void getLeftA(float_4& outA, float_4 k)
    {
        simd_assertLT(k, float_4(1));
        simd_assertGT(k, float_4(0));
        outA = 1 / k;
    }
    static void getRightAandB(float_4& outA, float_4& outB, float_4 k)
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

        float_4 s_orig = float_4(0);

        if (waveform == WaveForm::Fold) {
            s = rack::simd::sin(phase * twoPi);
          
            // TODO: scale, don't clip
            float_4 adj = SimdBlocks::ifelse( shapeAdjust < float_4(.095), float_4(.095), shapeAdjust);   // keep adj above .1
           // float_4 adj = shapeAdjust;
            s *= (adj * 10);
            s_orig = s;
            s = SimdBlocks::fold(s);
            s *= (5.f * 5.f / 5.6f);        // why do we need this correction?
        } else if (waveform == WaveForm::SawTri) {
            // TODO: move this into helper
            float_4 k = .5 + shapeAdjust / 2;
            float_4 x = phase;
            simd_assertGE(x, float_4(0));
            simd_assertLE(x, float_4(1));
            s = SimdBlocks::ifelse( x < k, x * aLeft,  aRight * x + bRight);
            s -= .5f;           // center it
            s *= 10;            // andfix range
        } else if (waveform == WaveForm::Sine) {
            s = rack::simd::sin(phase * twoPi);
            s *= 5;
        }

        // these are firing with folder and an ADSR on shape.
#ifndef NDEBUG
        if (s[0] > 20 || s[0] < -20) {
            printf("assert will fire. s_orig was %s\n", toStr(s_orig).c_str());
            printf("shapeAdjust was %s\n", toStr(shapeAdjust).c_str());
        }
        simd_assertLT(s, float_4(20));
        simd_assertGT(s, float_4(-20));
#endif
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
    ADSR16 adsr;

    std::function<float(float)> expLookup = ObjectCache<float>::getExp2Ex();

    // variables to stash processed knobs and other input
    float basePitch = 0;        // all the knobs, no cv. units are volts
    int numChannels = 1;      // 1..16
    int freqMultiplier = 1;
    float baseShapeGain = 0;    // 0..1 -> re-do this!
    float baseFeedback = 0;
    float baseOutputLevel = 1;  // 0..1
    bool enableAdsrLevel = false;
    bool enableAdsrFeedback = false;
    bool enableAdsrFM = false;
    bool enableAdsrShape = false;

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

#if 0
    {
        static int last = -1;
        if ( std::abs(basePitch - last) > .2) {
        //if (basePitch != last) {
            printf("base != last. diff = %f\n", basePitch - last);
            printf("base=%f, oct = %f, fine=%f q = %f\n",
                basePitch,
                TBase::params[OCTAVE_PARAM].value,
                TBase::params[FINE_TUNE_PARAM].value,
                q);
            last = basePitch;
            fflush(stdout);
        }
    }
    #endif

    //assert(basePitch < 10);

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
    baseOutputLevel =  TBase::params[OUTPUT_LEVEL_PARAM].value / 100.f;

    adsr.setA(TBase::params[ATTACK_PARAM].value);
    adsr.setD(TBase::params[DECAY_PARAM].value);
    adsr.setS(TBase::params[SUSTAIN_PARAM].value);
    adsr.setR(TBase::params[RELEASE_PARAM].value);
    adsr.setNumChannels(numChannels);

    enableAdsrLevel = TBase::params[ADSR_OUTPUT_LEVEL_PARAM].value > .5;
    enableAdsrFeedback = TBase::params[ADSR_FBCK_PARAM].value > .5;
    enableAdsrFM = TBase::params[ADSR_LFM_DEPTH_PARAM].value > .5;
    enableAdsrShape = TBase::params[ADSR_SHAPE_PARAM].value > .5;
}

template <class TBase>
inline void WVCO<TBase>::stepn()
{
    int numBanks = numChannels / 4;
    if (numChannels > numBanks * 4) {
        numBanks++;
    }

    // round up all the gates and run the ADSR;
    {
        float_4 gates[4];
        for (int i=0; i<4; ++i) {
            float_4 gate = (TBase::inputs[GATE_INPUT].getPolyVoltage(i*4) > float_4(1));
            simd_assertMask(gate);
            gates[i] = gate;
        }
       adsr.step(gates, TBase::engineGetSampleTime());
    }

     // update the pitch of every vco
    for (int bank=0; bank < numBanks; ++bank) {
        float_4 freq;
        for (int i=0; i<4; ++i) {
            const int channel = bank * 4 + i;

            float pitch = basePitch;
            // use SIMD here?
            pitch += TBase::inputs[VOCT_INPUT].getVoltage(channel);

            float _freq = expLookup(pitch);
            _freq *= freqMultiplier;

            const float time = std::clamp(_freq * TBase::engineGetSampleTime(), -.5f, 0.5f);
            freq[i] = time;
        }

        float_4 envMult = (enableAdsrShape) ? adsr.env[bank] : 1;
        simd_assertLE( envMult, float_4(1));
        simd_assertGE(envMult, float_4(0)); 

        dsp[bank].normalizedFreq = freq / WVCODsp::oversampleRate;
        dsp[bank].shapeAdjust = baseShapeGain * envMult; 

        assertLE( baseShapeGain, 1);
        assertGE( baseShapeGain, 0);   

        simd_assertLE( dsp[bank].shapeAdjust, float_4(1));
        simd_assertGE( dsp[bank].shapeAdjust, float_4(0));   

        // now let's compute triangle params
        const float_4 shapeGain = rack::simd::clamp(baseShapeGain * envMult, .01, .99);
        simd_assertLT(shapeGain, float_4(1));
        simd_assertGT(shapeGain, float_4(0));

        float_4 k = .5 + shapeGain / 2;
        float_4 a, b;
        TriFormula::getLeftA(a, k);
        dsp[bank].aLeft = a;
        TriFormula::getRightAandB(a, b, k);

        dsp[bank].aRight = a;
        dsp[bank].bRight = b;
        
        if (enableAdsrFeedback) {
            dsp[bank].feedback = baseFeedback * 3 * adsr.env[bank]; 
        } else {
            dsp[bank].feedback = baseFeedback * 3;
        }

        // TODO: add CV (use getNormalPolyVoltage)
        if (enableAdsrLevel) {
            dsp[bank].outputLevel = adsr.env[bank] * baseOutputLevel;
        } else {
            dsp[bank].outputLevel = baseOutputLevel;    
        }
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
        Port& port = WVCO<TBase>::inputs[LINEAR_FM_INPUT];
        float_4 fmInput = port.getPolyVoltageSimd<float_4>(baseChannel);

        float_4 fmInputScaling;
        if (enableAdsrFM) {
            fmInputScaling = adsr.env[bank] * (WVCO<TBase>::params[LINEAR_FM_DEPTH_PARAM].value * .003);
        } else {
            fmInputScaling = WVCO<TBase>::params[LINEAR_FM_DEPTH_PARAM].value * .003;
        }
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
            ret = {0, 100, 0, "Through-zero FM depth"};
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
            ret = {0, 100, 0, "Attck"};
            break;
        case WVCO<TBase>::DECAY_PARAM:
            ret = {-1.0f, 1.0f, 0, "Decay"};
            break;
        case WVCO<TBase>::SUSTAIN_PARAM:
            ret = {-1.0f, 1.0f, 0, "Sustain"};
            break;
        case WVCO<TBase>::RELEASE_PARAM:
            ret = {-1.0f, 1.0f, 0, "Release"};
            break;
        case WVCO<TBase>::OUTPUT_LEVEL_PARAM:
            ret = {0, 100, 50, "Level"};
            break;
        case WVCO<TBase>::ADSR_SHAPE_PARAM:
            ret = {0, 1, 0, "ADSR->shape"};
            break;
        case WVCO<TBase>::ADSR_FBCK_PARAM:
         ret = {0, 1, 00, "ADSR->Feedback"};
            break;
        case WVCO<TBase>::ADSR_OUTPUT_LEVEL_PARAM:
         ret = {0, 1, 0, "ADRS->Output Level"};
            break;
        case WVCO<TBase>::ADSR_LFM_DEPTH_PARAM:
         ret = {0, 1, 0, "ARSR->FM Depth"};
            break;
        default:
            assert(false);
    }
    return ret;
}
#endif
