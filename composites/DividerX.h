
#pragma once

#include <assert.h>
#include <memory>
#include "GateTrigger.h"
#include "IComposite.h"

namespace rack {
    namespace engine {
        struct Module;
    }
}
using Module = ::rack::engine::Module;


template <class TBase>
class DividerXDescription : public IComposite
{
public:
    Config getParam(int i) override;
    int getNumParams() override;
};

template <class TBase>
class DividerX : public TBase
{
public:

    DividerX(Module * module) : TBase(module),  inputProcessing(false)
    {
    }

    DividerX() : TBase(),  inputProcessing(false)
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
        MAIN_INPUT,
        NUM_INPUTS
    };

    enum OutputIds
    {
        FIRST_OUTPUT,
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
        return std::make_shared<DividerXDescription<TBase>>();
    }

    /**
     * Main processing entry point. Called every sample
     */
    void process(const typename TBase::ProcessArgs& args) override;

private:
    GateTrigger inputProcessing;
    int counter = 0;
    bool state = false;
};

template <class TBase>
inline void DividerX<TBase>::init()
{
}

template <class TBase>
inline void DividerX<TBase>::process(const typename TBase::ProcessArgs& args)
{
    float x = TBase::inputs[MAIN_INPUT].getVoltage(0);
    inputProcessing.go(x);
    if (inputProcessing.trigger()) {
        if (--counter < 0) {
            counter = 0;
            state = !state;
            float v = state ? 1 : -1;
            v *= 5;
            TBase::outputs[FIRST_OUTPUT].setVoltage(v, 0);
        }

    }
}

template <class TBase>
int DividerXDescription<TBase>::getNumParams()
{
    return DividerX<TBase>::NUM_PARAMS;
}

template <class TBase>
inline IComposite::Config DividerXDescription<TBase>::getParam(int i)
{
    Config ret(0, 1, 0, "");
    switch (i) {
        case DividerX<TBase>::TEST_PARAM:
            ret = {-1.0f, 1.0f, 0, "Test"};
            break;
        default:
            assert(false);
    }
    return ret;
}


