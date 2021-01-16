
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

/**
 * Friday
 * pretty stable, except very high Q always makes it unstable, and very high Fc is unstable
 * at 16X the high freq stablility is much better.
 * 
 * oversample   Q   R N
 *  4           5   5       no hump, blows up > 10k    
 *              7   7 X     blows up with 2nd hump > 20k
 *              7   10 X     blows up with 2nd hump > 20k
 * Tuesday:
 * Q    R       FcDC
 * 8.6  2.4     50    blows up at 1k
 * 8.6  2.4     2     gain gets really high at 50 hz
 * 
 * 
 * Monday:
 * oversample   Q   R N
 *  4           10  3     blows up with peak at 2k
 *  8           10  3       "  " 4k
 * 
 * If we keep rG >= 2.2 it's ok at 8x
 * 
 *  4           8   10    blows up when second peak hits 20k
 *  8           8   10      40k?
 * 
 *  4           5   6       blows up at fc 8
 *  8           5   6       stable
 * 
 * 
 * 
 */
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
    Limiter limiter_lp;
    Limiter limiter_hp;
    Limiter limiter_bp;
    Limiter limiter_pk;

    float outputGain_n = 0;
    bool limiterEnabled_n = 0;
    Divider divn;
    const int oversample = 4;
    NonUniformLookupTableParams<float> qCompParams;

    void stepn();
    void setupFreq();

    void setupLimiter();
    void setupQComp();

};

template <class TBase>
inline void F4<TBase>::setupQComp()
{
    if (qCompParams.size() == 0) {
#if 0   // this was for r=4
        NonUniformLookupTable<float>::addPoint(qCompParams, .005, 1);
        NonUniformLookupTable<float>::addPoint(qCompParams, .002, 2.3);
        NonUniformLookupTable<float>::addPoint(qCompParams, .001, 4.48);
        NonUniformLookupTable<float>::addPoint(qCompParams, .0005, 6.4);
        NonUniformLookupTable<float>::addPoint(qCompParams, .0002, 6.6);
        NonUniformLookupTable<float>::addPoint(qCompParams, .0005, 6.6);

        NonUniformLookupTable<float>::addPoint(qCompParams, .01, 1.05);
        NonUniformLookupTable<float>::addPoint(qCompParams, .02, 1.76);
#endif
        // There for R = 2.2
        NonUniformLookupTable<float>::addPoint(qCompParams, .0001f, 7.97f);
        NonUniformLookupTable<float>::addPoint(qCompParams, .0002f, 7.97f);
        NonUniformLookupTable<float>::addPoint(qCompParams, .0005f, 8.37f);
        NonUniformLookupTable<float>::addPoint(qCompParams, .001f,  5.02f);
        NonUniformLookupTable<float>::addPoint(qCompParams, .002f,  3.2f);
        NonUniformLookupTable<float>::addPoint(qCompParams, .01f,  2.1f);
        NonUniformLookupTable<float>::addPoint(qCompParams, .02f, 3.1f);
        
        NonUniformLookupTable<float>::finalize(qCompParams);
    }
}

template <class TBase>
inline void F4<TBase>::init()
{
     printf("called init\n"); fflush(stdout);
     divn.setup(4, [this]() {
        this->stepn();
    });

    setupQComp();

    // setupLimiter();
    onSampleRateChange();
}

template <class TBase>
inline void F4<TBase>::setupLimiter()
{
    limiter_lp.setTimes(1, 100, TBase::engineGetSampleTime());
    limiter_hp.setTimes(1, 100, TBase::engineGetSampleTime());
    limiter_bp.setTimes(1, 100, TBase::engineGetSampleTime());
    limiter_pk.setTimes(1, 100, TBase::engineGetSampleTime());
}


template <class TBase>
inline void F4<TBase>::onSampleRateChange()
{
    setupLimiter();
    params4p.onSampleTimeChange(TBase::engineGetSampleTime());

}

template <class TBase>
inline const StateVariableFilterParams4P<float>&  F4<TBase>::_params1() const 
{
    return params4p;
}

template <class TBase>
inline void F4<TBase>::stepn()
{
    params4p.setNotch( bool( std::round(F4<TBase>::params[NOTCH_PARAM].value)));

    setupFreq();
    limiterEnabled_n =  bool( std::round(F4<TBase>::params[LIMITER_PARAM].value));
}


template <class TBase>
inline void F4<TBase>::setupFreq()
{
    const float sampleTime = TBase::engineGetSampleTime();

  //  float qDbg, rDbg, fDbg;
    float qForFilter = 1;
    float freqForFilter = .1f;
    {
        float qVolts = F4<TBase>::params[Q_PARAM].value;
        qVolts += F4<TBase>::inputs[Q_INPUT].getVoltage(0);
        qVolts = std::clamp(qVolts, 0, 10);

        // exponentiate it to stretch out the high number.
        // let's get if from more or less 0..4
        // Invert it, too

        const float expMult = .25f; 
      //  const float q =  5.6 - std::exp2(qVolts * expMult);
        qForFilter = float(5.6 - std::exp2(qVolts * expMult));
    //    qDbg = q;

      //  printf("qvolt = %f giving q = %f\n", qVolts, q);
     //   params4p.setQ(q);


        const float qBar = 4 - qForFilter;
        outputGain_n = 1;
       const float qBarCutoff = 1.5;
       if (qBar < qBarCutoff) {
           outputGain_n = 1;
       } else {
           float gainReduction = 1 + (qBar - qBarCutoff);
           //printf("cut=%f reduct=%f\n", qBarCutoff, gainReduction);
           outputGain_n = 1 / gainReduction;
       }
        outputGain_n = std::min(outputGain_n, 1.f);

        // printf("qBar = %f outGain = %f\n", qBar, outputGain_n); fflush(stdout);
        // printf("qB-1.5=%f, x2 =%f p1=%f\n",
            
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

        
        freqForFilter = rack::dsp::FREQ_C4 * std::exp2(freqVolts + 30 - 4) / 1073741824;
        freqForFilter /= oversample;
        freqForFilter *= sampleTime;
      //  params4p.setFreq(freq);

        params4p.setR(r);
      //  fDbg = freq;
       // rDbg = r;
    }

    qForFilter = std::max(qForFilter, .07f);     // don't let Q get too high
    const float qComp = NonUniformLookupTable<float>::lookup(qCompParams, freqForFilter);
    // const float qComp = 1;

    params4p.setQ(qForFilter * qComp);
    params4p.setFreq(freqForFilter);

#if 0
    const float LRQ = qForFilter / .07;
    printf("f=%f, qbefore=%f comp=%f qafter=%f LRQ: %f\n", freqForFilter, qForFilter, qComp, qForFilter * qComp, LRQ);
#endif

}


template <class TBase>
inline void F4<TBase>::process(const typename TBase::ProcessArgs& args)
{
    divn.step();

    const float input =  F4<TBase>::inputs[AUDIO_INPUT].getVoltage(0);
    
    for (int i=0; i<oversample; ++i) {
        StateVariableFilter4P<T>::run(input, state4p, params4p);
    }


    {
    float output = state4p.lp;
    if (limiterEnabled_n) {
        output = limiter_lp.step(output)[0];
    } else {
        output *= outputGain_n;
    }
    output = std::min(10.f, output);
    output = std::max(-10.f, output);
    F4<TBase>::outputs[LP_OUTPUT].setVoltage(output, 0);
    }

    float output = state4p.hp;
    if (limiterEnabled_n) {
        output = limiter_hp.step(output)[0];
    } else {
        output *= outputGain_n;
    }
    output = std::min(10.f, output);
    output = std::max(-10.f, output);
    F4<TBase>::outputs[HP_OUTPUT].setVoltage(output, 0);

    output = state4p.bp;
     if (limiterEnabled_n) {
        output = limiter_bp.step(output)[0];
    } else {
        output *= outputGain_n;
    }
    output = std::min(10.f, output);
    output = std::max(-10.f, output);
    F4<TBase>::outputs[BP_OUTPUT].setVoltage(output, 0);

    output = state4p.peak;
     if (limiterEnabled_n) {
        output = limiter_pk.step(output)[0];
    } else {
        output *= outputGain_n;
    }
    output = std::min(10.f, output);
    output = std::max(-10.f, output);
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
            ret = {2.4f, 10, 3, "R"};
            break;
        case F4<TBase>::Q_PARAM:
            ret = {0, 10, 1.9f, "Q"};
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


