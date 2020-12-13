
#pragma once

#include "Sampler4vx.h"

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
class SampDescription : public IComposite
{
public:
    Config getParam(int i) override;
    int getNumParams() override;
};

template <class TBase>
class Samp : public TBase
{
public:

    Samp(Module * module) : TBase(module)
    {
    }
    Samp() : TBase()
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
        PITCH_INPUT,
        VELOCITY_INPUT,
        GATE_INPUT,
        NUM_INPUTS
    };

    enum OutputIds
    {
        AUDIO_OUTPUT,
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
        return std::make_shared<SampDescription<TBase>>();
    }

    /**
     * Main processing entry point. Called every sample
     */
    //void step() override;
    void process(const typename TBase::ProcessArgs& args) override;

private:

    Sampler4xv playback[4];         // 16 voices of polyphony

};


template <class TBase>
inline void Samp<TBase>::init()
{
}

template <class TBase>
inline void Samp<TBase>::process(const typename TBase::ProcessArgs& args)
{
}

template <class TBase>
int SampDescription<TBase>::getNumParams()
{
    return Samp<TBase>::NUM_PARAMS;
}

template <class TBase>
inline IComposite::Config SampDescription<TBase>::getParam(int i)
{
    Config ret(0, 1, 0, "");
    switch (i) {
        case Samp<TBase>::TEST_PARAM:
            ret = {-1.0f, 1.0f, 0, "Test"};
            break;
        default:
            assert(false);
    }
    return ret;
}


