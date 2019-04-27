
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
        FC_PARAM,
        Q_PARAM,
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
        case Filt<TBase>::FC_PARAM:
            ret = {-1.0f, 1.0f, 0, "Cutoff Freq"};
            break;
        case Filt<TBase>::Q_PARAM:
            ret = {-1.0f, 1.0f, 0, "Resonance"};
            break;
        default:
            assert(false);
    }
    return ret;
}


