
#pragma once

#include <assert.h>
#include <memory>
#include "IComposite.h"

using float_4 = rack::simd::float_4;

namespace rack {
    namespace engine {
        struct Module;
    }
}
using Module = ::rack::engine::Module;


template <class TBase>
class WVCODescription : public IComposite
{
public:
    Config getParam(int i) override;
    int getNumParams() override;
};

class WVCODsp
{
public:
    float_4 step(float_4 freq) {
        phaseAcc += freq;

		// Wrap phase
		phaseAcc -= simd::floor(phaseAcc);
        return phaseAcc;
    }
private:
    float_4 phaseAcc;
};

template <class TBase>
class WVCO : public TBase
{
public:

    WVCO(Module * module) : TBase(module)
    {
    }
    WVCO() : TBase()
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
        return std::make_shared<WVCODescription<TBase>>();
    }

    /**
     * Main processing entry point. Called every sample
     */
    void step() override;

private:

    WVCODsp dsp[4];

};


template <class TBase>
inline void WVCO<TBase>::init()
{
}


template <class TBase>
inline void WVCO<TBase>::step()
{
    for (int bank=0; bank<4; ++bank) {
        const int channel = 4 * bank;
        float_4 v = dsp[bank].step(.005);
        WVCO<TBase>::outputs[MAIN_OUTPUT].setVoltageSimd(v, channel);
    }
    // why can't I set to 16?
    WVCO<TBase>::outputs[ WVCO<TBase>::MAIN_OUTPUT].setChannels(15);
}

template <class TBase>
int WVCODescription<TBase>::getNumParams()
{
    return WVCO<TBase>::NUM_PARAMS;
}

template <class TBase>
inline IComposite::Config WVCODescription<TBase>::getParam(int i)
{
    Config ret(0, 1, 0, "");
    switch (i) {
        case WVCO<TBase>::TEST_PARAM:
            ret = {-1.0f, 1.0f, 0, "Test"};
            break;
        default:
            assert(false);
    }
    return ret;
}


