
#pragma once

#include "ADSR4.h"
#include "Divider.h"
#include "IComposite.h"
#include "PitchUtils.h"
#include "SinesVCO.h"

#include <algorithm>
#include <assert.h>
#include <memory>

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
class SinesDescription : public IComposite
{
public:
    Config getParam(int i) override;
    int getNumParams() override;
};

template <class TBase>
class Sines : public TBase
{
public:
    using T = float_4;

    Sines(Module * module) : TBase(module)
    {
    }
    Sines() : TBase()
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
        DRAWBAR1_PARAM,
        DRAWBAR2_PARAM,
        DRAWBAR3_PARAM,
        DRAWBAR4_PARAM,
        DRAWBAR5_PARAM,
        DRAWBAR6_PARAM,
        DRAWBAR7_PARAM,
        DRAWBAR8_PARAM,
        DRAWBAR9_PARAM,
        NUM_PARAMS
    };

    enum InputIds
    {
        VOCT_INPUT,
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
        DRAWBAR1_LIGHT,
        DRAWBAR2_LIGHT,
        DRAWBAR3_LIGHT,
        DRAWBAR4_LIGHT,
        DRAWBAR5_LIGHT,
        DRAWBAR6_LIGHT,
        DRAWBAR7_LIGHT,
        DRAWBAR8_LIGHT,
        DRAWBAR9_LIGHT,
        NUM_LIGHTS
    };

    /** Implement IComposite
     */
    static std::shared_ptr<IComposite> getDescription()
    {
        return std::make_shared<SinesDescription<TBase>>();
    }

    /**
     * Main processing entry point. Called every sample
     */
    //void step() override;
    void process(const typename TBase::ProcessArgs& args) override;

private:
    static const int numVoices = 16;
    static const int numDrawbars = 9;
    static const int numDrawbars4 = 12;    // round up to even SIMD per voice
    static const int numSinesPerVoices = numDrawbars4 / 4;
    static const int numSines = numVoices * numSinesPerVoices;
    static const int numEgNorm = numVoices / 4;
    static const int numEgPercussion = numEgNorm;

    SinesVCO<T> sines[numSines];
    ADSR4 normAdsr[numEgNorm];
    ADSR4 percAdsr[numEgPercussion];

    int numChannels_m = 1;      // 1..16
    float volumeNorm = 1;
    
    Divider divn;
    Divider divm;

    void stepn();
    void stepm();

    static float drawbarPitches[12];

};


template <class TBase>
inline void Sines<TBase>::init()
{
    divn.setup(4, [this]() {
        this->stepn();
    });
    divm.setup(16, [this]() {
        this->stepm();
    });

    // 1 was insnely slow and broken
    // .1 is ok .3 is slow
    for (int i = 0; i < numEgNorm; ++i) {
        float t = .05;
        normAdsr[i].setParams(t, t, 1, t);     
    }
    for (int i = 0; i < numEgPercussion; ++i) {
        float t = .05;
        percAdsr[i].setParams(t, .4, 1, t);     
    }

    for (int i = 0; i < NUM_LIGHTS; ++i) {
        Sines<TBase>::lights[i].setBrightness(3.f);
    }
}

template <class TBase>
inline void Sines<TBase>::stepm()
{
    numChannels_m = std::max<int>(1, TBase::inputs[VOCT_INPUT].channels);
    Sines<TBase>::outputs[MAIN_OUTPUT].setChannels(numChannels_m);

    volumeNorm = 1.f / float(numChannels_m);

}

template <class TBase>
inline void Sines<TBase>::stepn()
{
    const float semi = PitchUtils::semitone;
 
    
    for (int vx = 0; vx < numChannels_m; ++vx) {
        const float cv = Sines<TBase>::inputs[VOCT_INPUT].getVoltage(vx);
        const int baseSineIndex = numSinesPerVoices * vx;
        float_4 basePitch(cv);

        float_4 pitch = basePitch;
        pitch += float_4::load(drawbarPitches);
        sines[baseSineIndex].setPitch(pitch);

        const float* p = drawbarPitches + 4;
        pitch = basePitch;
        pitch += float_4::load(p);
        sines[baseSineIndex + 1].setPitch(pitch);

        p = drawbarPitches + 8;
        pitch = basePitch;
        pitch += float_4::load(p);
        sines[baseSineIndex + 2].setPitch(pitch);
    }
}

//#define _LOG
template <class TBase>
inline void Sines<TBase>::process(const typename TBase::ProcessArgs& args)
{
    divn.step();
    divm.step();

    const T deltaT(args.sampleTime);
    
    float_4 sines4 = 0;

    for (int vx = 0; vx < numChannels_m; ++vx) {
        const int adsrBank = vx / 4;
        const int adsrBankOffset = vx - (adsrBank * 4);
        const int baseSineIndex = numSinesPerVoices * vx;

        // for each voice we must compute all the sines for that voice.
        // Add them all together into sum, which will be the non-percussion
        //mix of all drawbars.
        T sum;
        sum = sines[baseSineIndex + 0].process(deltaT);
        sum += sines[baseSineIndex + 1].process(deltaT);

        float s = sum[0] + sum[1] + sum[2] + sum[3];
        s += sines[baseSineIndex + 2].process(deltaT)[0];
        sines4[adsrBankOffset] = s;

        // after each block of [up to] 4 voices, run the ADSRs
        bool outputNow = false;
        int bankToOutput = 0;

        // If we fill up a whole block, output it now - it's the voltages from the
        // previous bank.

        #ifdef _LOG
        printf("--- LOOKING FOR blocke, vx = %d newadsr bank = %d\n", vx, newAdsrBank);
        #endif
       // if (newAdsrBank && (vx != 0)) {
        if (adsrBankOffset == 3) {
            outputNow = true;
            bankToOutput = adsrBank;
            #ifdef _LOG
            printf("first case fired (block full) bto=%d\n", bankToOutput);
            #endif
 
        }

        // If it's the last voice and we only have a partial block, output - it's the voltages
        // for the current bank
        else if (vx == (numChannels_m - 1)) {
            outputNow = true;
            bankToOutput = adsrBank;
#ifdef _LOG
             printf("second case fired bto=%d\n", bankToOutput);
#endif
        }
        if (outputNow) {
#ifdef _LOG
            printf("output on vx=%d adsrbank=%d bankToOuput=%d\n sines sum=%s\n", vx, adsrBank, bankToOutput, toStr(sines4).c_str());
#endif
            Port& p = TBase::inputs[GATE_INPUT];
            float_4 g = p.getVoltageSimd<float_4>(bankToOutput * 4);
            float_4 gate4 = (g > float_4(1));
            simd_assertMask(gate4);
            float_4 normEnv = normAdsr[bankToOutput].step(gate4, args.sampleTime);
            sines4 *= normEnv;
#ifdef _LOG
            printf("polyGate = %s\n", toStr(gate4).c_str());
            printf("env = %s outCh=%d\n\n", toStr(normEnv).c_str(), bankToOutput * 4);
            fflush(stdout);
#endif
            //s = clamp(s, -10.f, 10.f);
            // WVCO<TBase>::outputs[MAIN_OUTPUT].setVoltageSimd(v, baseChannel);
            sines4 *= volumeNorm;
            Sines<TBase>::outputs[MAIN_OUTPUT].setVoltageSimd(sines4, bankToOutput * 4);

            sines4 = 0;
        }
    }
}

template <class TBase>
int SinesDescription<TBase>::getNumParams()
{
    return Sines<TBase>::NUM_PARAMS;
}

template <class TBase>
inline IComposite::Config SinesDescription<TBase>::getParam(int i)
{
    Config ret(0, 1, 0, "");
    switch (i) {
        case Sines<TBase>::DRAWBAR1_PARAM:
            ret = {0.f, 8.0f, 8, "16'"};            // brown
            break;
        case Sines<TBase>::DRAWBAR2_PARAM:
            ret = {0.f, 8.0f, 8, "5 1/3'"};         //brown (g above middle c)
            break;
        case Sines<TBase>::DRAWBAR3_PARAM:          // white (MIDDLE c)
            ret = {0.f, 8.0f, 8, "8'"};
            break;
        case Sines<TBase>::DRAWBAR4_PARAM:
            ret = {0.f, 8.0f, 8, "4'"};             // white C above middle C
            break;
        case Sines<TBase>::DRAWBAR5_PARAM:          // black . G ocatve and half a bove middle C
            ret = {0.f, 8.0f, 8, "2 2/3'"};
            break;
        case Sines<TBase>::DRAWBAR6_PARAM:          // white C two oct above middle c 
            ret = {0.f, 8.0f, 8, "2'"};
            break;
        case Sines<TBase>::DRAWBAR7_PARAM:          // black E above c + 2 oct.       
            ret = {0.f, 8.0f, 8, "1 3/5"};
            break;
        case Sines<TBase>::DRAWBAR8_PARAM:          // black g 2+ oct above middle C
            ret = {0.f, 8.0f, 8, "1 1/3'"};
            break;
        case Sines<TBase>::DRAWBAR9_PARAM:          //white
            ret = {0.f, 8.0f, 8, "1'"};
            break;
        default:
            assert(false);
    }
    return ret;
}

template <class TBase>
inline float Sines<TBase>::drawbarPitches[12] = {
        //16, 5 1/3,                  8, 4
        -1, 7 * PitchUtils::semitone, 0, 1,

         // 2 2/3,                      2, 1 3/5,                       1 1/3
         1 + 7 * PitchUtils::semitone, 2, 2 + 4 * PitchUtils::semitone, 2 + 7 * PitchUtils::semitone,

         // 1
         3, 0, 0, 0

};


