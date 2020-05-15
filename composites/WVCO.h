
#pragma once

#include <assert.h>
#include <memory>
//#include "functions.hpp"
#include <simd/vector.hpp>
#include <simd/functions.hpp>



#ifndef _MSC_VER
#include "ObjectCache.h"
#include "SimdBlocks.h"
#include "IComposite.h"

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

class WVCODsp
{
public:
    enum class WaveForm {Sine, Fold, SawTri};
    float_4 step(float_4 freq, WaveForm wf) {  
        phaseAcc += freq;
		// Wrap phase
		phaseAcc -= rack::simd::floor(phaseAcc);

        const __m128 twoPi = _mm_set_ps1(2 * 3.141592653589793238);
        float_4 s = rack::simd::sin(phaseAcc * twoPi);
        s *= 5;

        if (wf == WaveForm::Fold) {
           // printf("folding\n"); fflush(stdout);
            s = SimdBlocks::fold(s);
            s *= 5;
        }

        return s;
    }
private:
    float_4 phaseAcc = float_4::zero();
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

    WVCODsp dsp[4];
    std::function<float(float)> expLookup = ObjectCache<float>::getExp2Ex();

    float_4 getOscFreq(int bank);

};


template <class TBase>
inline void WVCO<TBase>::init()
{
}


template <class TBase>
inline float_4 WVCO<TBase>::getOscFreq(int bank)
{
    float_4 freq;

    const float basePitch = -4.0f + roundf(TBase::params[OCTAVE_PARAM].value) +      
        TBase::params[FINE_TUNE_PARAM].value / 12.0f;
    for (int i=0; i<4; ++i) {
        const int channel = bank * 4 + i;

        float pitch = basePitch;
        pitch += TBase::inputs[VOCT_INPUT].getVoltage(channel);

     //   if ((i == 0) && bank == 0)
     //       printf("base pitch = %f, cv = %f\n", basePitch, TBase::inputs[VOCT_INPUT].getVoltage(channel));

        // TODO: modulation
        //pitch += .25f * TBase::inputs[PITCH_MOD_INPUT].getVoltage(0) *
        //    taper(TBase::params[PARAM_PITCH_MOD_TRIM].value);

        const float q = float(log2(261.626));       // move up to pitch range of EvenVCO

        pitch += q;
        float _freq = expLookup(pitch);

       // if ((i == 0) && bank == 0)
       //     printf("q = %f, now _freq - %f\n", q, _freq);
        
        const int freqMultiplier = int(std::round(TBase::params[FREQUENCY_MULTIPLIER_PARAM].value)); 
        _freq *= freqMultiplier;
       

        const float time = std::clamp(_freq * TBase::engineGetSampleTime(), -.5f, 0.5f);
        freq[i] = time;

       // if (i == 0 && bank==0) {
       //     printf("pitch = %f mult=%d freq = %f time=%f\n", pitch, freqMultiplier, _freq, time);
       // }
    }
    return freq;
}

template <class TBase>
inline void WVCO<TBase>::step()
{
    // get wavform from UI
    WVCODsp::WaveForm wf;
    int wfFromUI = (int) std::round(TBase::params[WAVE_SHAPE_PARAM].value);
    wf = WVCODsp::WaveForm(wfFromUI);

    // get pitch from params.
   // const int octave = (int) std::round(TBase::params[_PARAM].value);
  //  float pitchCV = octave - 3;


    
    const int numChannels = std::max<int>(1, TBase::inputs[VOCT_INPUT].channels);
    WVCO<TBase>::outputs[ WVCO<TBase>::MAIN_OUTPUT].setChannels(numChannels);

    int numBanks = numChannels / 4;
    if (numChannels > numBanks * 4) {
        numBanks++;
    }
    for (int bank=0; bank < numBanks; ++bank) {
        const float_4 freq = getOscFreq(bank);
        const int baseChannel = 4 * bank;
        float_4 v = dsp[bank].step(freq, wf);       // was .005
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
            ret = {0.f, 10.0f, 0, "Octave"};
            break;
        case WVCO<TBase>::FREQUENCY_MULTIPLIER_PARAM:
            ret = {1.f, 16.0f, 1, "[nimp] Freq Ratio"};
            break;
        case WVCO<TBase>::FINE_TUNE_PARAM:
            ret = {-12.0f, 12.0f, 0, "[nimp] Fine Tune"};
            break;
        case WVCO<TBase>::FM_DEPTH_PARAM:
         ret = {.0f, 100.0f, 0, "[nimp] Freq Mod"};
            break;
        case WVCO<TBase>::LINEAR_FM_DEPTH_PARAM:
            ret = {0, 100, 0, "[nimp] Through-zero FM depth"};
            break;
        case WVCO<TBase>::WAVESHAPE_GAIN_PARAM:
            ret = {0, 100, 0, "[nimp] Shape Mod"};
            break;
        case WVCO<TBase>::WAVE_SHAPE_PARAM:
            ret = {0, 2, 0, "[nimp] Wave Shape"};
            break;
        case WVCO<TBase>::FEEDBACK_PARAM:
            ret = {0, 100, 0, "[nimp] FM feedback depth"};
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


