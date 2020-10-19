
#pragma once

#include <assert.h>
#include <memory>
#include "Divider.h"
#include "IComposite.h"
#include "Limiter.h"
#include "SqMath.h"
#include "SqPort.h"
#include "StateVariableFilter2.h"

#include "dsp/common.hpp"
#include "dsp/approx.hpp"
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
    //void step() override;
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

    float_4 outputGain_n = 0;
    bool limiterEnabled_n = 0;
    int numChannels_m = 0;
    int numBanks_m = 0;

    Divider divn;
    Divider divm;
    void stepn();
    void stepm();
    void setupFreq();
    void setupModes();
    void setupLimiter();

    static float_4 fastQFunc(float_4 qV, int numStages);
    static std::pair<float_4, float_4> fastFcFunc(float_4 freqVolts, float_4 rVolts, float oversample, float sampleTime);

    const int oversample = 4;
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

#if 1
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
#endif


template <class TBase>
inline float_4 F2_Poly<TBase>::fastQFunc(float_4 qV, int numStages)
{
  //  assert(qV >= 0);
 //   assert(qV <= 10);
    assert(numStages >=1 && numStages <= 2);

    const float expMult = (numStages == 1) ? 1 / 1.5f : 1 / 2.5f;
    float_4 q = rack::dsp::approxExp2_taylor5(qV * expMult) - .5;
    return q;
}

template <class TBase>
inline std::pair<float_4, float_4> F2_Poly<TBase>::fastFcFunc(float_4 freqVolts, float_4 rVolts, float oversample, float sampleTime) {
    assert(oversample == 4);
    assert(sampleTime < .0001);
    float_4 r =  rack::dsp::approxExp2_taylor5(rVolts/3.f);
    float_4 freq =  rack::dsp::FREQ_C4 *  rack::dsp::approxExp2_taylor5(freqVolts + 30 - 4) / 1073741824;

    freq /= oversample;
    freq *= sampleTime;

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
    for (int bank = 0; bank < numBanks_m; bank++) {
        const int baseChannel = 4 * bank;
        SqInput& qPort = TBase::inputs[Q_INPUT];

        float_4 qVolts = F2_Poly<TBase>::params[Q_PARAM].value;
        qVolts += qPort.getPolyVoltageSimd<float_4>(baseChannel);
        qVolts = rack::simd::clamp(qVolts, 0, 10);
        float_4 q = fastQFunc(qVolts, numStages);
        params1[bank].setQ(q);
        params2[bank].setQ(q);

        outputGain_n = 1 / q;
        if (numStages == 2) {
            outputGain_n *= 1 / q;
        }
        outputGain_n = SimdBlocks::min(outputGain_n, float_4(1.f));

        SqInput& rPort = TBase::inputs[R_INPUT];
        float_4 rVolts = F2_Poly<TBase>::params[R_PARAM].value;
        rVolts += rPort.getPolyVoltageSimd<float_4>(baseChannel);
        rVolts = rack::simd::clamp(rVolts, 0, 10);

        SqInput& fcPort = TBase::inputs[FC_INPUT];
        float_4 fVolts = F2_Poly<TBase>::params[FC_PARAM].value;
        fVolts += fcPort.getPolyVoltageSimd<float_4>(baseChannel);
        fVolts = rack::simd::clamp(fVolts, 0, 10);

        auto fr = fastFcFunc(fVolts, rVolts, oversample, sampleTime);

       // freq /= oversample;
      //  freq *= sampleTime;
        params1[bank].setFreq(fr.first);
        params2[bank].setFreq(fr.second);
    }
}

#if 0
template <class TBase>
inline void F2_Poly<TBase>::setupFreq()
{
    const float sampleTime = TBase::engineGetSampleTime();
    const int topologyInt = int( std::round(F2_Poly<TBase>::params[TOPOLOGY_PARAM].value));
    const int numStages = (topologyInt == 0) ? 1 : 2; 

    // printf("setupFreq, numB = %d\n", numBanks_m); fflush(stdout);

    for (int bank = 0; bank < numBanks_m; bank++) {
        const int baseChannel = 4 * bank;
        {
            SqInput& qPort = TBase::inputs[Q_INPUT];
            float_4 qVolts = F2_Poly<TBase>::params[Q_PARAM].value;
            qVolts += qPort.getPolyVoltageSimd<float_4>(baseChannel);
            qVolts = rack::simd::clamp(qVolts, 0, 10);

            // const float q =  std::exp2(qVolts/1.5f + 20 - 4) / 10000;
            // probably will have to change when we use the SIMD approx.
            // I doubt this function works with numbers this small.

            // 
            // 1/ 3 reduced q too much at 24
    
            const float expMult = (numStages == 1) ? 1 / 1.5f : 1 / 2.5f;

            // TODO: make this fast
            float_4 q;
            for (int i=0; i<4; ++i) {
                q[i] = std::exp2(qVolts[i] * expMult) - .5;
            }
           // const float q =  std::exp2(qVolts * expMult) - .5;
            params1[bank].setQ(q);
            params2[bank].setQ(q);

            outputGain_n = 1 / q;
            if (numStages == 2) {
                outputGain_n *= 1 / q;
            }
            outputGain_n = SimdBlocks::min(outputGain_n, float_4(1.f));
        // printf("Q = %f outGain = %f\n", q, outputGain_n); fflush(stdout);
        }

        {
            SqInput& rPort = TBase::inputs[R_INPUT];
            float_4 rVolts = F2_Poly<TBase>::params[R_PARAM].value;
            rVolts += rPort.getPolyVoltageSimd<float_4>(baseChannel);
            rVolts = rack::simd::clamp(rVolts, 0, 10);

            // TODO: make this fast
            //const float rx = std::exp2(rVolts/3.f);
            //const float r = rx; 
            float_4 r;
            for (int i=0; i<4; ++i) {
                r[i] = std::exp2(rVolts[i]/3.f);
            }

        //   printf("rv=%f, r=%f\n", rVolts, r); 

            SqInput& fcPort = TBase::inputs[FC_INPUT];
            float_4 freqVolts = F2_Poly<TBase>::params[FC_PARAM].value;
           //  printf("in setupFreq[%d] fv= %s\n", bank, toStr(freqVolts).c_str()); fflush(stdout);
            freqVolts += fcPort.getPolyVoltageSimd<float_4>(baseChannel);
          //   printf("2in setupFreq[%d] fv= %s\n", bank, toStr(freqVolts).c_str()); fflush(stdout);
            freqVolts = rack::simd::clamp(freqVolts, 0, 10);
          //   printf("3in setupFreq[%d] fv=%s\n", bank, toStr(freqVolts).c_str()); fflush(stdout);

            
            
            #if 0
            float freq = rack::dsp::FREQ_C4 * std::exp2(freqVolts + 30 - 4) / 1073741824;
            freq /= oversample;
            freq *= sampleTime;
            #endif
            float_4 freq;
            for (int i=0; i<4; ++i) {
                freq[i] = rack::dsp::FREQ_C4 * std::exp2(freqVolts[i] + 30 - 4) / 1073741824;
            }
            freq /= oversample;
            freq *= sampleTime;

           //  printf("i4n setupFreq[%d] freq=%s\n", bank, toStr(freqVolts).c_str()); fflush(stdout);

        //  printf("** freq 1=%f 2=%f freqXover = %f\n", freq / r, freq * r, freq * oversample); fflush(stdout);
            params1[bank].setFreq(freq / r);
            params2[bank].setFreq(freq * r);
        }
    }
}
#endif
template <class TBase>
inline void F2_Poly<TBase>::setupModes()
{
    const int modeParam = int( std::round(F2_Poly<TBase>::params[MODE_PARAM].value));
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
    for (int bank = 0; bank < numBanks_m; bank++) {
        params1[bank].setMode(mode);
        params2[bank].setMode(mode);
    }
}


template <class TBase>
inline void F2_Poly<TBase>::stepn()
{
    setupModes();
    setupFreq();
    limiterEnabled_n =  bool( std::round(F2_Poly<TBase>::params[LIMITER_PARAM].value));
}

#if 0

template <class TBase>
inline void F2_Poly<TBase>::process(const typename TBase::ProcessArgs& args)
{
    divm.step();
    divn.step();

    SqInput& inPort = TBase::inputs[AUDIO_INPUT];
    SqOutput& outPort = TBase::outputs[AUDIO_OUTPUT];
    for (int bank = 0; bank < numBanks_m; bank++) {
        const int baseChannel = 4 * bank;
        const float_4 input = inPort.getPolyVoltageSimd<float_4>(baseChannel);

        float_4 output = input;
        output = rack::simd::clamp(output, -10.f, 10.f);

        outPort.setVoltageSimd(output, baseChannel);
    }
}
#else

template <class TBase>
inline void F2_Poly<TBase>::process(const typename TBase::ProcessArgs& args)
{
    divm.step();
    divn.step();
    assert(oversample == 4);

    SqInput& inPort = TBase::inputs[AUDIO_INPUT];
    const int topologyInt = int( std::round(F2_Poly<TBase>::params[TOPOLOGY_PARAM].value));
    Topology topology = Topology(topologyInt);

    for (int bank = 0; bank < numBanks_m; bank++) {
        const int baseChannel = 4 * bank;
        const float_4 input = inPort.getPolyVoltageSimd<float_4>(baseChannel);
        //printf("got poly input: %s\n", toStr(input).c_str());
        T output = 0;
        switch(topology) {
            case Topology::SERIES:
                {
                    // series 4X
                    StateVariableFilter2<T>::run(input, state1[bank], params1[bank]);
                    StateVariableFilter2<T>::run(input, state1[bank], params1[bank]);
                    StateVariableFilter2<T>::run(input, state1[bank], params1[bank]);
                    const T temp = StateVariableFilter2<T>::run(input, state1[bank], params1[bank]);

                    StateVariableFilter2<T>::run(temp, state2[bank], params2[bank]);
                    StateVariableFilter2<T>::run(temp, state2[bank], params2[bank]);
                    StateVariableFilter2<T>::run(temp, state2[bank], params2[bank]);
                    output = StateVariableFilter2<T>::run(temp, state2[bank], params2[bank]);
                }
                break;
            case Topology::PARALLEL:
                {
                    // parallel add
                    StateVariableFilter2<T>::run(input, state1[bank], params1[bank]);
                    StateVariableFilter2<T>::run(input, state1[bank], params1[bank]);
                    StateVariableFilter2<T>::run(input, state1[bank], params1[bank]);
                    output = StateVariableFilter2<T>::run(input, state1[bank], params1[bank]);

                    StateVariableFilter2<T>::run(input, state2[bank], params2[bank]);
                    StateVariableFilter2<T>::run(input, state2[bank], params2[bank]);
                    StateVariableFilter2<T>::run(input, state2[bank], params2[bank]);
                    output += StateVariableFilter2<T>::run(input, state2[bank], params2[bank]);
                }
                break;
            case Topology::PARALLEL_INV:
                {
                    // parallel add
                    StateVariableFilter2<T>::run(input, state1[bank], params1[bank]);
                    StateVariableFilter2<T>::run(input, state1[bank], params1[bank]);
                    StateVariableFilter2<T>::run(input, state1[bank], params1[bank]);
                    output = StateVariableFilter2<T>::run(input, state1[bank], params1[bank]);

                    StateVariableFilter2<T>::run(input, state2[bank], params2[bank]);
                    StateVariableFilter2<T>::run(input, state2[bank], params2[bank]);
                    StateVariableFilter2<T>::run(input, state2[bank], params2[bank]);
                    output -= StateVariableFilter2<T>::run(input, state2[bank], params2[bank]);
                }
                break;
            case Topology::SINGLE:
                {
                    // one filter 4X
                    StateVariableFilter2<T>::run(input, state1[bank], params1[bank]);
                    StateVariableFilter2<T>::run(input, state1[bank], params1[bank]);
                    StateVariableFilter2<T>::run(input, state1[bank], params1[bank]);
                    output = StateVariableFilter2<T>::run(input, state1[bank], params1[bank]);
                }
                break;

            default: 
                assert(false);
        }

         if (limiterEnabled_n) {
            output = limiter.step(output);
        } else {
            output *= outputGain_n;
        }

        SqOutput& outPort = TBase::outputs[AUDIO_OUTPUT];
        output = rack::simd::clamp(output, -10.f, 10.f);

        outPort.setVoltageSimd(output, baseChannel);
    }
}
#endif

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
        case F2<TBase>::TOPOLOGY_PARAM:
            ret = {0, 3, 0, "Topology"};
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


