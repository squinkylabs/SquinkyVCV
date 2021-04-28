
#pragma once

#include <assert.h>

#include <algorithm>
#include <memory>

#include "ACDetector.h"
#include "AudioMath_4.h"
#include "Divider.h"
#include "IComposite.h"
#include "Limiter.h"
#include "ObjectCache.h"
#include "PeakDetector.h"
#include "SimdBlocks.h"
#include "SqLog.h"
#include "SqMath.h"
#include "SqPort.h"
#include "StateVariableFilter2.h"
#include "simd.h"

#ifndef _CLAMP
#define _CLAMP
namespace std {
inline float clamp(float v, float lo, float hi) {
    assert(lo < hi);
    return std::min(hi, std::max(v, lo));
}
}  // namespace std
#endif

namespace rack {
namespace engine {
struct Module;
}
}  // namespace rack
using Module = ::rack::engine::Module;

template <class TBase>
class F2_PolyDescription : public IComposite {
public:
    Config getParam(int i) override;
    int getNumParams() override;
};

/**
 * with optimizations and Vu
 *  a) mono 12db + lim       9.2  
 *  b) 12db no lim           9.17    
 *  c) 16 chan  24db + lim   60.75
 *  d) 24 lim mod-> q,r,fc   27.6
 *  e) " " 4ch     " "       28.6  
 *  f) 24 lim 4ch no mod     20.4  
 *  g) 24 lim no mo          20      
 *  
 * 2021 apr, fixed test
 *                           orig      round2   zero CV update
 *  a) mono 12db + lim        9.12   -> 9.06    7.6
 *  b) 12db no lim            9.2   -> 8.9      7.7
 *  c) 16 chan  24db + lim    58.5  -> 58.07    55
 *  d) 24 lim mod-> q,r,fc    63.0  -> 26.9     17
 *  e) " " 4ch     " "        64.1  -> 27.9     17
 *  f) 24 lim 4ch no mod      24.5  -> 19.7     16.4
 *  g) 24 lim no mo           19.7  -> 19.5     16.4
 * 
 * hal
 * 
 * 2021 - re-boot
 * mono 12 + lim:   9.02
 * 12 no lim        8.9  
 * 24+lim:          18.19
 * 16ch             22.32    
 * 
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
class F2_Poly : public TBase {
public:
    using T = float_4;

    F2_Poly(Module* module) : TBase(module) {
        init();
    }

    F2_Poly() : TBase() {
        init();
    }

    void init();

    enum ParamIds {
        TOPOLOGY_PARAM,
        FC_PARAM,
        R_PARAM,
        Q_PARAM,
        MODE_PARAM,
        LIMITER_PARAM,
        FC_TRIM_PARAM,
        UNUSED_CV_UPDATE_FREQ,
        VOL_PARAM,
        SCHEMA_PARAM,
        Q_TRIM_PARAM,
        R_TRIM_PARAM,
        ALT_LIMITER_PARAM,
        NUM_PARAMS
    };

    enum InputIds {
        AUDIO_INPUT,
        FC_INPUT,
        Q_INPUT,
        R_INPUT,
        NUM_INPUTS
    };

    enum OutputIds {
        AUDIO_OUTPUT,
        NUM_OUTPUTS
    };

    enum LightIds {
        VOL0_LIGHT,
        VOL1_LIGHT,
        VOL2_LIGHT,
        VOL3_LIGHT,
        LIMITER_LIGHT,
        NUM_LIGHTS
    };

    enum class Topology {
        SINGLE,
        SERIES,
        PARALLEL,
        PARALLEL_INV
    };

    /** Implement IComposite
     */
    static std::shared_ptr<IComposite> getDescription() {
        return std::make_shared<F2_PolyDescription<TBase>>();
    }

    /**
     * Main processing entry point. Called every sample
     */
    void process(const typename TBase::ProcessArgs& args) override;

    void onSampleRateChange() override;

    const StateVariableFilterParams2<T>& _params1(int bank) const;
    const StateVariableFilterParams2<T>& _params2(int bank) const;

    static float_4 computeGain_fast(bool twoStages, float_4 q4, float_4 r4);
    static float_4 computeGain_slow(bool twoStages, float_4 q4, float_4 r4);
    static float_4 processR_slow(float_4 rawR);
    static float_4 processR_fast(float_4 rawR);

private:
    StateVariableFilterParams2<T> params1[4];
    StateVariableFilterParams2<T> params2[4];
    StateVariableFilterState2<T> state1[4];
    StateVariableFilterState2<T> state2[4];
    Limiter limiter;
    StateVariableFilter2<T>::processFunction filterFunc = nullptr;
    std::shared_ptr<LookupTableParams<float>> audioTaperLookupParams = ObjectCache<float>::getAudioTaper();
    const int oversample = 4;

    float_4 outputGain_n = 0;
    bool limiterEnabled_m = 0;
    int numChannels_m = 0;
    int lastNumChannels_m = 0;
    int numBanks_m = 0;
    Topology topology_m = Topology::SINGLE;
    float_4 lastQCV[4] = {-1};
    float_4 lastRCV[4] = {-1};
    float_4 lastFcCV[4] = {-1};
    float lastFcKnob = -1;
    float lastFcTrim = -1;
    float lastRKnob = -1;
    float lastRTrim = -1;
    float lastQKnob = -1;
    float lastQTrim = -1;
    float lastVolume = -1;
    int lastTopology = -1;
    int lastAltLimiter = -1;

    void resetParamCache() {
        lastQCV[4] = {-1};
        lastRCV[4] = {-1};
        lastFcCV[4] = {-1};
        lastFcKnob = -1;
        lastFcTrim = -1;
        lastRKnob = -1;
        lastRTrim = -1;
        lastQKnob = -1;
        lastQTrim = -1;
        lastVolume = -1;
        lastTopology = -1;
        lastAltLimiter = -1;
    }

    float_4 processedRValue[4] = {-1};
    float_4 volume = {-1};

    // Divider divn;
    // default to 1/4 SR cv update, go on next one
    int stepNcounter = 3;
    int stepNmax = 3;

    Divider divm;
    PeakDetector4 peak;
    void stepn();
    void stepm();
    void setupFreq();
    void setupVolume();
    void setupModes();
    void setupProcFunc();
    void setupLimiter();
    void updateVUOutput();

    static float_4 fastQFunc(float_4 qV, int numStages);
    static std::pair<float_4, float_4> fastFcFunc2(float_4 freqVolts, float_4 rVolts, float oversample, float sampleTime, bool twoPeaks);

    using processFunction = void (F2_Poly<TBase>::*)(const typename TBase::ProcessArgs& args);
    processFunction procFun;

    void processOneBankSeries(const typename TBase::ProcessArgs& args);
    void processOneBank12_lim(const typename TBase::ProcessArgs& args);
    // void processOneBank24_lim(const typename TBase::ProcessArgs& args);
    void processOneBank12_nolim(const typename TBase::ProcessArgs& args);
    void processGeneric(const typename TBase::ProcessArgs& args);

    AudioMath_4::ScaleFun scaleFc = AudioMath_4::makeScalerWithBipolarAudioTrim(0, 10, 0, 10);
};

template <class TBase>
inline void F2_Poly<TBase>::init() {
    //  divn.setup(4, [this]() {
    //      this->stepn();
    //  });
    divm.setup(16, [this]() {
        this->stepm();
    });
    setupLimiter();
    peak.setDecay(2);
}

template <class TBase>
inline void F2_Poly<TBase>::stepm() {
    SqInput& inPort = TBase::inputs[AUDIO_INPUT];
    SqOutput& outPort = TBase::outputs[AUDIO_OUTPUT];

    numChannels_m = inPort.channels;
    outPort.setChannels(numChannels_m);

    numBanks_m = (numChannels_m / 4) + ((numChannels_m % 4) ? 1 : 0);

  
    if (lastNumChannels_m != numChannels_m) {
        lastNumChannels_m = numChannels_m;
        resetParamCache();
    }

    setupModes();
    setupProcFunc();
    limiterEnabled_m = bool(std::round(F2_Poly<TBase>::params[LIMITER_PARAM].value));

    bool altLim = bool(std::round(F2_Poly<TBase>::params[ALT_LIMITER_PARAM].value));
    if (altLim != (lastAltLimiter == 1)) {
        lastAltLimiter = altLim;
        setupLimiter();
    }

    stepNmax = 3;

    TBase::lights[LIMITER_LIGHT].value = (limiterEnabled_m) ? 1.f : .2f;
}

template <class TBase>
inline void F2_Poly<TBase>::setupLimiter() {
    const bool alt = (lastAltLimiter == 1);
    limiter.setTimes(
        alt ? 3.f : 1.f,
        alt ? 20.f : 100.f,
        TBase::engineGetSampleTime());
    limiter.setInputGain(alt ? 20.f : 4.f);
    // SQINFO("setupLim, alt = %d", alt);
}

template <class TBase>
inline void F2_Poly<TBase>::onSampleRateChange() {
    setupLimiter();
}

template <class TBase>
inline const StateVariableFilterParams2<float_4>& F2_Poly<TBase>::_params1(int bank) const {
    return params1[bank];
}

template <class TBase>
inline const StateVariableFilterParams2<float_4>& F2_Poly<TBase>::_params2(int bank) const {
    return params2[bank];
}

template <class TBase>
inline float_4 F2_Poly<TBase>::fastQFunc(float_4 qV, int numStages) {
    assert(numStages >= 1 && numStages <= 2);

    const float expMult = (numStages == 1) ? 1 / 1.5f : 1 / 2.5f;
    float_4 q = rack::dsp::approxExp2_taylor5(qV * expMult) - .5;
    return q;
}

template <class TBase>
inline std::pair<float_4, float_4> F2_Poly<TBase>::fastFcFunc2(float_4 freqVolts, float_4 r, float oversample, float sampleTime, bool twoPeaks) {
    assert(oversample == 4);
    assert(sampleTime < .0001);
    float_4 freq = rack::dsp::FREQ_C4 * rack::dsp::approxExp2_taylor5(freqVolts + 30 - 4) / 1073741824;

    freq /= oversample;
    freq *= sampleTime;

    // we could pass in 1/r, too
    float_4 f1 = twoPeaks ? freq / r : freq;
    float_4 f2 = twoPeaks ? freq * r : freq;
    return std::make_pair(f1, f2);
}

template <class TBase>
inline void F2_Poly<TBase>::setupVolume() {
    const float v = F2_Poly<TBase>::params[VOL_PARAM].value;
    if (lastVolume == v) {
        return;
    }

    lastVolume = v;
    const float procVolume = 4 * std::sqrt(2.f) * LookupTable<float>::lookup(*audioTaperLookupParams, v / 100);
    volume = float_4(procVolume);
}

template <class TBase>
inline float_4 F2_Poly<TBase>::computeGain_fast(bool twoStages, float_4 q4, float_4 r4) {
    const float_4 oneOverQ = 1.0f / q4;
    if (!twoStages) {
        return sqrt(oneOverQ);
    }
    const float_4 oneOverQSq = 1.0f / (q4 * q4);

    const float_4 interp = r4 * float_4(.5f);
    const float_4 g_interp = interp * oneOverQ + (float_4(1.f) - interp) * oneOverQSq;

    float_4 g = SimdBlocks::ifelse((r4 > float_4(2.f)), oneOverQ, g_interp);
    g = sqrt(g);
    //SQINFO("g = %f", g[0]);
    return g;
}

template <class TBase>
inline float_4 F2_Poly<TBase>::computeGain_slow(bool twoStages, float_4 q4, float_4 r4) {
    float_4 outputGain4 = 0;
    // let's try "half bass suck"
    // TODO: make faster
    {
        for (int i = 0; i < 4; ++i) {
            float g_ = 0;
            float q_ = q4[i];
            float r_ = r4[i];

            float oneOverQ = 1.f / q_;
            float oneOverQSq = 1.f / (q_ * q_);
            if (!twoStages) {
                g_ = oneOverQ;
            } else if (r_ >= 2.f) {
                g_ = oneOverQ;
            } else {
                float interp = r_ / 2.f;  // 0..1
                g_ = interp * oneOverQ + (1.f - interp) * oneOverQSq;
            }

            auto dbGain = AudioMath::db(g_);
            dbGain *= .5f;
            g_ = float(AudioMath::gainFromDb(dbGain));
            outputGain4[i] = g_;
        }
    }
    //SQINFO("cg(%f, %f) = %f", q4[0], r4[0]);
    return outputGain4;
}

template <class TBase>
inline float_4 F2_Poly<TBase>::processR_fast(float_4 r) {
    r = SimdBlocks::ifelse((r > 3), r - 1.5, r * .5f);
    r = rack::dsp::approxExp2_taylor5(r / 3.f);
    return r;
}

template <class TBase>
inline float_4 F2_Poly<TBase>::processR_slow(float_4 rawR) {
    for (int i = 0; i < 4; ++i) {
        float r = rawR[i];

        if (r > 3) {
            r -= 1.5;
        } else {
            r /= 2;  // make less sensitive for low value
        }
        rawR[i] = r;
    }
    return rack::dsp::approxExp2_taylor5(rawR / 3.f);
}

template <class TBase>
inline void F2_Poly<TBase>::setupFreq() {
    const float sampleTime = TBase::engineGetSampleTime();
    const int topologyInt = int(std::round(F2_Poly<TBase>::params[TOPOLOGY_PARAM].value));
    const int numStages = (topologyInt == 0) ? 1 : 2;
    bool topologyChanged = false;
    if (topologyInt != lastTopology) {
        lastTopology = topologyInt;
        topologyChanged = true;
    }

    const bool fcKnobChanged = lastFcKnob != F2_Poly<TBase>::params[FC_PARAM].value;
    const bool fcTrimChanged = lastFcTrim != F2_Poly<TBase>::params[FC_TRIM_PARAM].value;

    const bool rKnobChanged = lastRKnob != F2_Poly<TBase>::params[R_PARAM].value;
    const bool rTrimChanged = lastRTrim != F2_Poly<TBase>::params[R_TRIM_PARAM].value;

    const bool qKnobChanged = lastQKnob != F2_Poly<TBase>::params[Q_PARAM].value;
    const bool qTrimChanged = lastQTrim != F2_Poly<TBase>::params[Q_TRIM_PARAM].value;

    lastFcKnob = F2_Poly<TBase>::params[FC_PARAM].value;
    lastFcTrim = F2_Poly<TBase>::params[FC_TRIM_PARAM].value;
    lastQKnob = F2_Poly<TBase>::params[Q_PARAM].value;
    lastQTrim = F2_Poly<TBase>::params[Q_TRIM_PARAM].value;
    lastRKnob = F2_Poly<TBase>::params[R_PARAM].value;
    lastRTrim = F2_Poly<TBase>::params[R_TRIM_PARAM].value;

    for (int bank = 0; bank < numBanks_m; bank++) {
        const int baseChannel = 4 * bank;

        // First, let's round up all the CV for this bank
        SqInput& fCport = TBase::inputs[FC_INPUT];
        float_4 fcCV = fCport.getPolyVoltageSimd<float_4>(baseChannel);
        const bool fcCVChanged = rack::simd::movemask(fcCV != lastFcCV[bank]);

        SqInput& qport = TBase::inputs[Q_INPUT];
        float_4 qCV = qport.getPolyVoltageSimd<float_4>(baseChannel);

        const bool qCVChanged = rack::simd::movemask(qCV != lastQCV[bank]);

        SqInput& rport = TBase::inputs[R_INPUT];
        float_4 rCV = rport.getPolyVoltageSimd<float_4>(baseChannel);
        const bool rCVChanged = rack::simd::movemask(rCV != lastRCV[bank]);

        // if R changed, do calcs that depend only on R
        if (rCVChanged || rTrimChanged || rKnobChanged) {
            lastRCV[bank] = rCV;
            rCV = rack::simd::clamp(rCV, 0, 10);
            float_4 combinedRVolage = scaleFc(
                rCV,
                lastRKnob,
                lastRTrim);
            processedRValue[bank] = processR_fast(combinedRVolage);
        }

        // compute Q. depends on R
        if (qCVChanged || qKnobChanged || qTrimChanged ||
            topologyChanged ||
            rCVChanged || rTrimChanged || rKnobChanged) {
            lastQCV[bank] = qCV;
            qCV = rack::simd::clamp(qCV, 0, 10);
            float_4 combinedQ = scaleFc(
                qCV,
                lastQKnob,
                lastQTrim);
            float_4 q = fastQFunc(combinedQ, numStages);
            params1[bank].setQ(q);
            params2[bank].setQ(q);

            // is it just rCV, or should it be combined? I think combined
            outputGain_n = computeGain_fast(numStages == 2, q, processedRValue[bank]);
        }

        if (fcCVChanged || fcKnobChanged || fcTrimChanged ||
            rCVChanged || rKnobChanged || rTrimChanged ||
            topologyChanged) {
            // SQINFO("changed: %d, %d, %d, %d", fcCVChanged, rChanged, fcKnobChanged, fcTrimChanged);
            lastFcCV[bank] = fcCV;
            float_4 combinedFcVoltage = scaleFc(
                fcCV,
                lastFcKnob,
                lastFcTrim);

            auto fr = fastFcFunc2(combinedFcVoltage, processedRValue[bank], float(oversample), sampleTime, numStages == 2);

            params1[bank].setFreq(fr.first);
            params2[bank].setFreq(fr.second);
        }
    }
}

template <class TBase>
inline void F2_Poly<TBase>::setupModes() {
    const int modeParam = int(std::round(F2_Poly<TBase>::params[MODE_PARAM].value));
    auto mode = StateVariableFilter2<T>::Mode(modeParam);
    filterFunc = StateVariableFilter2<T>::getProcPointer(mode, oversample);

    const int topologyInt = int(std::round(F2_Poly<TBase>::params[TOPOLOGY_PARAM].value));
    topology_m = Topology(topologyInt);
}

template <class TBase>
inline void F2_Poly<TBase>::setupProcFunc() {
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
inline void F2_Poly<TBase>::stepn() {
    ++stepNcounter;
    if (stepNcounter <= stepNmax) {
        return;
    }
    stepNcounter = 0;

    setupFreq();
    setupVolume();
    updateVUOutput();
}

template <class TBase>
inline void F2_Poly<TBase>::process(const typename TBase::ProcessArgs& args) {
    divm.step();

    // always do the CV calc, even though it might be divided.
    stepn();
    assert(oversample == 4);
    assert(procFun);
    (this->*procFun)(args);
}

template <class TBase>
inline void F2_Poly<TBase>::updateVUOutput() {
    const int divFactor = stepNmax + 1;
    peak.decay(divFactor * TBase::engineGetSampleTime() * 5);
    const float level = peak.get();
    // TODO: these values are not the right ones for us
    const float lMax = 10;
    const float dim = .2f;
    const float bright = .8f;
    TBase::lights[VOL3_LIGHT].value = (level >= lMax) ? bright : dim;
    TBase::lights[VOL2_LIGHT].value = (level >= ((lMax - 2) * .5)) ? bright : dim;
    TBase::lights[VOL1_LIGHT].value = (level >= (lMax * .25f)) ? bright : dim;
    TBase::lights[VOL0_LIGHT].value = (level >= (lMax * .125f)) ? bright : dim;
    // SQINFO("level = %.2f", level);
}

#define ENDPROC(chan)                                \
    output *= volume;                                \
    output = rack::simd::clamp(output, -20.f, 20.f); \
    peak.step(output);                               \
    outPort.setVoltageSimd(output, chan);

template <class TBase>
inline void F2_Poly<TBase>::processOneBankSeries(const typename TBase::ProcessArgs& args) {
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
    ENDPROC(0);
}

template <class TBase>
inline void F2_Poly<TBase>::processOneBank12_lim(const typename TBase::ProcessArgs& args) {
    SqInput& inPort = TBase::inputs[AUDIO_INPUT];
    const float_4 input = inPort.getPolyVoltageSimd<float_4>(0);

    T output = (*filterFunc)(input, state1[0], params1[0]);
    output = limiter.step(output);

    SqOutput& outPort = TBase::outputs[AUDIO_OUTPUT];
    ENDPROC(0);
}

template <class TBase>
inline void F2_Poly<TBase>::processOneBank12_nolim(const typename TBase::ProcessArgs& args) {
    SqInput& inPort = TBase::inputs[AUDIO_INPUT];
    const float_4 input = inPort.getPolyVoltageSimd<float_4>(0);

    T output = (*filterFunc)(input, state1[0], params1[0]);
    output *= outputGain_n;

    SqOutput& outPort = TBase::outputs[AUDIO_OUTPUT];
    ENDPROC(0);
}

template <class TBase>
inline void F2_Poly<TBase>::processGeneric(const typename TBase::ProcessArgs& args) {
    SqInput& inPort = TBase::inputs[AUDIO_INPUT];
    for (int bank = 0; bank < numBanks_m; bank++) {
        const int baseChannel = 4 * bank;
        const float_4 input = inPort.getPolyVoltageSimd<float_4>(baseChannel);
        T output;
        switch (topology_m) {
            case Topology::SERIES: {
                const T temp = (*filterFunc)(input, state1[bank], params1[bank]);
                output = (*filterFunc)(temp, state2[bank], params2[bank]);
            } break;
            case Topology::PARALLEL: {
                // parallel add
                output = (*filterFunc)(input, state1[bank], params1[bank]);
                output += (*filterFunc)(input, state2[bank], params2[bank]);
            } break;
            case Topology::PARALLEL_INV: {
                // parallel sub
                output = (*filterFunc)(input, state1[bank], params1[bank]);
                output -= (*filterFunc)(input, state2[bank], params2[bank]);
            } break;
            case Topology::SINGLE: {
                // one filter 4X
                output = (*filterFunc)(input, state1[bank], params1[bank]);
            } break;
            default:
                assert(false);
        }

        if (limiterEnabled_m) {
            output = limiter.step(output);
        } else {
            output *= outputGain_n;
        }

        SqOutput& outPort = TBase::outputs[AUDIO_OUTPUT];
        ENDPROC(baseChannel);
    }
}

template <class TBase>
int F2_PolyDescription<TBase>::getNumParams() {
    return F2_Poly<TBase>::NUM_PARAMS;
}

template <class TBase>
inline IComposite::Config F2_PolyDescription<TBase>::getParam(int i) {
    Config ret(0, 1, 0, "");
    switch (i) {
        case F2_Poly<TBase>::TOPOLOGY_PARAM:
            ret = {0, 3, 0, "Topology"};
            break;
        case F2_Poly<TBase>::MODE_PARAM:
            ret = {0, 3, 0, "Mode"};
            break;
        case F2_Poly<TBase>::FC_PARAM:
            ret = {0, 10, 5, "Cutoff freq (Fc)"};
            break;
        case F2_Poly<TBase>::R_PARAM:
            ret = {0, 10, 0, "Spread (R)"};
            break;
        case F2_Poly<TBase>::Q_PARAM:
            ret = {0, 10, 2, "Resonance (Q)"};
            break;
        case F2_Poly<TBase>::LIMITER_PARAM:
            ret = {0, 1, 1, "Limiter"};
            break;
        case F2_Poly<TBase>::FC_TRIM_PARAM:
            ret = {-1, 1, 0, "Fc modulation trim"};
            break;
        case F2_Poly<TBase>::Q_TRIM_PARAM:
            ret = {-1, 1, 0, "Q modulation trim"};
            break;
        case F2_Poly<TBase>::R_TRIM_PARAM:
            ret = {-1, 1, 0, "R modulation trim"};
            break;
        case F2_Poly<TBase>::UNUSED_CV_UPDATE_FREQ:
            ret = {0, 1, 0, "CV update fidelity"};
            break;
        case F2_Poly<TBase>::VOL_PARAM:
            ret = {0, 100, 50, "Output volume"};
            break;
        case F2_Poly<TBase>::SCHEMA_PARAM:
            ret = {0, 10, 0, "schema"};
            break;
        case F2_Poly<TBase>::ALT_LIMITER_PARAM:
            ret = {0, 1, 1, "alt limit"};
            break;
        default:
            assert(false);
    }
    return ret;
}
