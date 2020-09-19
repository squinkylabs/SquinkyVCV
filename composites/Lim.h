
#pragma once

#include "Divider.h"
#include "Limiter.h"
#include "SqPort.h"

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
class LimDescription : public IComposite
{
public:
    Config getParam(int i) override;
    int getNumParams() override;
};

template <class TBase>
class Lim : public TBase
{
public:

    Lim(Module * module) : TBase(module)
    {
    }
    Lim() : TBase()
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
        AUDIO_INPUT,
        NUM_INPUTS
    };

    enum OutputIds
    {
        AUDIO_OUTPUT,
        DEBUG_OUTPUT,
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
        return std::make_shared<LimDescription<TBase>>();
    }

    /**
     * Main processing entry point. Called every sample
     */
    //void step() override;
    void process(const typename TBase::ProcessArgs& args) override;

    void onSampleRateChange() override;

private:

    Limiter limiters[4];
    void setupLimiter();
    void stepm();

    int numChannels_m = 0;
    int numBanks_m = 0;
    Divider divm;

};


template <class TBase>
inline void Lim<TBase>::init()
{
    setupLimiter();
    divm.setup(16, [this]() {
        this->stepm();
    });
}

template <class TBase>
inline void Lim<TBase>::stepm()
{
    SqInput& inPort = TBase::inputs[AUDIO_INPUT];
    SqOutput& outPort = TBase::outputs[AUDIO_OUTPUT];
    numChannels_m = inPort.channels;
    outPort.setChannels(numChannels_m);
    numBanks_m = (numChannels_m / 4) + ((numChannels_m % 4) ? 1 : 0);   
    // printf("\n****** after stepm banks = %d numch=%d\n", numBanks_m, numChannels_m);
}

template <class TBase>
inline void Lim<TBase>::process(const typename TBase::ProcessArgs& args)
{
    divm.step();
   
    SqInput& inPort = TBase::inputs[AUDIO_INPUT];
    SqOutput& outPort = TBase::outputs[AUDIO_OUTPUT];
    
    for (int bank = 0; bank < numBanks_m; ++bank) {
        const int baseChannel = bank * 4;
        const float_4 input = inPort.getPolyVoltageSimd<float_4>(baseChannel);
        const float_4 output = limiters[bank].step(input);
        outPort.setVoltageSimd(output, baseChannel);

     //   printf("bank=%d, ch=%d\n", bank, baseChannel);
     //   printf("input = %s output=%s\n", toStr(input).c_str(), toStr(output).c_str());

        float_4 debug = limiters[bank]._lag()._memory();
        Lim<TBase>::outputs[DEBUG_OUTPUT].setVoltageSimd(debug, baseChannel);
    }
}

template <class TBase>
inline void Lim<TBase>::setupLimiter()
{
    for (int i = 0; i<4; ++i) {
        limiters[i].setTimes(1, 100, TBase::engineGetSampleTime());
    }
}


template <class TBase>
inline void Lim<TBase>::onSampleRateChange()
{
    setupLimiter();
}

template <class TBase>
int LimDescription<TBase>::getNumParams()
{
    return Lim<TBase>::NUM_PARAMS;
}

template <class TBase>
inline IComposite::Config LimDescription<TBase>::getParam(int i)
{
    Config ret(0, 1, 0, "");
    switch (i) {
        case Lim<TBase>::TEST_PARAM:
            ret = {-1.0f, 1.0f, 0, "Test"};
            break;
        default:
            assert(false);
    }
    return ret;
}


