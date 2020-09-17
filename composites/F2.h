
#pragma once

#include <assert.h>
#include <memory>
#include "Divider.h"
#include "IComposite.h"
#include "Limiter.h"
#include "SqMath.h"
#include "StateVariableFilter2.h"

#include "dsp/common.hpp"
#include <algorithm>


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
class F2Description : public IComposite
{
public:
    Config getParam(int i) override;
    int getNumParams() override;
};

/**
 * high freq limits with oversample
 * OS / max bandwidth lowQ / max peak freq / max atten for 10k cutoff
 *  1X      10k         12k     0db
 *  3X      10k         20k     6db
 *  5X      12k         20k     10db
 *  10      14k         20k     12db
 * 
 */
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

    void init();

    enum ParamIds
    {
        TOPOLOGY_PARAM,
        FC_PARAM,
        R_PARAM,
        Q_PARAM,
        MODE_PARAM,
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
        AUDIO_OUTPUT,
        NUM_OUTPUTS
    };

    enum LightIds
    {
        NUM_LIGHTS
    };

    enum class Topology
    {
        SINGLE,
        SERIES,
        PARALLEL
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

    void onSampleRateChange() override;

private:

    using T = float;
    StateVariableFilterParams2<T> params1;
    StateVariableFilterParams2<T> params2;
    StateVariableFilterState2<T> state1;
    StateVariableFilterState2<T> state2;

    Limiter limiter;

    float outputGain_n = 0;
    bool limiterEnabled_n = 0;

    Divider divn;
    void stepn();
    void setupFreq();
    void setupModes();
    void setupLimiter();
};

template <class TBase>
inline void F2<TBase>::init()
{
     divn.setup(4, [this]() {
        this->stepn();
    });
    setupLimiter();
}

template <class TBase>
inline void F2<TBase>::setupLimiter()
{
    printf("in setup lim\n"); fflush(stdout);
    limiter.setTimes(1, 100, TBase::engineGetSampleTime());
}


template <class TBase>
inline void F2<TBase>::onSampleRateChange()
{
    setupLimiter();
}

template <class TBase>
inline void F2<TBase>::setupFreq()
{
    const float sampleTime = TBase::engineGetSampleTime();
    const int topologyInt = int( std::round(F2<TBase>::params[TOPOLOGY_PARAM].value));
    const int numStages = (topologyInt == 0) ? 1 : 2; 

    {
        float qVolts = F2<TBase>::params[Q_PARAM].value;
        qVolts += F2<TBase>::inputs[Q_INPUT].getVoltage(0);
        qVolts = std::clamp(qVolts, 0, 10);

        // const float q =  std::exp2(qVolts/1.5f + 20 - 4) / 10000;
        // probably will have to change when we use the SIMD approx.
        // I doubt this function works with numbers this small.

        // 
        // 1/ 3 reduced q too much at 24
   
        const float expMult = (numStages == 1) ? 1 / 1.5f : 1 / 2.5f;

        const float q =  std::exp2(qVolts * expMult) - .5;
        params1.setQ(q);
        params2.setQ(q);

        outputGain_n = 1 / q;
        if (numStages == 2) {
             outputGain_n *= 1 / q;
        }
        outputGain_n = std::min(outputGain_n, 1.f);
       // printf("Q = %f outGain = %f\n", q, outputGain_n); fflush(stdout);
    }


    {
        float rVolts = F2<TBase>::params[R_PARAM].value;
        rVolts += F2<TBase>::inputs[R_INPUT].getVoltage(0);
        rVolts = std::clamp(rVolts, 0, 10);

        const float rx = std::exp2(rVolts/3.f);
        const float r = rx; 
    //   printf("rv=%f, r=%f\n", rVolts, r);


        float freqVolts = F2<TBase>::params[FC_PARAM].value;
        freqVolts += F2<TBase>::inputs[FC_INPUT].getVoltage(0);
        freqVolts = std::clamp(freqVolts, 0, 10);

        
        float freq = rack::dsp::FREQ_C4 * std::exp2(freqVolts + 30 - 4) / 1073741824;
        freq *= sampleTime;
    //  printf("freq 1=%f 2=%f\n", freq / r, freq * r);
        params1.setFreq(freq / r);
        params2.setFreq(freq * r);
    }

    
}

template <class TBase>
inline void F2<TBase>::setupModes()
{
    const int modeParam = int( std::round(F2<TBase>::params[MODE_PARAM].value));
    StateVariableFilterParams2<T>::Mode mode;
    switch(modeParam) {
        case 0:
            mode = StateVariableFilterParams2<T>::Mode::LowPass;
            break;
         case 1:
            mode = StateVariableFilterParams2<T>::Mode::BandPass;
            break;
         case 2:
            mode = StateVariableFilterParams2<T>::Mode::HighPass;
            break;
         case 3:
            mode = StateVariableFilterParams2<T>::Mode::Notch;
            break;
        default: 
            assert(false);
    }
    // BandPass, LowPass, HiPass, Notch
    params1.setMode(mode);
    params2.setMode(mode);
}

template <class TBase>
inline void F2<TBase>::stepn()
{
    setupModes();
    setupFreq();
    limiterEnabled_n =  bool( std::round(F2<TBase>::params[LIMITER_PARAM].value));
}

template <class TBase>
inline void F2<TBase>::process(const typename TBase::ProcessArgs& args)
{
    divn.step();

    const float input =  F2<TBase>::inputs[AUDIO_INPUT].getVoltage(0);

    const int topologyInt = int( std::round(F2<TBase>::params[TOPOLOGY_PARAM].value));
    Topology topology = Topology(topologyInt);
    float output = 0;
    switch(topology) {
        case Topology::SERIES:
            {
                // series 4X
                StateVariableFilter2<T>::run(input, state1, params1);
                StateVariableFilter2<T>::run(input, state1, params1);
                StateVariableFilter2<T>::run(input, state1, params1);
                const float temp = StateVariableFilter2<T>::run(input, state1, params1);

                StateVariableFilter2<T>::run(temp, state2, params2);
                StateVariableFilter2<T>::run(temp, state2, params2);
                StateVariableFilter2<T>::run(temp, state2, params2);
                output = StateVariableFilter2<T>::run(temp, state2, params2);
            }
            break;
        case Topology::PARALLEL:
            {
                // parallel add
                StateVariableFilter2<T>::run(input, state1, params1);
                StateVariableFilter2<T>::run(input, state1, params1);
                StateVariableFilter2<T>::run(input, state1, params1);
                output = StateVariableFilter2<T>::run(input, state1, params1);

                StateVariableFilter2<T>::run(input, state2, params2);
                StateVariableFilter2<T>::run(input, state2, params2);
                StateVariableFilter2<T>::run(input, state2, params2);
                output += StateVariableFilter2<T>::run(input, state2, params2);
            }
            break;
        case Topology::SINGLE:
            {
                // one filter 4X
                StateVariableFilter2<T>::run(input, state1, params1);
                StateVariableFilter2<T>::run(input, state1, params1);
                StateVariableFilter2<T>::run(input, state1, params1);
                output = StateVariableFilter2<T>::run(input, state1, params1);
            }
            break;

        default: 
            assert(false);
    }

    if (limiterEnabled_n) {
        output = limiter.step(output)[0];
    } else {
        output *= outputGain_n;
    }
    output = std::min(10.f, output);
    output = std::max(-10.f, output);

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
            ret = {0, 2, 0, "Topology"};
            break;
        case F2<TBase>::MODE_PARAM:
            ret = {0, 3, 0, "Mode"};
            break;
        case F2<TBase>::FC_PARAM:
            ret = {0, 10, 5, "Fc"};
            break;
        case F2<TBase>::R_PARAM:
            ret = {0, 10, 0, "R"};
            break;
        case F2<TBase>::Q_PARAM:
            ret = {0, 10, 2, "Q"};
            break;
        case F2<TBase>::LIMITER_PARAM:
            ret = {0, 1, 1, "Limiter"};
            break;
        default:
            assert(false);
    }
    return ret;
}


