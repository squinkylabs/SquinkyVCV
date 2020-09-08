
#pragma once

#include <assert.h>
#include <memory>
#include "Divider.h"
#include "IComposite.h"
#include "StateVariable4P.h"

namespace rack {
    namespace engine {
        struct Module;
    }
}
using Module = ::rack::engine::Module;


template <class TBase>
class F4Description : public IComposite
{
public:
    Config getParam(int i) override;
    int getNumParams() override;
};

template <class TBase>
class F4 : public TBase
{
public:

    F4(Module * module) : TBase(module)
    {
        init();
    }

    F4() : TBase()
    {
        init();
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
     //   TOPOLOGY_PARAM,
        FC_PARAM,
        R_PARAM,
        Q_PARAM,
        NOTCH_PARAM,
        NUM_PARAMS
    };

    enum InputIds
    {
        AUDIO_INPUT,
        NUM_INPUTS
    };

    enum OutputIds
    {
        LP_OUTPUT,
        HP_OUTPUT,
        BP_OUTPUT,
        PK_OUTPUT,
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
        return std::make_shared<F4Description<TBase>>();
    }

    /**
     * Main processing entry point. Called every sample
     */
    //void step() override;
    void process(const typename TBase::ProcessArgs& args) override;

private:

    using T = float;
    StateVariableFilterParams4P<T> params4p;
    StateVariableFilterState4P<T> state4p;
    Divider divn;

    void stepn();

};


template <class TBase>
inline void F4<TBase>::init()
{
     divn.setup(4, [this]() {
        this->stepn();
    });

}

template <class TBase>
inline void F4<TBase>::stepn()
{
    const float sampleTime = TBase::engineGetSampleTime();


    params4p.Rg = F4<TBase>::params[R_PARAM].value;
    params4p.Qg = F4<TBase>::params[Q_PARAM].value;

//printf("\n");
    params4p.setFreqVolts(F4<TBase>::params[FC_PARAM].value, sampleTime);
    #if 0
    params4p.setFreqVolts(-4);
    params4p.setFreqVolts(-3);
    params4p.setFreqVolts(-2);
    params4p.setFreqVolts(11);
    params4p.setFreqVolts(0);
    
    params4p.setFreqVolts(1);
    params4p.setFreqVolts(2);
    params4p.setFreqVolts(4);
    params4p.setFreqVolts(5);
    params4p.setFreqVolts(7);
    params4p.setFreqVolts(9);
    params4p.setFreqVolts(10);
    #endif

    //printf("just set fcg to %f\n", params4p.fcg); fflush(stdout);
}

template <class TBase>
inline void F4<TBase>::process(const typename TBase::ProcessArgs& args)
{
    divn.step();

    const float input =  F4<TBase>::inputs[AUDIO_INPUT].getVoltage(0);
    
    StateVariableFilter4P<T>::run(input, state4p, params4p);

    // Let it go crazy while we are just experimenting
    //   assert(output < 20);
    //   assert(output > -20);

    float output = state4p.lp;
    output = std::min(20.f, output);
    output = std::max(-20.f, output);
    F4<TBase>::outputs[LP_OUTPUT].setVoltage(output, 0);

    output = state4p.hp;
    output = std::min(20.f, output);
    output = std::max(-20.f, output);
    F4<TBase>::outputs[HP_OUTPUT].setVoltage(output, 0);

    output = state4p.bp;
    output = std::min(20.f, output);
    output = std::max(-20.f, output);
    F4<TBase>::outputs[BP_OUTPUT].setVoltage(output, 0);

    output = state4p.peak;
    output = std::min(20.f, output);
    output = std::max(-20.f, output);
    F4<TBase>::outputs[PK_OUTPUT].setVoltage(output, 0);
}

template <class TBase>
int F4Description<TBase>::getNumParams()
{
    return F4<TBase>::NUM_PARAMS;
}

template <class TBase>
inline IComposite::Config F4Description<TBase>::getParam(int i)
{
    Config ret(0, 1, 0, "");
    switch (i) {
        case F4<TBase>::FC_PARAM:
            ret = {0, 10, 5, "Fc"};
            break;
        case F4<TBase>::R_PARAM:
            ret = {2.4, 10, 3, "R"};
            break;
        case F4<TBase>::Q_PARAM:
            ret = {1.3, 5, 1.9, "Q"};
            break;
        case F4<TBase>::NOTCH_PARAM:
            ret = {0, 1, 0, "Notch"};
            break;
        default:
            assert(false);
    }
    return ret;
}


