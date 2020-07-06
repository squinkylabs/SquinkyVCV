
#pragma once

/**
 * 7/2 same number. if m=160 (instead of 16) : 80.8
 *  if n=40 (instead of 4) : 67
 * conclusion: no point in optimizing m path. could get a little from n path,
 * but not much.
 * 
 * optimize for no sync or fm: 84.7
 * add missing CV, clean up: 86.1
 * do shape calcs at reduced rate 87.4
 * 6/13 start to refactor 86.8% (gates at full rate). 
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
#include "ADSR16.h"
#include "WVCODsp.h"
#include "LookupTable.h"
#include "ObjectCache.h"

#include "IComposite.h"

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
        SHAPE_INPUT,
        LINEAR_FM_DEPTH_INPUT,
        FEEDBACK_INPUT,
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
        return {"Sine", "Wave folder", "Saw<>Triangle"};
    }

private:
    Divider divn;
    Divider divm;
    WVCODsp dsp[4];
    ADSR16 adsr;

    std::function<float(float)> expLookup = ObjectCache<float>::getExp2Ex();
    std::shared_ptr<LookupTableParams<float>>  audioTaperLookupParams = ObjectCache<float>::getAudioTaper();

    // variables to stash processed knobs and other input
    // variables with _m suffix updated every m period
    float_4 basePitch_m = 0;        // all the knobs, no cv. units are volts
    int numChannels_m = 1;      // 1..16
    int numBanks_m = 1;     // 1..4
    float_4 depth_m;                // exp mode depth from knob
    float_4 baseFmDepth_m = 0;
    bool fmDepthConnected_m = false;
    bool feedbackConnected_m = false;
    bool fmInputConnected_m = false;
    bool syncInputConnected_m = false;

    float_4 freqMultiplier_m = 1;
    float baseShapeGain = 0;    // 0..1 -> re-do this!
    float_4 baseFeedback_m = 0;
    float_4 baseOutputLevel_m = 1;  // 0..x
    float_4 baseOffset_m = 0;
    bool enableAdsrLevel = false;
    bool enableAdsrFeedback = false;
    bool enableAdsrFM = false;
    bool enableAdsrShape = false;

    float_4 getOscFreq(int bank);

    /**
     * This was originally a regular /4 stepn, but
     * I started doing it at X 1.
     * Now need to refactor
     */
    void stepn_fullRate();

    void stepn_lowerRate();

    /**
     * This is called "very infrequently"
     * Currently every 16 samples, but could go lower. for knobs and such
     */
    void stepm();

    /**
     * combine CV with knobs (basePitch_m),
     * send to DSPs
     */
    void updateFreq_n();
    void updateShapes_n();
};


template <class TBase>
inline void WVCO<TBase>::init()
{
    adsr.setNumChannels(1);         // just to prime the pump, will write true value later
    divn.setup(4, [this]() {
        stepn_lowerRate();
    });
     divm.setup(16, [this]() {
        stepm();
    });
}

template <class TBase>
inline void WVCO<TBase>::stepm()
{
    numChannels_m = std::max<int>(1, TBase::inputs[VOCT_INPUT].channels);
    WVCO<TBase>::outputs[ WVCO<TBase>::MAIN_OUTPUT].setChannels(numChannels_m);

    numBanks_m = numChannels_m / 4;
    if (numChannels_m > numBanks_m * 4) {
        numBanks_m++;
    }

    float basePitch = -4.0f + roundf(TBase::params[OCTAVE_PARAM].value) +      
        TBase::params[FINE_TUNE_PARAM].value / 12.0f;

    const float q = float(log2(261.626));       // move up to pitch range of EvenVCO
    basePitch += q;
    basePitch_m = float_4(basePitch);

    depth_m = .3 * LookupTable<float>::lookup(
                *audioTaperLookupParams, 
                TBase::params[FM_DEPTH_PARAM].value * .01f);


    freqMultiplier_m =  float_4(std::round(TBase::params[FREQUENCY_MULTIPLIER_PARAM].value)); 

    baseFmDepth_m = float_4(WVCO<TBase>::params[LINEAR_FM_DEPTH_PARAM].value * .003);
    {
        Port& depthCVPort = WVCO<TBase>::inputs[LINEAR_FM_DEPTH_INPUT];
        fmDepthConnected_m = depthCVPort.isConnected();
    }
    {
        Port& feedbackPort = WVCO<TBase>::inputs[FEEDBACK_INPUT];
        feedbackConnected_m = feedbackPort.isConnected();
    }
    {
        Port& fmInputPort = WVCO<TBase>::inputs[LINEAR_FM_INPUT];
        fmInputConnected_m = fmInputPort.isConnected();
    }
    {
        Port& syncInputPort = WVCO<TBase>::inputs[SYNC_INPUT];
        syncInputConnected_m = syncInputPort.isConnected(); 
    }

  

    int wfFromUI = (int) std::round(TBase::params[WAVE_SHAPE_PARAM].value);
    WVCODsp::WaveForm wf = WVCODsp::WaveForm(wfFromUI);

    baseShapeGain = TBase::params[WAVESHAPE_GAIN_PARAM].value / 100;
    const bool sync = TBase::inputs[SYNC_INPUT].isConnected();


    for (int bank=0; bank < numBanks_m; ++bank) {
        dsp[bank].waveform = wf;
        dsp[bank].setSyncEnable(sync);
        dsp[bank].waveformOffset = baseOffset_m;
    }

    // these numbers here are just values found by experimenting - no math.
    baseFeedback_m =  float_4(3.f * TBase::params[FEEDBACK_PARAM].value * 2.f / 1000.f);
    baseOutputLevel_m =  float_4(TBase::params[OUTPUT_LEVEL_PARAM].value / 100.f);

    // Sine, Fold, SawTri
    // find the correct offset and gains to apply the waveformat
    // get thet them nomalized
    switch (wf) {
        case WVCODsp::WaveForm::Sine:
            baseOffset_m = 0;
            baseOutputLevel_m *= 5;
            break;
        case WVCODsp::WaveForm::Fold:
            baseOffset_m = 0;
            baseOutputLevel_m *=  (5.f * 5.f / 5.6f);
            break;
        case WVCODsp::WaveForm::SawTri:
            baseOffset_m = -.5f; 
            baseOutputLevel_m *= 10;
            break;
        default:
            assert(0);
    }

    const bool snap = TBase::params[SNAP_PARAM].value > .5f;
    const bool snap2 = TBase::params[SNAP2_PARAM].value > .5f;
    const bool snap3 = TBase::params[SNAP_PARAM].value > 1.5f;

    float k = 1;
    if (snap || snap2) {
        k = .6;
    }
     if (snap3 || (snap && snap2)) {
        k = .3;
    }
 
    adsr.setParams(
        TBase::params[ATTACK_PARAM].value * .01,
        TBase::params[DECAY_PARAM].value * .01,
        TBase::params[SUSTAIN_PARAM].value * .01,
        TBase::params[RELEASE_PARAM].value * .01,
        k
    ) ;

    adsr.setNumChannels(numChannels_m);

    enableAdsrLevel = TBase::params[ADSR_OUTPUT_LEVEL_PARAM].value > .5;
    enableAdsrFeedback = TBase::params[ADSR_FBCK_PARAM].value > .5;
    enableAdsrFM = TBase::params[ADSR_LFM_DEPTH_PARAM].value > .5;
    enableAdsrShape = TBase::params[ADSR_SHAPE_PARAM].value > .5;
}

template <class TBase>
inline void WVCO<TBase>::updateFreq_n()
{
    for (int bank=0; bank < numBanks_m; ++bank) {
        float_4 freq=0;

        const int baseChannel = bank;

        float_4 pitch = basePitch_m;
        // use SIMD here?
        Port& v8Port = TBase::inputs[VOCT_INPUT]; 
        pitch += v8Port.getPolyVoltageSimd<float_4>(baseChannel);

        Port& fmInputPort = TBase::inputs[FM_INPUT];
        pitch += fmInputPort.getPolyVoltage(baseChannel) * depth_m; 
        for (int i=0; i<4; ++i) { 
             freq[i] = expLookup(pitch[i]);      
        }       
     //   float _freq = expLookup(pitch);
        freq *= freqMultiplier_m;
        float_4 time = rack::simd::clamp(freq * TBase::engineGetSampleTime(), -.5f, 0.5f);
        freq = time;
    
        dsp[bank].normalizedFreq = freq / WVCODsp::oversampleRate;
    }
}

template <class TBase>
inline void WVCO<TBase>::updateShapes_n()
{
    for (int bank=0; bank < numBanks_m; ++bank) {
        const int baseChannel = bank * 4;


        float_4 envMult = (enableAdsrShape) ? adsr.get(bank) : 1;
        simd_assertLE( envMult, float_4(2));
        simd_assertGE(envMult, float_4(0)); 
       
        float_4 baseGain = baseShapeGain;
        Port& port = TBase::inputs[SHAPE_INPUT];
        if (port.isConnected()) {
            baseGain *= port.getPolyVoltageSimd<float_4>(baseChannel);
            baseGain *= float_4(.1f);
            baseGain = rack::simd::clamp(baseGain, 0, 1);
        }
        float_4 correctedWaveShapeMultiplier = baseGain * envMult;
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
        }
        dsp[bank].correctedWaveShapeMultiplier = correctedWaveShapeMultiplier;

        assertLE( baseShapeGain, 1);
        assertGE( baseShapeGain, 0);   


        // we could spit this triangle calc it we aren't triangle,
        // but it doesn't take very long.

        // could to this at 'n' rate, and only it triangle
        // now let's compute triangle params
        if (dsp[bank].waveform == WVCODsp::WaveForm::SawTri) {
            const float_4 shapeGain = rack::simd::clamp(baseGain * envMult, .01, .99);
            simd_assertLT(shapeGain, float_4(2));
            simd_assertGT(shapeGain, float_4(0));

            float_4 k = .5 + shapeGain / 2;
            float_4 a, b;
            TriFormula::getLeftA(a, k);
            dsp[bank].aLeft = a;
            TriFormula::getRightAandB(a, b, k);

            dsp[bank].aRight = a;
            dsp[bank].bRight = b;
        }
    }
}

template <class TBase>
inline void WVCO<TBase>::stepn_lowerRate()
{
    updateFreq_n();    
    updateShapes_n();
}

template <class TBase>
inline void  __attribute__((flatten)) WVCO<TBase>::stepn_fullRate()
{
    assert(numBanks_m > 0);

    //--------------------------------------------------------------------
    // round up all the gates and run the ADSR;
    {
        // can do gates is lower rate (but it's no better)
        float_4 gates[4];
        for (int i=0; i<4; ++i) {
            Port& p = TBase::inputs[GATE_INPUT];
            float_4 g = p.getVoltageSimd<float_4>(i * 4);
            float_4 gate = (g > float_4(1));
            simd_assertMask(gate);
            gates[i] = gate;
        }

        adsr.step(gates, TBase::engineGetSampleTime());
    }
    
    // ----------------------------------------------------------------------------
    // now update all the DSP params.
    // This is the new, cleaner version.
    for (int bank=0; bank < numBanks_m; ++bank) {
        
        float_4 feedbackAmount = baseFeedback_m;;  
        if (enableAdsrFeedback) {
            feedbackAmount *= adsr.get(bank); 
        }
        if (feedbackConnected_m) {
            Port& feedbackPort = WVCO<TBase>::inputs[FEEDBACK_INPUT];
            feedbackAmount *= feedbackPort.getPolyVoltageSimd<float_4>(bank * 4) * float_4(.1);
            feedbackAmount = rack::simd::clamp(feedbackAmount, float_4(0), float_4(1));
        }
        dsp[bank].feedback = feedbackAmount;

        // TODO: add CV (use getNormalPolyVoltage)
        dsp[bank].outputLevel = baseOutputLevel_m;    
        if (enableAdsrLevel) {
            dsp[bank].outputLevel *= adsr.get(bank);
        } 
    }
}

template <class TBase>
inline void  __attribute__((flatten)) WVCO<TBase>::step()
{
    // clock the sub-sample rate tasks
    divn.step();
    divm.step();

    stepn_fullRate();
    assert(numBanks_m > 0);

    // this could even be moves out of the "every sample" loop
    if (!syncInputConnected_m && !fmInputConnected_m) {
        for (int bank=0; bank < numBanks_m; ++bank) {
            const int baseChannel = 4 * bank;
            dsp[bank].fmInput = 0;
            float_4 v =dsp[bank].step( float_4::zero());
            WVCO<TBase>::outputs[MAIN_OUTPUT].setVoltageSimd(v, baseChannel);
        }
    } else {

        // TODO: don't do this if fm input port not connected. sync also
        for (int bank=0; bank < numBanks_m; ++bank) {
            const int baseChannel = 4 * bank;
            Port& fmInputPort = WVCO<TBase>::inputs[LINEAR_FM_INPUT];
            float_4 fmInput = fmInputPort.getPolyVoltageSimd<float_4>(baseChannel);

            // TODO:much of this could be done in stepn
            // TODO: add depth CV
            float_4 fmInputScaling = baseFmDepth_m;
            if (enableAdsrFM) {
                fmInputScaling *= adsr.get(bank);
            }
            if (fmDepthConnected_m) {
                Port& depthPort = WVCO<TBase>::inputs[LINEAR_FM_DEPTH_INPUT];
                fmInputScaling *= depthPort.getPolyVoltageSimd<float_4>(baseChannel) * float_4(.1f);
                fmInputScaling = rack::simd::clamp(fmInputScaling, float_4(0), float_4(1));
            }
            dsp[bank].fmInput = fmInput * fmInputScaling;

            Port& syncPort = WVCO<TBase>::inputs[SYNC_INPUT];
            const float_4 syncInput = syncPort.getPolyVoltageSimd<float_4>(baseChannel);
            float_4 v = dsp[bank].step(syncInput); 
            WVCO<TBase>::outputs[MAIN_OUTPUT].setVoltageSimd(v, baseChannel);
        }
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
            ret = {1.f, 16.0f, 1, "Frequency ratio"};
            break;
        case WVCO<TBase>::FINE_TUNE_PARAM:
            ret = {-12.0f, 12.0f, 0, "Fine tune"};
            break;
        case WVCO<TBase>::FM_DEPTH_PARAM:
         ret = {.0f, 100.0f, 0, "Frequency modulation"};
            break;
        case WVCO<TBase>::LINEAR_FM_DEPTH_PARAM:
            ret = {0, 100, 0, "Through-zero FM depth"};
            break;
        case WVCO<TBase>::WAVESHAPE_GAIN_PARAM:
            ret = {0, 100, 0, "Shape modulation"};
            break;
        case WVCO<TBase>::WAVE_SHAPE_PARAM:
            ret = {0, 2, 0, "Wave shape"};
            break;
        case WVCO<TBase>::FEEDBACK_PARAM:
            ret = {0, 100, 0, "FM feedback depth"};
            break;
        case WVCO<TBase>::ATTACK_PARAM:
            ret = {0, 100, 50, "ADSR Attack"};
            break;
        case WVCO<TBase>::DECAY_PARAM:
            ret = {0, 100, 50, "ADSR Decay"};
            break;
        case WVCO<TBase>::SUSTAIN_PARAM:
            ret = {0, 100, 50, "ADSR Sustain"};
            break;
        case WVCO<TBase>::RELEASE_PARAM:
            ret = {0, 100, 50, "ADSR Release"};
            break;
        case WVCO<TBase>::OUTPUT_LEVEL_PARAM:
            ret = {0, 100, 100, "Output level"};
            break;
        case WVCO<TBase>::ADSR_SHAPE_PARAM:
            ret = {0, 1, 0, "ADSR->shape"};
            break;
        case WVCO<TBase>::ADSR_FBCK_PARAM:
         ret = {0, 1, 0, "ADSR->feedback"};
            break;
        case WVCO<TBase>::ADSR_OUTPUT_LEVEL_PARAM:
         ret = {0, 1, 0, "ADSR->output Level"};
            break;
        case WVCO<TBase>::ADSR_LFM_DEPTH_PARAM:
         ret = {0, 1, 0, "ADSR->FM Depth"};
            break;
        case WVCO<TBase>::SNAP_PARAM:
            ret = {0, 2, 0, "ADSR Snap"};
            break;
         case WVCO<TBase>::SNAP2_PARAM:
            ret = {0, 1, 0, "ARSR Snap2"};
            break;
        default:
            assert(false);
    }
    return ret;
}
#endif
