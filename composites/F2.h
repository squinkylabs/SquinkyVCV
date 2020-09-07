
#pragma once

#include <assert.h>
#include <memory>
#include "Divider.h"
#include "IComposite.h"
#include "StateVariableFilter.h"
#include "StateVariable4P.h"

namespace rack {
    namespace engine {
        struct Module;
    }
}
using Module = ::rack::engine::Module;


template <class TBase>
class F2Description : public IComposite
{
public:
    Config getParam(int i) override;
    int getNumParams() override;
};

template <class TBase>
class F2 : public TBase
{
public:

    F2(Module * module) : TBase(module)
    {
        init();
    }

    F2() : TBase()
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
        TOPOLOGY_PARAM,
        FC_PARAM,
        R_PARAM,
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
        return std::make_shared<F2Description<TBase>>();
    }

    /**
     * Main processing entry point. Called every sample
     */
    //void step() override;
    void process(const typename TBase::ProcessArgs& args) override;

private:

    using T = float;
    StateVariableFilterParams<T> params1;
    StateVariableFilterParams<T> params2;
    StateVariableFilterParams4P<T> params4p;

    StateVariableFilterState<T> state1;
    StateVariableFilterState<T> state2;
    StateVariableFilterState4P<T> state4p;

    Divider divn;


    void stepn();

};


template <class TBase>
inline void F2<TBase>::init()
{
    printf("called F2 init\n"); fflush(stdout);
     divn.setup(4, [this]() {
        this->stepn();
    });

}

template <class TBase>
inline void F2<TBase>::stepn()
{
    const float sampleTime = TBase::engineGetSampleTime();
    float fc = 500 * sampleTime;
    params1.setQ(4);
    params2.setQ(4);
    

    params1.setFreq(fc);
    params2.setFreq(fc * 4);

    params1.setMode(StateVariableFilterParams<T>::Mode::LowPass);
    params2.setMode(StateVariableFilterParams<T>::Mode::LowPass);

  //  params4p.fcg = F2<TBase>::params[FC_PARAM].value;
   
    params4p.Rg = F2<TBase>::params[R_PARAM].value;
    params4p.Qg = F2<TBase>::params[Q_PARAM].value;

//printf("\n");
    params4p.setFreqVolts(F2<TBase>::params[FC_PARAM].value, sampleTime);
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
inline void F2<TBase>::process(const typename TBase::ProcessArgs& args)
{
    divn.step();

    const float input =  F2<TBase>::inputs[AUDIO_INPUT].getVoltage(0);

    const int topology = int( std::round(F2<TBase>::params[TOPOLOGY_PARAM].value));
    float output = 0;
    switch(topology) {
        case 0:
            {
                // series
                const float temp = StateVariableFilter<T>::run(input, state1, params1);
                output = StateVariableFilter<T>::run(temp, state2, params2);
            }
            break;
        case 1:
            {
                // parallel add
                const float temp1 = StateVariableFilter<T>::run(input, state1, params1);
                const float temp2 = StateVariableFilter<T>::run(input, state2, params2);
                output = temp1 + temp2;
            }
            break;
        case 2:
            {
                // parallel subtract
                const float temp1 = StateVariableFilter<T>::run(input, state1, params1);
                const float temp2 = StateVariableFilter<T>::run(input, state2, params2);
                output = temp1 - temp2;
            }
            break;
        case 3:
            {
                // just one
                output = StateVariableFilter<T>::run(input, state1, params1);
            }
            break;
          case 4:
            {
                // for test:
               // input = 1;
 
               // output = StateVariableFilter4P<T>::run(input, state4p, params4p);
                output = StateVariableFilter4P<T>::run(input, state4p, params4p);
            }
            
            break;
        default: 
            assert(false);

    }
    // Let it go crazy while we are just experimenting
    //   assert(output < 20);
    //   assert(output > -20);
    output = std::min(20.f, output);
    output = std::max(-20.f, output);

    F2<TBase>::outputs[AUDIO_OUTPUT].setVoltage(output, 0);
}

template <class TBase>
int F2Description<TBase>::getNumParams()
{
    return F2<TBase>::NUM_PARAMS;
}

template <class TBase>
inline IComposite::Config F2Description<TBase>::getParam(int i)
{
    Config ret(0, 1, 0, "");
    switch (i) {
        case F2<TBase>::TOPOLOGY_PARAM:
            ret = {0, 4, 4, "Topology"};
            break;
        case F2<TBase>::FC_PARAM:
            ret = {0, 10, 5, "Fc"};
            break;
        case F2<TBase>::R_PARAM:
            ret = {2.4, 10, 3, "R"};
            break;
        case F2<TBase>::Q_PARAM:
            ret = {1.3, 5, 1.9, "Q"};
            break;
        default:
            assert(false);
    }
    return ret;
}


