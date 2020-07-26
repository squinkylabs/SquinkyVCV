
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
        TEST_PARAM,
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
    
    Divider divn;
    Divider divm;

    void stepn();
    void stepm();

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
}

template <class TBase>
inline void Sines<TBase>::stepm()
{
    numChannels_m = std::max<int>(1, TBase::inputs[VOCT_INPUT].channels);
    Sines<TBase>::outputs[MAIN_OUTPUT].setChannels(numChannels_m);

}

template <class TBase>
inline void Sines<TBase>::stepn()
{
    const float semi = PitchUtils::semitone;
    static const float pitches[12] = {
        -1, 0, 7 * semi, 1,
        1 + 7 * semi, 2, 2 + 4 * semi, 2 + 7 * semi,
        3, 0, 0, 0};
    
    for (int vx = 0; vx < numChannels_m; ++vx) {
        const float cv = Sines<TBase>::inputs[VOCT_INPUT].getVoltage(vx);
        const int baseSineIndex = numSinesPerVoices * vx;
        float_4 basePitch(cv);

        float_4 pitch = basePitch;
        pitch += float_4::load(pitches);
        sines[baseSineIndex].setPitch(pitch);

        const float* p = pitches + 4;
        pitch = basePitch;
        pitch += float_4::load(p);
        sines[baseSineIndex + 1].setPitch(pitch);

        p = pitches + 8;
        pitch = basePitch;
        pitch += float_4::load(p);
        sines[baseSineIndex + 2].setPitch(pitch);
    }
}


/*
        // can do gates is lower rate (but it's no better)
        float_4 gates[4];
        for (int i=0; i<4; ++i) {
            Port& p = TBase::inputs[GATE_INPUT];
            float_4 g = p.getVoltageSimd<float_4>(i * 4);
            float_4 gate = (g > float_4(1));
            simd_assertMask(gate);
            gates[i] = gate;
        }
        */

//#define _LOG
template <class TBase>
inline void Sines<TBase>::process(const typename TBase::ProcessArgs& args)
{
    divn.step();
    divm.step();

    const T deltaT(args.sampleTime);
    
    int lastAdsrBank = -1;
    float_4 sines4 = 0;

    for (int vx = 0; vx < numChannels_m; ++vx) {
        const int adsrBank = vx / 4;
        const int adsrBankOffset = vx - (adsrBank * 4);
     //   const bool newAdsrBank = (adsrBank != lastAdsrBank);
     //   lastAdsrBank = adsrBank;
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
            Sines<TBase>::outputs[MAIN_OUTPUT].setVoltageSimd(sines4, bankToOutput * 4);

            sines4 = 0;
        }
    }
}


#if 0
template <class TBase>
inline void Sines<TBase>::process(const typename TBase::ProcessArgs& args)
{
    divn.step();
    divm.step();

    const T deltaT(args.sampleTime);
    

    for (int vx = 0; vx < numChannels_m; ++vx) {
        const int baseSineIndex = numSinesPerVoices * vx;

        T sum;
        sum = sines[baseSineIndex + 0].process(deltaT);
        sum += sines[baseSineIndex + 1].process(deltaT);

        float s = sum[0] + sum[1] + sum[2] + sum[3];
        s += sines[baseSineIndex + 2].process(deltaT)[0];

        bool gate = Sines<TBase>::inputs[GATE_INPUT].getVoltage(vx) > 1;
        float_4 g4(0);
        if (gate) {
            g4 = float_4::mask();
        }
    //  printf("g4 is %s\n", toStr(g4).c_str()); fflush(stdout);
        float_4 env = noramAdsr[0].step(g4, args.sampleTime);

        s *= env[0];
        s *= .3;
        s = clamp(s, -10.f, 10.f);
        Sines<TBase>::outputs[MAIN_OUTPUT].setVoltage(s, 0);
    }
}
#endif

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
        case Sines<TBase>::TEST_PARAM:
            ret = {-1.0f, 1.0f, 0, "Test"};
            break;
        default:
            assert(false);
    }
    return ret;
}


