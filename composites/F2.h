
#pragma once

#include <assert.h>
#include <memory>
#include "Divider.h"
#include "IComposite.h"
#include "StateVariableFilter.h"

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

    StateVariableFilterState<T> state1;
    StateVariableFilterState<T> state2;


    Divider divn;


    void stepn();

};


template <class TBase>
inline void F2<TBase>::init()
{
     divn.setup(4, [this]() {
        this->stepn();
    });

}

template <class TBase>
inline void F2<TBase>::stepn()
{
    const float sampleTime = TBase::engineGetSampleTime();
  //  float fc = 500 * sampleTime;

    const float q = F2<TBase>::params[Q_PARAM].value;
    params1.setQ(q);
    params2.setQ(q);
    
    params1.setMode(StateVariableFilterParams<T>::Mode::LowPass);
    params2.setMode(StateVariableFilterParams<T>::Mode::LowPass);

    const float r = F2<TBase>::params[R_PARAM].value;

    float freqVolts = F2<TBase>::params[FC_PARAM].value;
    float freq = rack::dsp::FREQ_C4 * std::exp2(freqVolts + 30 - 4) / 1073741824;
    freq *= sampleTime;
    params1.setFreq(freq);
    params2.setFreq(freq * r);
  //  printf("freq = %f\n", freq); fflush(stdout);

}

template <class TBase>
inline void F2<TBase>::process(const typename TBase::ProcessArgs& args)
{
    divn.step();

    const float input =  F2<TBase>::inputs[AUDIO_INPUT].getVoltage(0);

    const int topology = int( std::round(F2<TBase>::params[TOPOLOGY_PARAM].value));
    float output = 0;
    switch(topology) {
        case 3:
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
        case 0:
            {
                // just one
                output = StateVariableFilter<T>::run(input, state1, params1);
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
            ret = {0, 3, 0, "Topology"};
            break;
        case F2<TBase>::FC_PARAM:
            ret = {0, 10, 5, "Fc"};
            break;
        case F2<TBase>::R_PARAM:
            ret = {1, 10, 1, "R"};
            break;
        case F2<TBase>::Q_PARAM:
            ret = {1.3, 5, 1.9, "Q"};
            break;
        default:
            assert(false);
    }
    return ret;
}


