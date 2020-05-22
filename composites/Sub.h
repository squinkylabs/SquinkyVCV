
#pragma once

#ifndef _MSC_VER 
#include "asserts.h"
#include "Divider.h"
#include "SubVCO.h"
#include <assert.h>
#include <memory>
#include "IComposite.h"


namespace rack {
    namespace engine {
        struct Module;
    }
}
using Module = ::rack::engine::Module;


template <class TBase>
class SubDescription : public IComposite
{
public:
    Config getParam(int i) override;
    int getNumParams() override;
};

template <class TBase>
class Sub : public TBase
{
public:

    Sub(Module * module) : TBase(module)
    {
    }
    Sub() : TBase()
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
        OCTAVE1_PARAM,
        OCTAVE2_PARAM,
     //   SEMI1_PARAM,
     //   SEMI2_PARAM,
        FINE1_PARAM,
        FINE2_PARAM,
        SUB1_TUNE_PARAM,
        SUB2_TUNE_PARAM,
        SUB1_LEVEL_PARAM,
        SUB2_LEVEL_PARAM,
        NUM_PARAMS
    };

    enum InputIds
    {
        VOCT_INPUT,
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
        return std::make_shared<SubDescription<TBase>>();
    }

    /**
     * Main processing entry point. Called every sample
     */
    void step() override;

private:

    VoltageControlledOscillator<16, 16, float_4, int32_4> oscillators[4];

    /**
     * number of oscillator pairs
     */
    int numChannels = 1;
    int numBanks = 1;
   // float basePitch1 = 0;
  //  float basePitch2 = 0;
    Divider divn;
    void stepn();

};


template <class TBase>
inline void Sub<TBase>::init()
{
    divn.setup(4, [this]() {
        this->stepn();
    });


    oscillators[0].channels = 1;     // Totally idiotic.
}

template <class TBase>
inline void Sub<TBase>::stepn()
{
    // much of this could be done less often.
    // many vars could be float_4

    numChannels = std::max<int>(1, TBase::inputs[VOCT_INPUT].channels);
    Sub<TBase>::outputs[ Sub<TBase>::MAIN_OUTPUT].setChannels(numChannels);

    const int numVCO = numChannels * 2;
    numBanks = numVCO / 4;
    if (numVCO > numBanks * 4) {
        numBanks++;
    }


    // This is very wrong, in so many ways.
    // pitch is in volts
    const float basePitch1 = Sub<TBase>::params[OCTAVE1_PARAM].value + Sub<TBase>::params[FINE1_PARAM].value - 4;
    const float basePitch2 = Sub<TBase>::params[OCTAVE2_PARAM].value + Sub<TBase>::params[FINE2_PARAM].value - 4;

    for (int bank = 0; bank < numBanks; ++bank) {
        float_4 combinedPitch(0);
        // TODO: add pitch CV
        combinedPitch[0] = basePitch1;
        combinedPitch[1] = basePitch2;

        combinedPitch[2] = basePitch1;
        combinedPitch[3] = basePitch2;

        oscillators[bank].setPitch(combinedPitch);
    }
}

template <class TBase>
inline void Sub<TBase>::step()
{
    divn.step();
    // look at controls and update VCO

    // run the audio
    const float sampleTime = TBase::engineGetSampleTime();
    int channel = 0;
    for (int bank=0; bank < numBanks; ++bank) {
        oscillators[bank].process(sampleTime, 0);

        // now, what do do with the output? to now lets grab pairs
        // of saws and add them
        float_4 saws = oscillators[bank].saw();
        float_4 subs = oscillators[bank].sub();
        float pair = saws[0]+ saws[1] + subs[0] + subs[1]; 
        Sub<TBase>::outputs[MAIN_OUTPUT].setVoltage(pair, channel++);

        pair = saws[2]+ saws[3] + subs[2] + subs[3];   
        Sub<TBase>::outputs[MAIN_OUTPUT].setVoltage(pair, channel++);
    }
}

template <class TBase>
int SubDescription<TBase>::getNumParams()
{
    return Sub<TBase>::NUM_PARAMS;
}

template <class TBase>
inline IComposite::Config SubDescription<TBase>::getParam(int i)
{
    Config ret(0, 1, 0, "");
    switch (i) {
        case Sub<TBase>::OCTAVE1_PARAM:
            ret = {0, 10, 4, "VCO 1 octave"};
            break;
        case Sub<TBase>::OCTAVE2_PARAM:
            ret = {0, 10, 4, "VCO 2 octave"};
            break;
        case Sub<TBase>::FINE1_PARAM:
            ret = {-1, 1, 0, "VCO 1 fine tune"};
            break;
        case Sub<TBase>::FINE2_PARAM:
            ret = {-1, 1, 0, "VCO 2 fine tune"};
            break;
        case Sub<TBase>::SUB1_TUNE_PARAM:
            ret = {-1, 1, 0, "VCO 1 subharmonic divisor"};
            break;
        case Sub<TBase>::SUB2_TUNE_PARAM:
            ret = {-1, 1, 0, "VCO 2 subharmonic divisor"};
            break;
        case Sub<TBase>::SUB1_LEVEL_PARAM:
            ret = {-1, 1, 0, "VCO 1 subharmonic level"};
            break;
        case Sub<TBase>::SUB2_LEVEL_PARAM:
            ret = {-1, 1, 0, "VCO 2 subharmonic level"};
            break;
        default:
            assert(false);
    }
    return ret;
}
#endif



