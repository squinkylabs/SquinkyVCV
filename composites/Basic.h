
#pragma once

#include "BasicVCO.h"
#include "Divider.h"
#include "IComposite.h"

#include "engine/Port.hpp"

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

/**
 * Initial perf:
 *      1 saw, 5.95
 */
template <class TBase>
class Basic : public TBase
{
public:

    enum class Waves
    {
        SIN,
        TRI,
        SAW,
        SQUARE,
        EVEN,
        END     // just a marker
    };

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
        WAVEFORM_PARAM,
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
    int numBanks_m = 0;
    
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
    numChannels_m = std::max<int>(1, TBase::inputs[VOCT_INPUT].channels);
    Basic<TBase>::outputs[MAIN_OUTPUT].setChannels(numChannels_m);

    numBanks_m = (numChannels_m / 4);
    numBanks_m +=((numChannels_m %4) == 0) ? 0 : 1;

    for (int i=0; i<numBanks_m; ++i) {
        vcos[i].setWaveform((BasicVCO::Waveform)(int)TBase::params[WAVEFORM_PARAM].value);
    }
}

template <class TBase>
inline void Basic<TBase>::stepn()
{
    const float sampleTime = TBase::engineGetSampleTime();
    for (int bank = 0; bank < numBanks_m; ++ bank) {
        const int baseIndex = bank * 4;
        Port& p = TBase::inputs[VOCT_INPUT];
        const float_4 cv = p.getVoltageSimd<float_4>(baseIndex);
        vcos[bank].setPitch(cv, sampleTime);
    }
}

template <class TBase>
inline void Basic<TBase>::process(const typename TBase::ProcessArgs& args)
{
    divn.step();
    divm.step();

    // TODO: move this into step_m
    BasicVCO::pfunc pp = vcos[0].getProcPointer();

    for (int bank = 0; bank < numBanks_m; ++ bank) {
      //  float_4 output = vcos[bank].process(args.sampleTime);
      float_4 output = ((&vcos[bank])->*pp)(args.sampleTime);
     // pp();
        Basic<TBase>::outputs[MAIN_OUTPUT].setVoltageSimd(output, bank * 4);
    }
}

template <class TBase>
int BasicDescription<TBase>::getNumParams()
{
    return Basic<TBase>::NUM_PARAMS;
}

template <class TBase>
inline IComposite::Config BasicDescription<TBase>::getParam(int i)
{
    const float numWaves = (float) Basic<TBase>::Waves::END;
    const float defWave = (float) Basic<TBase>::Waves::SIN;
    Config ret(0, 1, 0, "");

    switch (i) {
        case Basic<TBase>::WAVEFORM_PARAM:
            ret = {0.0f, numWaves, defWave, "Waveform"};
            break;
        default:
            assert(false);
    }
    return ret;
}


