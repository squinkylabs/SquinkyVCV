
#pragma once

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
        SEMI1_PARAM,
        SEMI2_PARAM,
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
        return std::make_shared<SubDescription<TBase>>();
    }

    /**
     * Main processing entry point. Called every sample
     */
    void step() override;

private:

};


template <class TBase>
inline void Sub<TBase>::init()
{
}


template <class TBase>
inline void Sub<TBase>::step()
{
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
        case Sub<TBase>::SEMI1_PARAM:
            ret = {-12, 12, 0, "VCO 1 semitone"};
            break;
        case Sub<TBase>::SEMI2_PARAM:
            ret = {-12, 12, 0, "VCO 2 semitone"};
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


