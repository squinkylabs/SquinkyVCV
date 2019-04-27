
#pragma once

#include <assert.h>
#include <memory>
#include "IComposite.h"

#ifdef __V1
namespace rack {
    namespace engine {
        struct Module;
    }
}
using Module = rack::engine::Module;
#else
namespace rack {
    struct Module;
};
using Module = rack::Module;
#endif

template <class TBase>
class FiltDescription : public IComposite
{
public:
    Config getParam(int i) override;
    int getNumParams() override;
};

template <class TBase>
class Filt : public TBase
{
public:

    Filt(Module * module) : TBase(module)
    {
    }
    Filt() : TBase()
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
        NUM_INPUTS
    };

    enum OutputIds
    {
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
        return std::make_shared<FiltDescription<TBase>>();
    }

    /**
     * Main processing entry point. Called every sample
     */
    void step() override;

private:

};


template <class TBase>
inline void Filt<TBase>::init()
{
}


template <class TBase>
inline void Filt<TBase>::step()
{
}

template <class TBase>
int FiltDescription<TBase>::getNumParams()
{
    return Filt<TBase>::NUM_PARAMS;
}

template <class TBase>
inline IComposite::Config FiltDescription<TBase>::getParam(int i)
{
    Config ret(0, 1, 0, "");
    switch (i) {
        case Filt<TBase>::TEST_PARAM:
            ret = {-1.0f, 1.0f, 0, "Test"};
            break;
        default:
            assert(false);
    }
    return ret;
}


