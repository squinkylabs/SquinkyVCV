
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

/**
 * more cv, limit pitch: 16.7 / 45.9
 * 16.1 / 43.3 after CV
 * reality check before drawbar CV: 15.0 / 42.1
 * with parabolic sine aprox: 11.3/28.9 (but this could alias - need to be smarter)
 * use floor instead of comp: 16.1 /47
 * make sineVCO do nothing: 6.6, 10.8
 * set n to 16: 12.7 / 37.8 (some improvement, but not massive)
 * set m to 64, no change/
 * 
 * fixed, back to 14.6 / 41
 * percussion decay 120 / 148
 * add percussion: 14.6 /41.5
 * Perf 7/28 mono: 12.5% 4vx: 37.8
 * 
 */

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
        PERCUSSION1_PARAM,
        PERCUSSION2_PARAM,
        DECAY_PARAM,
        KEYCLICK_PARAM,
        ATTACK_PARAM,
        RELEASE_PARAM,
        NUM_PARAMS
    };

    enum InputIds
    {
        VOCT_INPUT,
        GATE_INPUT,
        DRAWBAR1_INPUT,
        DRAWBAR2_INPUT,
        DRAWBAR3_INPUT,
        DRAWBAR4_INPUT,
        DRAWBAR5_INPUT,
        DRAWBAR6_INPUT,
        DRAWBAR7_INPUT,
        DRAWBAR8_INPUT,
        DRAWBAR9_INPUT,
        ATTACK_INPUT,
        RELEASE_INPUT,
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
    void computeBaseDrawbars_m();
    void computeFinalDrawbars_n();

    const float* getDrawbarPitches() const;

    float_4 baseDrawbarVolumes_m[numDrawbars4 / 4] = {};
    float_4 basePercussionVolumes_m[numDrawbars4 / 4] = {};

    float_4 finalDrawbarVolumes_n[numDrawbars4 / 4] = {};
    float_4 finalPercussionVolumes_n[numDrawbars4 / 4] = {};

    bool lastDecayParamBool = false;
    int lastKeyclickParamInt = -1;
    float lastAttackParam = -1;
    float lastReleaseParam = -1;
    float lastAttackCV = 0;
    float lastReleaseCV = 0;
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

    for (int i = 0; i < NUM_LIGHTS; ++i) {
        Sines<TBase>::lights[i].setBrightness(3.f);
    }
}

static float gainFromSlider(float slider)
{
    float sliderDb = (slider < .5) ? -100 : (slider - 8) * 3;

    float sliderPower = std::pow(10.f, sliderDb / 10.f);
    float ret = std::sqrt(sliderPower);
    return ret;
}


//#define _LOG

template <class TBase>
inline void Sines<TBase>::computeBaseDrawbars_m()
{
    float power = 0;

    float gains[numDrawbars];

    for (int i=0; i<numDrawbars; ++i) {
        float slider = Sines<TBase>::params[DRAWBAR1_PARAM + i].value;
        // 8 is 0db, 7 -is -3db 0 is off
        float sliderDb = (slider < .5) ? -100 : (slider - 8) * 3;

        float sliderPower = std::pow(10.f, sliderDb / 10.f);
        gains[i] = std::sqrt(sliderPower);
        power += sliderPower;
    }

    float gainComp = 1;
    if (power > 1) {
        gainComp  = 1.f / std::sqrt(power);
    }

    baseDrawbarVolumes_m[2] = 0;
    for (int i=0; i<numDrawbars; ++i) {
        int bank = i / 4;
        int offset = i - (bank * 4);
        baseDrawbarVolumes_m[bank][offset] = gains[i] * gainComp;
    }

    basePercussionVolumes_m[1][0] = gainFromSlider( Sines<TBase>::params[PERCUSSION1_PARAM ].value);
    basePercussionVolumes_m[0][3] = gainFromSlider( Sines<TBase>::params[PERCUSSION2_PARAM ].value);
}

template <class TBase>
inline void Sines<TBase>::computeFinalDrawbars_n()
{
    for (int i=0; i<numDrawbars; ++i) {
        int bank = i / 4;
        int offset = i - (bank * 4);

        bool connected = Sines<TBase>::inputs[DRAWBAR1_INPUT + i].isConnected();
        finalDrawbarVolumes_n[bank][offset] = baseDrawbarVolumes_m[bank][offset] * 
            (connected ? Sines<TBase>::inputs[DRAWBAR1_INPUT + i].getVoltage(0) : 10);

    }

    for (int i=0; i<3; ++i) {
        float_4 x = finalDrawbarVolumes_n[i];
        x = SimdBlocks::ifelse(x < 0, 0, x); 
        finalDrawbarVolumes_n[i] = x * float_4(.1);

        finalPercussionVolumes_n[i] = basePercussionVolumes_m[i];     
    }
}

template <class TBase>
inline void Sines<TBase>::stepm()
{
    numChannels_m = std::max<int>(1, TBase::inputs[VOCT_INPUT].channels);
    Sines<TBase>::outputs[MAIN_OUTPUT].setChannels(numChannels_m);

    volumeNorm = 4.f / std::sqrt( float(numChannels_m));
    computeBaseDrawbars_m();
}

/**
 * input: param = {0..100}
 *      cv = {-10..10}
 * 
 * output 0..1
 */
inline float combineEnvelopeParamAndCV(float param, float cv) {
    float temp = param + 10 * cv;   // -100..+200
    temp *= 1.f / 100.f;    // -1 .. 2
    temp = std::clamp(temp, 0.f, 1.f);
    return temp;
}

template <class TBase>
inline void Sines<TBase>::stepn()
{
    computeFinalDrawbars_n();

    const float* drawbarPitches = getDrawbarPitches();
    const float sampleRate = TBase::engineGetSampleRate();
    for (int vx = 0; vx < numChannels_m; ++vx) {
        const float cv = Sines<TBase>::inputs[VOCT_INPUT].getVoltage(vx);
        const int baseSineIndex = numSinesPerVoices * vx;
        float_4 basePitch(cv);

        float_4 pitch = basePitch;
        pitch += float_4::load(drawbarPitches);
        sines[baseSineIndex].setPitch(pitch, sampleRate);

        const float* p = drawbarPitches + 4;
        pitch = basePitch;
        pitch += float_4::load(p);
        sines[baseSineIndex + 1].setPitch(pitch, sampleRate);

        p = drawbarPitches + 8;
        pitch = basePitch;
        pitch += float_4::load(p);
        sines[baseSineIndex + 2].setPitch(pitch, sampleRate);
    }

    const bool decayParamBool = Sines<TBase>::params[DECAY_PARAM].value > .5; 
    const int keyClickParamInt = int( std::round(Sines<TBase>::params[KEYCLICK_PARAM].value)); 
    const float attackParam =  Sines<TBase>::params[ATTACK_PARAM].value;
    const float releaseParam =  Sines<TBase>::params[RELEASE_PARAM].value;
    const float attackCV = Sines<TBase>::inputs[ATTACK_INPUT].getVoltage(0);
    const float releaseCV = Sines<TBase>::inputs[RELEASE_INPUT].getVoltage(0);
    // this could easily be done in stepm, also, but with this change check it should be ok.
    if ((lastDecayParamBool != decayParamBool) || 
        (lastKeyclickParamInt != keyClickParamInt) ||
        (lastAttackParam != attackParam) ||
        (lastReleaseParam != releaseParam) ||
        (lastAttackCV != attackCV) ||
        (lastReleaseCV != releaseCV)
        ) {

        lastDecayParamBool = decayParamBool;
        lastKeyclickParamInt = keyClickParamInt;
        lastAttackParam = attackParam;
        lastReleaseParam = releaseParam;
        lastReleaseCV = releaseCV;
        lastAttackCV = attackCV;

        const float decay =  decayParamBool ? .5 : .7;

        float attack = combineEnvelopeParamAndCV(attackParam, attackCV);
        float release = combineEnvelopeParamAndCV(releaseParam, releaseCV);
        float attackMult = 1;
        float releaseMult = 1;
        
        if (keyClickParamInt) {
            if (attack < .1) {
                attack = 0;           
                attackMult = 10;
            }
            if (release < .1) {
                release = 0;
                releaseMult = 2;
            }
        }

        for (int i = 0; i < numEgNorm; ++i) {
            normAdsr[i].setA(attack, attackMult);
            normAdsr[i].setD(release, releaseMult);
            normAdsr[i].setS(1);
            normAdsr[i].setR(release, releaseMult);
        }

        for (int i = 0; i < numEgPercussion; ++i) {
            percAdsr[i].setA(attack, attackMult); 
            percAdsr[i].setD(decay, 1);    
            percAdsr[i].setS(0);    
            percAdsr[i].setR(release, releaseMult);       
        }
    }
}

template <class TBase>
inline void Sines<TBase>::process(const typename TBase::ProcessArgs& args)
{
    divn.step();
    divm.step();

    const T deltaT(args.sampleTime);
    
    float_4 sines4 = 0;
    float_4 percSines4 = 0;

    for (int vx = 0; vx < numChannels_m; ++vx) {
        const int adsrBank = vx / 4;
        const int adsrBankOffset = vx - (adsrBank * 4);
        const int baseSineIndex = numSinesPerVoices * vx;

        // for each voice we must compute all the sines for that voice.
        // Add them all together into sum, which will be the non-percussion
        //mix of all drawbars.
        T sum;
        sum = sines[baseSineIndex + 0].process(deltaT) *  finalDrawbarVolumes_n[0];
        sum += sines[baseSineIndex + 1].process(deltaT) *  finalDrawbarVolumes_n[1];

        float s = sum[0] + sum[1] + sum[2] + sum[3];
        s += sines[baseSineIndex + 2].process(deltaT)[0] *  finalDrawbarVolumes_n[2][0];
        sines4[adsrBankOffset] = s;

        sum = sines[baseSineIndex + 0].get() *  finalPercussionVolumes_n[0];
        sum += sines[baseSineIndex + 1].get() *  finalPercussionVolumes_n[1];
        s = sum[0] + sum[1] + sum[2] + sum[3];
        percSines4[adsrBankOffset] = s;

        // after each block of [up to] 4 voices, run the ADSRs
        bool outputNow = false;
        int bankToOutput = 0;

        const bool gateConnected = TBase::inputs[GATE_INPUT].isConnected();

        // If we fill up a whole block, output it now - it's the voltages from the
        // previous bank.
        if (adsrBankOffset == 3) {
            outputNow = true;
            bankToOutput = adsrBank;
        }

        // If it's the last voice and we only have a partial block, output - it's the voltages
        // for the current bank
        else if (vx == (numChannels_m - 1)) {
            outputNow = true;
            bankToOutput = adsrBank;
        }

        float_4 gate4 = 0;
        if (outputNow) {
            if (gateConnected) {
                Port& p = TBase::inputs[GATE_INPUT];
                float_4 g = p.getVoltageSimd<float_4>(bankToOutput * 4);
                gate4 = (g > float_4(1));
                simd_assertMask(gate4);
                float_4 normEnv = normAdsr[bankToOutput].step(gate4, args.sampleTime);
                sines4 *= normEnv;
            }

            sines4 *= volumeNorm;

            if (gateConnected) {
                float_4 percEnv = percAdsr[bankToOutput].step(gate4, args.sampleTime);
                percSines4 *= percEnv;
            }
            sines4 += percSines4;

            Sines<TBase>::outputs[MAIN_OUTPUT].setVoltageSimd(sines4, bankToOutput * 4);
            sines4 = 0;
            percSines4 = 0;
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
            ret = {0.f, 8.0f, 8, "16' volume"};            // brown
            break;
        case Sines<TBase>::DRAWBAR2_PARAM:
            ret = {0.f, 8.0f, 8, "5 1/3' volume"};         //brown (g above middle c)
            break;
        case Sines<TBase>::DRAWBAR3_PARAM:          // white (MIDDLE c)
            ret = {0.f, 8.0f, 8, "8' volume"};
            break;
        case Sines<TBase>::DRAWBAR4_PARAM:
            ret = {0.f, 8.0f, 8, "4' volume"};             // white C above middle C
            break;
        case Sines<TBase>::DRAWBAR5_PARAM:          // black . G octave and half above middle C
            ret = {0.f, 8.0f, 8, "2 2/3' volume"};
            break;
        case Sines<TBase>::DRAWBAR6_PARAM:          // white C two oct above middle c 
            ret = {0.f, 8.0f, 8, "2' volume"};
            break;
        case Sines<TBase>::DRAWBAR7_PARAM:          // black E above c + 2 oct.       
            ret = {0.f, 8.0f, 8, "1 3/5' volume"};
            break;
        case Sines<TBase>::DRAWBAR8_PARAM:          // black g 2+ oct above middle C
            ret = {0.f, 8.0f, 8, "1 1/3' volume"};
            break;
        case Sines<TBase>::DRAWBAR9_PARAM:          //white
            ret = {0.f, 8.0f, 8, "1' volume"};
            break;
        case Sines<TBase>::PERCUSSION1_PARAM:
             ret = {0.f, 8.0f, 0, "2 2/3' percussion volume"};
             break;
        case Sines<TBase>::PERCUSSION2_PARAM:
             ret = {0.f, 8.0f, 0, "4' percussion volume"};
             break;
        case Sines<TBase>::DECAY_PARAM:
             ret = {0.f, 1.0f, 1, "percussion decay time"};
             break;
        case Sines<TBase>::KEYCLICK_PARAM:
             ret = {0.f, 1.0f, 0, "key click"};
             break;
        case Sines<TBase>::ATTACK_PARAM:
            ret = {0.f, 100.0f, 0, "attack time"};
             break;
        case Sines<TBase>::RELEASE_PARAM:
            ret = {0.f, 100.0f, 0, "release time"};
             break;
        default:
            assert(false);
    }
    return ret;
}

template <class TBase>
inline const float* Sines<TBase>::getDrawbarPitches() const {
    static float values[12] = {
   //16, 5 1/3,                  8, 4
        -1, -1 + 7 * PitchUtils::semitone, 0, 1,
         // 2 2/3,                      2, 1 3/5,                       1 1/3
         1 + 7 * PitchUtils::semitone, 2, 2 + 4 * PitchUtils::semitone, 2 + 7 * PitchUtils::semitone,
         // 1
         3, 0, 0, 0};
    return values;
}



