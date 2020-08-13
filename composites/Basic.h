
#pragma once

#include "BasicVCO.h"
#include "Divider.h"
#include "IComposite.h"

#include <assert.h>
#include <memory>

namespace rack {
    namespace engine {
        struct Module;
    }
}
using Module = ::rack::engine::Module;


template <class TBase>
class BasicDescription : public IComposite
{
public:
    Config getParam(int i) override;
    int getNumParams() override;
};

template <class TBase>
class Basic : public TBase
{
public:

    Basic(Module * module) : TBase(module)
    {
    }
    Basic() : TBase()
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
        return std::make_shared<BasicDescription<TBase>>();
    }

    /**
     * Main processing entry point. Called every sample
     */
    //void step() override;
    void process(const typename TBase::ProcessArgs& args) override;

private:

    BasicVCO vcos[4];
    int numChannels_m = 1;      // 1..16
    
    Divider divn;
    Divider divm;

    void stepn();
    void stepm();

};


template <class TBase>
inline void Basic<TBase>::init()
{
    divn.setup(4, [this]() {
        this->stepn();
    });
    divm.setup(16, [this]() {
        this->stepm();
    });
}

template <class TBase>
inline void Basic<TBase>::stepm()
{
}

template <class TBase>
inline void Basic<TBase>::stepn()
{
}

template <class TBase>
inline void Basic<TBase>::process(const typename TBase::ProcessArgs& args)
{
    divn.step();
    divm.step();
}

template <class TBase>
int BasicDescription<TBase>::getNumParams()
{
    return Basic<TBase>::NUM_PARAMS;
}

template <class TBase>
inline IComposite::Config BasicDescription<TBase>::getParam(int i)
{
    Config ret(0, 1, 0, "");
    switch (i) {
        case Basic<TBase>::TEST_PARAM:
            ret = {-1.0f, 1.0f, 0, "Test"};
            break;
        default:
            assert(false);
    }
    return ret;
}


