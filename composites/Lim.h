
#pragma once

#include "Limiter.h"

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

    Limiter limiter;
    void setupLimiter();

};


template <class TBase>
inline void Lim<TBase>::init()
{
    setupLimiter();
}

template <class TBase>
inline void Lim<TBase>::process(const typename TBase::ProcessArgs& args)
{

   // numChannels_m = std::max<int>(1, TBase::inputs[VOCT_INPUT].channels);
    int numChannels_m = TBase::inputs[AUDIO_INPUT].channels;
    TBase::outputs[AUDIO_OUTPUT].setChannels(numChannels_m);
    printf("just wrote %d channels\n", numChannels_m);
    printf("out chann to mee looks like %d\n", TBase::outputs[AUDIO_OUTPUT].channels);



    float_4 input;
    input[0] = Lim<TBase>::inputs[AUDIO_INPUT].getVoltage(0);
    float_4 output = limiter.step(input);
    Lim<TBase>::outputs[AUDIO_OUTPUT].setVoltage(output[0], 0);

    float_4 debug = limiter._lag()._memory();
    Lim<TBase>::outputs[DEBUG_OUTPUT].setVoltage(debug[0], 0);

}

template <class TBase>
inline void Lim<TBase>::setupLimiter()
{
    limiter.setTimes(1, 100, TBase::engineGetSampleTime());
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


