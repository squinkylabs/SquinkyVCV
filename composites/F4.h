
#pragma once

#include <assert.h>
#include <memory>
#include "Divider.h"
#include "IComposite.h"
#include "Limiter.h"
#include "StateVariable4P.h"


#ifndef _CLAMP
#define _CLAMP
namespace std {
    inline float clamp(float v, float lo, float hi)
    {
        assert(lo < hi);
        return std::min(hi, std::max(v, lo));
    }
}
#endif

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
        FC_PARAM,
        R_PARAM,
        Q_PARAM,
        NOTCH_PARAM,
        LIMITER_PARAM,
        NUM_PARAMS
    };

    enum InputIds
    {
        AUDIO_INPUT,
        FC_INPUT,
        Q_INPUT,
        R_INPUT,
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
    void process(const typename TBase::ProcessArgs& args) override;

    void onSampleRateChange() override;


    using T = float;    
    const StateVariableFilterParams4P<T>& _params1() const;
    // const StateVariableFilterParams4P<T>& _params2() const;

private:


    StateVariableFilterParams4P<T> params4p;
    StateVariableFilterState4P<T> state4p;
    Limiter limiter;

    float outputGain_n = 0;
    bool limiterEnabled_n = 0;
    Divider divn;
    const int oversample = 4;

    void stepn();
    void setupFreq();

    void setupLimiter();

};


template <class TBase>
inline void F4<TBase>::init()
{
     divn.setup(4, [this]() {
        this->stepn();
    });

    setupLimiter();
}

template <class TBase>
inline void F4<TBase>::setupLimiter()
{
    limiter.setTimes(1, 100, TBase::engineGetSampleTime());
}


template <class TBase>
inline void F4<TBase>::onSampleRateChange()
{
    setupLimiter();
}

template <class TBase>
inline const StateVariableFilterParams4P<float>&  F4<TBase>::_params1() const 
{
    return params4p;
}

template <class TBase>
inline void F4<TBase>::stepn()
{
    #if 0
    const float sampleTime = TBase::engineGetSampleTime();

    params4p.Rg = F4<TBase>::params[R_PARAM].value;
    params4p.Qg = F4<TBase>::params[Q_PARAM].value;

    float freqVolts = F4<TBase>::params[FC_PARAM].value;
    float freq = rack::dsp::FREQ_C4 * std::exp2(freqVolts + 30 - 4) / 1073741824;
    freq *= sampleTime;
    params4p.setFreq(freq);
    #endif

    params4p.setNotch( bool( std::round(F4<TBase>::params[NOTCH_PARAM].value)));

    setupFreq();
    limiterEnabled_n =  bool( std::round(F4<TBase>::params[LIMITER_PARAM].value));
}


template <class TBase>
inline void F4<TBase>::setupFreq()
{
    const float sampleTime = TBase::engineGetSampleTime();
  //  const int topologyInt = int( std::round(F4<TBase>::params[TOPOLOGY_PARAM].value));
  //  const int numStages = (topologyInt == 0) ? 1 : 2; 

  printf("f4::setup freq\n");

    {
        float qVolts = F4<TBase>::params[Q_PARAM].value;
        qVolts += F4<TBase>::inputs[Q_INPUT].getVoltage(0);
        qVolts = std::clamp(qVolts, 0, 10);

        // const float q =  std::exp2(qVolts/1.5f + 20 - 4) / 10000;
        // probably will have to change when we use the SIMD approx.
        // I doubt this function works with numbers this small.

        // 
        // 1/ 3 reduced q too much at 24
   
      //  const float expMult = (numStages == 1) ? 1 / 1.5f : 1 / 2.5f;
        const float expMult = 1/2.f; 
        const float q =  std::exp2(qVolts * expMult) - .5;
        printf("qvolt = %f giving q = %f\n", qVolts, q);
        params4p.setQ(q);


        outputGain_n = 1 / q;

        outputGain_n = std::min(outputGain_n, 1.f);
       // printf("Q = %f outGain = %f\n", q, outputGain_n); fflush(stdout);
    }

    {
        float rVolts = F4<TBase>::params[R_PARAM].value;
        rVolts += F4<TBase>::inputs[R_INPUT].getVoltage(0);
        rVolts = std::clamp(rVolts, 0, 10);

        const float rx = std::exp2(rVolts/3.f);
        const float r = rx; 
    //   printf("rv=%f, r=%f\n", rVolts, r);


        float freqVolts = F4<TBase>::params[FC_PARAM].value;
        freqVolts += F4<TBase>::inputs[FC_INPUT].getVoltage(0);
        freqVolts = std::clamp(freqVolts, 0, 10);

        
        float freq = rack::dsp::FREQ_C4 * std::exp2(freqVolts + 30 - 4) / 1073741824;
        freq /= oversample;
        freq *= sampleTime;
     // printf("** freq =%f freqXover = %f\n", freq , freq * oversample); fflush(stdout);
        params4p.setFreq(freq);
       // params2.setFreq(freq * r);
       params4p.setR(r);
    }

}


template <class TBase>
inline void F4<TBase>::process(const typename TBase::ProcessArgs& args)
{
    divn.step();

    const float input =  F4<TBase>::inputs[AUDIO_INPUT].getVoltage(0);
    
    assert(oversample == 4);
    StateVariableFilter4P<T>::run(input, state4p, params4p);
    StateVariableFilter4P<T>::run(input, state4p, params4p);
    StateVariableFilter4P<T>::run(input, state4p, params4p);
    StateVariableFilter4P<T>::run(input, state4p, params4p);

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
            ret = {1.3, 10, 1.9, "Q"};
            break;
        case F4<TBase>::NOTCH_PARAM:
            ret = {0, 1, 0, "Notch"};
            break;
        case F4<TBase>::LIMITER_PARAM:
            ret = {0, 1, 1, "Limiter"};
            break;
        default:
            assert(false);
    }
    return ret;
}


