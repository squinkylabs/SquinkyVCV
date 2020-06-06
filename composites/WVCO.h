
#pragma once

/**
 * 6/4 use new sine approximation, down to 124%
 * 5/31: feature complete:
 *  8channels 148%
 * 
 * -finline-limit=500000  down to 144.7
 * down to 66 if step is gutted. so let's make an optimized step.
 * (removing adsr inner only took down to 64). SO, asside from inner loop, there must be a lot of bs to get rid of
 * instead of gutting step, just gutted oversampled part. that gives 97. so optimize at step level seems sensible.
 * however, minimal inner loop is still 136. same when I add flatten.
 * 
 * removing the optimzied sine takes back to 67, so the loop is gnarly
 * it's 110 with the filter removed, 82 with sine and filter removed
 * so - inner loops = 70 (40 filter, 30 sine)
 * 
 * 
 * 
 * 
 * 5/25 : 8 channels = 135%
 * 2,005,168 bytes in reduced plugin
 * 
 * down to 96% with re-written asserts
 * first try update envelopes audio rate: 136
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

/**
 * SIMD FM VCO block
 * implements 4 VCOs
 */
class WVCODsp
{
public:
    enum class WaveForm {Sine, Fold, SawTri};
    
    static const int oversampleRate = 4;
    float_4 buffer[oversampleRate];
    float_4 lastOutput = float_4(0);
    WVCODsp() {
        downsampler.setup(oversampleRate);
    }

#ifdef _OPTSIN
    float_4 stepSin() {
       //printf("stepsin opt\n");
#if 1
        int bufferIndex;

        __m128 twoPi = {_mm_set_ps1(2 * 3.141592653589793238)};
        // experiments - try fast approx -> fine
        // try using last output to avoid re-calc
    //    float_4 phaseMod = (feedback * rack::simd::sin(phaseAcc * twoPi));
    //    float_4 phaseMod = (feedback * SimdBlocks::sinTwoPi(phaseAcc * twoPi));
        float_4 phaseMod = (feedback * lastOutput);
        phaseMod += fmInput;

    
        for (bufferIndex = 0; bufferIndex < oversampleRate; ++bufferIndex) {
            phaseAcc += normalizedFreq;
            phaseAcc = SimdBlocks::wrapPhase01(phaseAcc);
            float_4 phase = SimdBlocks::wrapPhase01(phaseAcc + phaseMod);
            //float_4 s = rack::simd::sin(phase * twoPi);
            float_4 s = SimdBlocks::sinTwoPi(phase * twoPi);
            s *= 5;
            buffer[bufferIndex] = s;
        }
        float_4 finalSample = downsampler.process(buffer);
        lastOutput = finalSample;
        return finalSample * outputLevel;
    #else   
        return 0;
    #endif
    }
#endif

    void doSync(float_4 syncValue, int32_4& syncIndex)   {
        if (syncEnabled) {
            float_4 syncCrossing = float_4::zero();
            syncValue -= float_4(0.01f);
            const float_4 syncValueGTZero = syncValue > float_4::zero();
            const float_4 lastSyncValueLEZero = lastSyncValue <= float_4::zero();
            simd_assertMask(syncValueGTZero);
            simd_assertMask(lastSyncValueLEZero);

            const float_4 justCrossed = syncValueGTZero & lastSyncValueLEZero;
            simd_assertMask(justCrossed);
            int m = rack::simd::movemask(justCrossed);

            // If any crossed
            if (m) {
                float_4 deltaSync = syncValue - lastSyncValue;
                syncCrossing = float_4(1.f) - syncValue / deltaSync;
                syncCrossing *= float_4(oversampleRate);
                //syncIndex = syncCrossing;
                float_4 syncIndexF = SimdBlocks::ifelse(justCrossed, syncCrossing, float_4(-1));
                syncIndex = syncIndexF;
            }
            // now make a 
            lastSyncValue = syncValue;
        }
    }
    
    float_4 step(float_4 syncValue) {

#ifdef _OPTSIN
        if (!syncEnabled &&  (waveform == WaveForm::Sine)) {
            return stepSin();
        }
#endif
       // printf("not doing ops. sy=%d wv = %d\n", syncEnabled, waveform);
      //  bool synced = false;
        int32_4 syncIndex = int32_t(-1); // Index in the oversample loop where sync occurs [0, OVERSAMPLE)
      //  float_4 syncCrossing = float_4::zero(); // Offset that sync occurs [0.0f, 1.0f)
        doSync(syncValue, syncIndex);

       //  __m128 twoPi = {_mm_set_ps1(2 * 3.141592653589793238)};
        // float_4 phaseMod = (feedback * rack::simd::sin(phaseAcc * twoPi));
        float_4 phaseMod = (feedback * lastOutput);
        phaseMod += fmInput;

        for (int i=0; i< oversampleRate; ++i) {
           
          //  int32_t syncNowInt = SimdBlocks::ifelse(syncIndex == int32_4(0), int32_4::mask(), int32_4::zero());
          //  simd_assertMask(syncNowInt);
          //  float_4 syncNow = syncNowInt;
           // imd_assertMask(syncNow);
            float_4 syncNow =  float_4(syncIndex) == float_4::zero();
            simd_assertMask(syncNow);

            stepOversampled(i, phaseMod, syncNow);
            syncIndex -= int32_t(1);
        }
        if (oversampleRate == 1) {
            return buffer[0] * outputLevel;
        } else {
            float_4 finalSample = downsampler.process(buffer);
            // printf("dsp step using output level %s\n", toStr(outputLevel).c_str());
            finalSample += waveformOffset;
            lastOutput = finalSample;
            return finalSample * outputLevel;
        }
    }

    void stepOversampled(int bufferIndex, float_4 phaseModulation, float_4 syncNow)
    {
        float_4 s;
        phaseAcc += normalizedFreq;
        phaseAcc = SimdBlocks::wrapPhase01(phaseAcc);
        phaseAcc = SimdBlocks::ifelse(syncNow, float_4::zero(), phaseAcc);

       // __m128 twoPi = {_mm_set_ps1(2 * 3.141592653589793238)};
        float_4 twoPi (2 * 3.141592653589793238);

        float_4 phase = SimdBlocks::wrapPhase01(phaseAcc + phaseModulation);

      //  float_4 s_orig = float_4(0);

       // phase = SimdBlocks::ifelse(syncNow, float_4::zero(), phase);

        if (waveform == WaveForm::Fold) {
         //   s = rack::simd::sin(phase * twoPi);
          
            // TODO: scale, don't clip
          //  float_4 adj = SimdBlocks::ifelse( shapeAdjust < float_4(.095), float_4(.095), shapeAdjust);   // keep adj above .1
          //  float
           // float_4 adj = shapeAdjust;
         //   float_4 adj = sh
         //   s *= (adj * 10);
            s = SimdBlocks::sinTwoPi(phase * twoPi);
            s *= correctedWaveShapeMultiplier;
          //  s_orig = s;
            s = SimdBlocks::fold(s);
         //   s *= (5.f * 5.f / 5.6f);        // why do we need this correction?
        } else if (waveform == WaveForm::SawTri) {
            // TODO: move this into helper
           // float_4 k = .5 + shapeAdjust / 2;
            float_4 k = correctedWaveShapeMultiplier;
            float_4 x = phase;
            simd_assertGE(x, float_4(0));
            simd_assertLE(x, float_4(1));
            s = SimdBlocks::ifelse( x < k, x * aLeft,  aRight * x + bRight);
         //   s -= .5f;           // center it
          //  s *= 10;            // andfix range
        } else if (waveform == WaveForm::Sine) {
            s = SimdBlocks::sinTwoPi(phase * twoPi);
          //  s *= 5;
        } else {
            s = 0;
        }

        // these are firing with folder and an ADSR on shape.
#if 0
        if (s[0] > 20 || s[0] < -20) {
            printf("assert will fire. s_orig was %s\n", toStr(s_orig).c_str());
            printf("shapeAdjust was %s\n", toStr(shapeAdjust).c_str());
        }
        simd_assertLT(s, float_4(20));
        simd_assertGT(s, float_4(-20));
#endif
        buffer[bufferIndex] = s;
    }

    void setSyncEnable(bool f) {
        syncEnabled = f;
        // printf("set sync enabled to %d\n", syncEnabled);
    }

    // public variables. The composite will set these on us,
    // and we will use them to generate audio.

    // f / fs
    float_4 normalizedFreq = float_4::zero();
    float_4 fmInput = float_4::zero();

    WaveForm waveform;
   // float_4 shapeAdjust = 1;    // 0..1
  //  float_4 waveShapePreShift = 0;      // this will be added to waveform before shape lookup
  //  float_4 waveShapePreMultiply = 1;
    float_4 correctedWaveShapeMultiplier = 1;
    
    float_4 aRight = 0;            // y = ax + b for second half of tri
    float_4 bRight = 0;
    float_4 aLeft = 0;
    float_4 feedback = 0;
    float_4 outputLevel = 1;
    float_4 waveformOffset = 0;

private:
    float_4 phaseAcc = float_4::zero();
    float_4 lastSyncValue = float_4::zero();
    IIRDecimator<float_4> downsampler;

    bool syncEnabled = false;
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
        SNAP_PARAM,
        SNAP2_PARAM,
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
     __attribute__((flatten))
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
    float baseOutputLevel = 1;  // 0..x
    float baseOffset = 0;
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
    adsr.setNumChannels(1);         // just to prime the pump, will write true value later
    divn.setup(1, [this]() {
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
    const bool sync = TBase::inputs[SYNC_INPUT].isConnected();

    int numBanks = numChannels / 4;
    if (numChannels > numBanks * 4) {
        numBanks++;
    }
    for (int bank=0; bank < numBanks; ++bank) {
        dsp[bank].waveform = wf;
        dsp[bank].setSyncEnable(sync);
        dsp[bank].waveformOffset = baseOffset;
    }

    // these numbers here are just values found by experimenting - no math.
    baseFeedback =  TBase::params[FEEDBACK_PARAM].value * 2.f / 1000.f;
    baseOutputLevel =  TBase::params[OUTPUT_LEVEL_PARAM].value / 100.f;

    // Sine, Fold, SawTri
    // find the correct offset and gains to apply the waveformat
    // get thet them nomalized
    switch (wf) {
        case WVCODsp::WaveForm::Sine:
            baseOffset = 0;
            baseOutputLevel *= 5;
            break;
        case WVCODsp::WaveForm::Fold:
            baseOffset = 0;
            baseOutputLevel *=  (5.f * 5.f / 5.6f);
            break;
        case WVCODsp::WaveForm::SawTri:
            baseOffset = -.5f; 
            baseOutputLevel *= 10;
            break;
        default:
            assert(0);
    }


#if 1
    const bool snap = TBase::params[SNAP_PARAM].value > .5f;
    const bool snap2 = TBase::params[SNAP2_PARAM].value > .5f;

    float k = 1;
    if (snap || snap2) {
        k = .6;
    }
     if (snap && snap2) {
        k = .3;
    }
 
    adsr.setParams(
        TBase::params[ATTACK_PARAM].value * .01,
        TBase::params[DECAY_PARAM].value * .01,
        TBase::params[SUSTAIN_PARAM].value * .01,
        TBase::params[RELEASE_PARAM].value * .01,
        k
    ) ;
#else

    adsr.setA(TBase::params[ATTACK_PARAM].value * .01);
    adsr.setD(TBase::params[DECAY_PARAM].value * .01);
    adsr.setS(TBase::params[SUSTAIN_PARAM].value * .01);
    adsr.setR(TBase::params[RELEASE_PARAM].value * .01);
    adsr.setSnap( TBase::params[SNAP_PARAM].value > .5f);
#endif
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

   static int x = 0;
    // round up all the gates and run the ADSR;
    {
        float_4 gates[4];
        for (int i=0; i<4; ++i) {
            Port& p = TBase::inputs[GATE_INPUT];
            float_4 g = p.getVoltageSimd<float_4>(i * 4);
            float_4 gate = (g > float_4(1));
            simd_assertMask(gate);
            gates[i] = gate;
#if 0
            if (x == 0 && i==0) {
                printf("\nsending gate to adsr %s\n", toStrLiteral(gate).c_str());
                printf("poly gate cv = %s\n", toStr(
                    g
                    ).c_str());
            }
#endif
        }
        adsr.step(gates, TBase::engineGetSampleTime());
    }

    // TODO: make this faster, and/or do less often
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

        float_4 envMult = (enableAdsrShape) ? adsr.get(bank) : 1;
        simd_assertLE( envMult, float_4(2));
        simd_assertGE(envMult, float_4(0)); 

        dsp[bank].normalizedFreq = freq / WVCODsp::oversampleRate;
       
       
      
      // moved the calculations that were done at oversample rate here,
      // but there is still a lot stuff that doesn't have to be done at sample rate.
       // dsp[bank].shapeAdjust = baseShapeGain * envMult; 
        float_4 correctedWaveShapeMultiplier = baseShapeGain * envMult;
        switch(dsp[bank].waveform) {
            case WVCODsp::WaveForm::Sine:
                break;
            case WVCODsp::WaveForm::Fold:
                correctedWaveShapeMultiplier += float_4(.095);
                correctedWaveShapeMultiplier *= 10;
                break;
            case WVCODsp::WaveForm::SawTri:
                correctedWaveShapeMultiplier = .5 + correctedWaveShapeMultiplier / 2;
                break;
        //    default:
            // done't assert - sometimes we get here before waveform is initialized
               // assert(false);
        }
        dsp[bank].correctedWaveShapeMultiplier = correctedWaveShapeMultiplier;

#if 0
        // TODO: move this into helper
           // float_4 k = .5 + shapeAdjust / 2;
            float_4 k = correctedWaveShapeMultiplier;
            float_4 x = phase;
            simd_assertGE(x, float_4(0));
            simd_assertLE(x, float_4(1));
            s = SimdBlocks::ifelse( x < k, x * aLeft,  aRight * x + bRight);
#endif


        assertLE( baseShapeGain, 1);
        assertGE( baseShapeGain, 0);   

      //  simd_assertLE( dsp[bank].shapeAdjust, float_4(2));
     //   simd_assertGE( dsp[bank].shapeAdjust, float_4(0));   

        // now let's compute triangle params
        const float_4 shapeGain = rack::simd::clamp(baseShapeGain * envMult, .01, .99);
        simd_assertLT(shapeGain, float_4(2));
        simd_assertGT(shapeGain, float_4(0));

        float_4 k = .5 + shapeGain / 2;
        float_4 a, b;
        TriFormula::getLeftA(a, k);
        dsp[bank].aLeft = a;
        TriFormula::getRightAandB(a, b, k);

        dsp[bank].aRight = a;
        dsp[bank].bRight = b;
        
        if (enableAdsrFeedback) {
            dsp[bank].feedback = baseFeedback * 3 * adsr.get(bank); 
        } else {
            dsp[bank].feedback = baseFeedback * 3;
        }

        // TODO: add CV (use getNormalPolyVoltage)
        if (enableAdsrLevel) {
            dsp[bank].outputLevel = adsr.get(bank) * baseOutputLevel;
#if 0
              if (x == 0) {
                printf("update level with env %s\n", toStr(adsr.env[bank]).c_str());
                printf("finaly output %s\n", toStr(dsp[bank].outputLevel).c_str());
            }
#endif
        } else {
            dsp[bank].outputLevel = float_4(baseOutputLevel);    
        }
    }
    x++;
    if (x > 1000) {
        x = 0;
    }
}


template <class TBase>
inline void  __attribute__((flatten)) WVCO<TBase>::step()
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
            fmInputScaling = adsr.get(bank) * (WVCO<TBase>::params[LINEAR_FM_DEPTH_PARAM].value * .003);
        } else {
            fmInputScaling = WVCO<TBase>::params[LINEAR_FM_DEPTH_PARAM].value * .003;
        }
        dsp[bank].fmInput = fmInput * fmInputScaling;

        port = WVCO<TBase>::inputs[SYNC_INPUT];
      //  const bool isConnected = port.isConnected();
        const float_4 syncInput = port.getPolyVoltageSimd<float_4>(baseChannel);
        float_4 v = dsp[bank].step(syncInput); 
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
            ret = {0, 100, 0, "Shape Mod"};
            break;
        case WVCO<TBase>::WAVE_SHAPE_PARAM:
            ret = {0, 2, 0, "Wave Shape"};
            break;
        case WVCO<TBase>::FEEDBACK_PARAM:
            ret = {0, 100, 0, "FM feedback depth"};
            break;
        case WVCO<TBase>::ATTACK_PARAM:
            ret = {0, 100, 50, "Attck"};
            break;
        case WVCO<TBase>::DECAY_PARAM:
            ret = {0, 100, 50, "Decay"};
            break;
        case WVCO<TBase>::SUSTAIN_PARAM:
            ret = {0, 100, 50, "Sustain"};
            break;
        case WVCO<TBase>::RELEASE_PARAM:
            ret = {0, 100, 50, "Release"};
            break;
        case WVCO<TBase>::OUTPUT_LEVEL_PARAM:
            ret = {0, 100, 100, "Level"};
            break;
        case WVCO<TBase>::ADSR_SHAPE_PARAM:
            ret = {0, 1, 0, "ADSR->shape"};
            break;
        case WVCO<TBase>::ADSR_FBCK_PARAM:
         ret = {0, 1, 0, "ADSR->Feedback"};
            break;
        case WVCO<TBase>::ADSR_OUTPUT_LEVEL_PARAM:
         ret = {0, 1, 0, "ADRS->Output Level"};
            break;
        case WVCO<TBase>::ADSR_LFM_DEPTH_PARAM:
         ret = {0, 1, 0, "ARSR->FM Depth"};
            break;
        case WVCO<TBase>::SNAP_PARAM:
            ret = {0, 1, 1, "ARSR Snap"};
            break;
         case WVCO<TBase>::SNAP2_PARAM:
            ret = {0, 1, 1, "ARSR Snap2"};
            break;
        default:
            assert(false);
    }
    return ret;
}
#endif
