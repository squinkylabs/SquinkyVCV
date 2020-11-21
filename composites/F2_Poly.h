
#pragma once

#include "AudioMath_4.h"
#include <assert.h>
#include <memory>
#include "Divider.h"
#include "IComposite.h"
#include "Limiter.h"
#include "SqMath.h"
#include "SqPort.h"
#include "StateVariableFilter2.h"

//#include "dsp/common.hpp"
//#include "dsp/approx.hpp"
#include "simd.h"
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
class F2_PolyDescription : public IComposite
{
public:
    Config getParam(int i) override;
    int getNumParams() override;
};

/**
 * reduce unnecessary fc calcs:
 *  *  12+lim:      8.7   
 *  12, no lim:     8.7
 *  24+lim  :       17.7
 * 16:              21.7 
 * 
 * refactor oversample:
 *  *  12+lim:      10.7   
 *  12, no lim:     10.7
 *  24+lim  :       19.7
 * 16:              28.1
 * 
 * inlined inner:
 * 
 *  12+lim:    12.5
 *  12, no lim: 12.45
 *  24+lim  :   20.0
 * 16:          34.0
 * 
 * New bench  reg    / gutted  / norm/no clamp  / start of new pointer strategy
 *  12+lim: 16.5     / 11.4     / 12.96     /   12.19
 *  12, no lim: 16.5 / 11.4     / 12.7      /   12.8
 *  24+lim  : 25.0   /16.7      / 20.1      /   18.2
 * 16: 39.6          / 26.3     / 35.2      /   28.3
 * 
 * After optimizing:
 *
 * old mono version, 1 channel:24.4
 * poly one, mono: 15.8.
 * poly one, 16 ch: 40.7 
 * 
 * with n=16 goes to 13.3, 34.6. So it's mostly audio processing.
 * 
 * First perf test of poly version:
 * 
 * old mono version, 1 channel:24.4
 * poly one, mono: 63.
 * poly one, 16 ch: 231
 * 
 * when n ==16 instead of 4
 * poly one, mono: 25.
 * poly one, 16 ch: 83
 * 
 * 
 * 
 * 
 * high freq limits with oversample
 * OS / max bandwidth lowQ / max peak freq / max atten for 10k cutoff
 *  1X      10k         12k     0db
 *  3X      10k         20k     6db
 *  5X      12k         20k     10db
 *  10      14k         20k     12db
 * 
 */
template <class TBase>
class F2_Poly : public TBase
{
public:
    using T = float_4;

    F2_Poly(Module * module) : TBase(module)
    {
        init();
    }

    F2_Poly() : TBase()
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
        FC_TRIM_PARAM,
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
        PARALLEL,
        PARALLEL_INV
    };

    /** Implement IComposite
     */
    static std::shared_ptr<IComposite> getDescription()
    {
        return std::make_shared<F2_PolyDescription<TBase>>();
    }

    /**
     * Main processing entry point. Called every sample
     */
    void process(const typename TBase::ProcessArgs& args) override;

    void onSampleRateChange() override;

    const StateVariableFilterParams2<T>& _params1() const;
    const StateVariableFilterParams2<T>& _params2() const;
private:

    StateVariableFilterParams2<T> params1[4];
    StateVariableFilterParams2<T> params2[4];
    StateVariableFilterState2<T> state1[4];
    StateVariableFilterState2<T> state2[4];
    Limiter limiter;
    StateVariableFilter2<T>::processFunction filterFunc = nullptr;
    const int oversample = 4;

    float_4 outputGain_n = 0;
    bool limiterEnabled_m = 0;
    int numChannels_m = 0;
    int numBanks_m = 0;
    Topology topology_m = Topology::SINGLE;
    float_4 lastQv[4] = {-1};
    float_4 lastRv[4] = {-1};

    float_4 lastFcVC[4] = {-1};
    float lastFcKnob = -1;
    float lastFcTrim = -1;

    float_4 processedRValue = -1;

    Divider divn;
    Divider divm;
    void stepn();
    void stepm();
    void setupFreq();
    void setupModes();
    void setupProcFunc();
    void setupLimiter();

    static float_4 fastQFunc(float_4 qV, int numStages);
    static std::pair<float_4, float_4> fastFcFunc2(float_4 freqVolts, float_4 rVolts, float oversample, float sampleTime);

    using  processFunction = void (F2_Poly<TBase>::*)(const typename TBase::ProcessArgs& args);
    processFunction procFun;

    void processOneBankSeries(const typename TBase::ProcessArgs& args);
    void processOneBank12_lim(const typename TBase::ProcessArgs& args);
    void processOneBank12_nolim(const typename TBase::ProcessArgs& args);
    void processGeneric(const typename TBase::ProcessArgs& args);

     AudioMath_4::ScaleFun scaleFc = AudioMath_4::makeScalerWithBipolarAudioTrim(0, 10, 0, 10);    
};

template <class TBase>
inline void F2_Poly<TBase>::init()
{
    divn.setup(4, [this]() {
        this->stepn();
    });
    divm.setup(16, [this]() {
        this->stepm();
    });
    setupLimiter();
}

template <class TBase>
inline void F2_Poly<TBase>::stepm()
{
    SqInput& inPort = TBase::inputs[AUDIO_INPUT];
    SqOutput& outPort = TBase::outputs[AUDIO_OUTPUT];
  
    numChannels_m = inPort.channels;
    outPort.setChannels(numChannels_m);

    numBanks_m = (numChannels_m / 4) + ((numChannels_m % 4) ? 1 : 0);

    setupModes();
    setupProcFunc();
    limiterEnabled_m =  bool( std::round(F2_Poly<TBase>::params[LIMITER_PARAM].value));
}

template <class TBase>
inline void F2_Poly<TBase>::setupLimiter()
{
    limiter.setTimes(1, 100, TBase::engineGetSampleTime());
}

template <class TBase>
inline void F2_Poly<TBase>::onSampleRateChange()
{
    setupLimiter();
}

template <class TBase>
inline const StateVariableFilterParams2<float_4>& F2_Poly<TBase>::_params1() const 
{
    return params1[0];
}

template <class TBase>
inline const StateVariableFilterParams2<float_4>& F2_Poly<TBase>::_params2() const 
{
    return params2[0];
}

template <class TBase>
inline float_4 F2_Poly<TBase>::fastQFunc(float_4 qV, int numStages)
{
    assert(numStages >=1 && numStages <= 2);

    const float expMult = (numStages == 1) ? 1 / 1.5f : 1 / 2.5f;
    float_4 q = rack::dsp::approxExp2_taylor5(qV * expMult) - .5;
    return q;
}

template <class TBase>
inline std::pair<float_4, float_4> F2_Poly<TBase>::fastFcFunc2(float_4 freqVolts, float_4 r, float oversample, float sampleTime) {
    assert(oversample == 4);
    assert(sampleTime < .0001);
    float_4 freq =  rack::dsp::FREQ_C4 *  rack::dsp::approxExp2_taylor5(freqVolts + 30 - 4) / 1073741824;

    freq /= oversample;
    freq *= sampleTime;

// we could pass in 1/r, too
    float_4 f1 = freq / r;
    float_4 f2 = freq * r;
    return std::make_pair(f1, f2);
}

template <class TBase>
inline void F2_Poly<TBase>::setupFreq()
{
    const float sampleTime = TBase::engineGetSampleTime();
    const int topologyInt = int( std::round(F2_Poly<TBase>::params[TOPOLOGY_PARAM].value));
    const int numStages = (topologyInt == 0) ? 1 : 2; 

    const bool fcKnobChanged = lastFcKnob != F2_Poly<TBase>::params[FC_PARAM].value;
    const bool fcTrimChanged = lastFcTrim != F2_Poly<TBase>::params[FC_TRIM_PARAM].value;

    lastFcKnob = F2_Poly<TBase>::params[FC_PARAM].value;
    lastFcTrim = F2_Poly<TBase>::params[FC_TRIM_PARAM].value;

    for (int bank = 0; bank < numBanks_m; bank++) {
        const int baseChannel = 4 * bank;
        SqInput& qPort = TBase::inputs[Q_INPUT];

        float_4 qVolts = F2_Poly<TBase>::params[Q_PARAM].value;
        qVolts += qPort.getPolyVoltageSimd<float_4>(baseChannel);
        qVolts = rack::simd::clamp(qVolts, 0, 10);

        int changeMask = rack::simd::movemask(qVolts != lastQv[bank]);
        if (changeMask) {
            lastQv[bank] = qVolts;
            float_4 q = fastQFunc(qVolts, numStages);
            params1[bank].setQ(q);
            params2[bank].setQ(q);

            outputGain_n = 1 / q;
            if (numStages == 2) {
                outputGain_n *= 1 / q;
            }
            outputGain_n = SimdBlocks::min(outputGain_n, float_4(1.f));
            //printf("q = %f, oututGain-n = %f\n", q[0], outputGain_n[0]);
        }

        SqInput& rPort = TBase::inputs[R_INPUT];
        float_4 rVolts = F2_Poly<TBase>::params[R_PARAM].value;
        rVolts += rPort.getPolyVoltageSimd<float_4>(baseChannel);
        rVolts = rack::simd::clamp(rVolts, 0, 10);
        const bool rChanged = rack::simd::movemask(rVolts != lastRv[bank]);
        if (rChanged) {
            lastRv[bank] = rVolts;
            processedRValue =  rack::dsp::approxExp2_taylor5(rVolts/3.f);
            //printf("rv=%f procR = %f\n", rVolts[0], processedRValue[0]);
        }

        SqInput& fcPort = TBase::inputs[FC_INPUT];
        float_4 fcCV = fcPort.getPolyVoltageSimd<float_4>(baseChannel);
        const bool fcCVChanged =  rack::simd::movemask(fcCV != lastFcVC[bank]);
        if (fcCVChanged || rChanged || fcKnobChanged || fcTrimChanged) {
            lastFcVC[bank] = fcCVChanged;

            float_4 combinedFcVoltage = scaleFc(
                fcCV,
                lastFcKnob,
                lastFcTrim);

            auto fr = fastFcFunc2(combinedFcVoltage, processedRValue, oversample, sampleTime);

            params1[bank].setFreq(fr.first);
            params2[bank].setFreq(fr.second);
        }     
    }
}

template <class TBase>
inline void F2_Poly<TBase>::setupModes()
{
    const int modeParam = int( std::round(F2_Poly<TBase>::params[MODE_PARAM].value));
    auto mode =  StateVariableFilter2<T>::Mode(modeParam);
    filterFunc = StateVariableFilter2<T>::getProcPointer(mode, oversample);

    const int topologyInt = int( std::round(F2_Poly<TBase>::params[TOPOLOGY_PARAM].value));
    topology_m = Topology(topologyInt);
}

template <class TBase>
inline void F2_Poly<TBase>::setupProcFunc()
{
    procFun = &F2_Poly<TBase>::processGeneric;
    if (numBanks_m == 1) {
        if (topology_m == Topology::SERIES) {
            procFun = &F2_Poly<TBase>::processOneBankSeries;
        } else if (topology_m == Topology::SINGLE) {
            if (limiterEnabled_m) {
                procFun = &F2_Poly<TBase>::processOneBank12_lim; 
            } else {
                procFun = &F2_Poly<TBase>::processOneBank12_nolim; 
            }
        }
    }
}


template <class TBase>
inline void F2_Poly<TBase>::stepn()
{
    setupFreq();
}


template <class TBase>
inline void F2_Poly<TBase>::process(const typename TBase::ProcessArgs& args)
{
    divm.step();
    divn.step();
    assert(oversample == 4);
    assert(procFun);
    (this->*procFun)(args);
}


template <class TBase>
inline void F2_Poly<TBase>::processOneBankSeries(const typename TBase::ProcessArgs& args) 
{
    SqInput& inPort = TBase::inputs[AUDIO_INPUT];
    const float_4 input = inPort.getPolyVoltageSimd<float_4>(0);

    const T temp = (*filterFunc)(input, state1[0], params1[0]);
    T output = (*filterFunc)(temp, state2[0], params2[0]);

    if (limiterEnabled_m) {
        output = limiter.step(output);
    } else {
        output *= outputGain_n;
    }

    SqOutput& outPort = TBase::outputs[AUDIO_OUTPUT];
    output = rack::simd::clamp(output, -10.f, 10.f);
    outPort.setVoltageSimd(output, 0);
}

template <class TBase>
inline void F2_Poly<TBase>::processOneBank12_lim(const typename TBase::ProcessArgs& args) 
{
    SqInput& inPort = TBase::inputs[AUDIO_INPUT];
    const float_4 input = inPort.getPolyVoltageSimd<float_4>(0);

    T output = (*filterFunc)(input, state1[0], params1[0]);
    output = limiter.step(output);

    SqOutput& outPort = TBase::outputs[AUDIO_OUTPUT];
    output = rack::simd::clamp(output, -10.f, 10.f);
    outPort.setVoltageSimd(output, 0);
}

template <class TBase>
inline void F2_Poly<TBase>::processOneBank12_nolim(const typename TBase::ProcessArgs& args) 
{
    SqInput& inPort = TBase::inputs[AUDIO_INPUT];
    const float_4 input = inPort.getPolyVoltageSimd<float_4>(0);

    T output = (*filterFunc)(input, state1[0], params1[0]);
    output *= outputGain_n;

    SqOutput& outPort = TBase::outputs[AUDIO_OUTPUT];
    output = rack::simd::clamp(output, -10.f, 10.f);
    outPort.setVoltageSimd(output, 0);
}

template <class TBase>
inline void F2_Poly<TBase>::processGeneric(const typename TBase::ProcessArgs& args) 
{
    SqInput& inPort = TBase::inputs[AUDIO_INPUT];
    for (int bank = 0; bank < numBanks_m; bank++) {
        const int baseChannel = 4 * bank;
        const float_4 input = inPort.getPolyVoltageSimd<float_4>(baseChannel);
        T output;
        switch(topology_m) {
            case Topology::SERIES:
                {
                    const T temp = (*filterFunc)(input, state1[bank], params1[bank]);
                    output = (*filterFunc)(temp, state2[bank], params2[bank]);
                }
                break;
            case Topology::PARALLEL:
                {
                    // parallel add
                    output = (*filterFunc)(input, state1[bank], params1[bank]);
                    output += (*filterFunc)(input, state2[bank], params2[bank]);
                }
                break;
            case Topology::PARALLEL_INV:
                {
                    // parallel sub
                    output = (*filterFunc)(input, state1[bank], params1[bank]);
                    output -= (*filterFunc)(input, state2[bank], params2[bank]);
                }
                break;
            case Topology::SINGLE:
                {
                    // one filter 4X
                    output = (*filterFunc)(input, state1[bank], params1[bank]);
                }
                break;
            default: 
                assert(false);
        }

         if (limiterEnabled_m) {
            output = limiter.step(output);
        } else {
            output *= outputGain_n;
        }

        SqOutput& outPort = TBase::outputs[AUDIO_OUTPUT];
        output = rack::simd::clamp(output, -10.f, 10.f);

        outPort.setVoltageSimd(output, baseChannel);
    }
}

template <class TBase>
int F2_PolyDescription<TBase>::getNumParams()
{
    return F2_Poly<TBase>::NUM_PARAMS;
}

template <class TBase>
inline IComposite::Config F2_PolyDescription<TBase>::getParam(int i)
{
    Config ret(0, 1, 0, "");
    switch (i) {
        case F2_Poly<TBase>::TOPOLOGY_PARAM:
            ret = {0, 3, 0, "Topology"};
            break;
        case F2_Poly<TBase>::MODE_PARAM:
            ret = {0, 3, 0, "Mode"};
            break;
        case F2_Poly<TBase>::FC_PARAM:
            ret = {0, 10, 5, "Fc"};
            break;
        case F2_Poly<TBase>::R_PARAM:
            ret = {0, 10, 0, "R"};
            break;
        case F2_Poly<TBase>::Q_PARAM:
            ret = {0, 10, 2, "Q"};
            break;
        case F2_Poly<TBase>::LIMITER_PARAM:
            ret = {0, 1, 1, "Limiter"};
            break;
        case F2_Poly<TBase>::FC_TRIM_PARAM:
         ret = {-1, 1, 0, "Fc modulation trim"};
            break;
        default:
            assert(false);
    }
    return ret;
}


