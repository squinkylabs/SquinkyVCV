
#pragma once

#include <assert.h>

#include <memory>

#include "Cmprsr.h"
#include "CompressorParamHolder.h"
#include "Divider.h"
#include "IComposite.h"
#include "LookupTableFactory.h"
#include "ObjectCache.h"
#include "SqLog.h"
#include "SqPort.h"

namespace rack {
namespace engine {
struct Module;
}
}  // namespace rack

using Module = ::rack::engine::Module;

template <class TBase>
class Compressor2Description : public IComposite {
public:
    Config getParam(int i) override;
    int getNumParams() override;
};

/**
 */
template <class TBase>
class Compressor2 : public TBase {
public:
    Compressor2(Module* module) : TBase(module) {
    }
    Compressor2() : TBase() {
    }

    /**
    * re-calc everything that changes with sample
    * rate. Also everything that depends on baseFrequency.
    *
    * Only needs to be called once.
    */
    void init();

    enum ParamIds {
        ATTACK_PARAM,
        RELEASE_PARAM,
        THRESHOLD_PARAM,
        RATIO_PARAM,
        MAKEUPGAIN_PARAM,
        NOTBYPASS_PARAM,
        WETDRY_PARAM,
        CHANNEL_PARAM,
        NUM_PARAMS
    };

    enum InputIds {
        LAUDIO_INPUT,
        // RAUDIO_INPUT,
        NUM_INPUTS
    };

    enum OutputIds {
        LAUDIO_OUTPUT,
        //  RAUDIO_OUTPUT,
        DEBUG_OUTPUT,
        NUM_OUTPUTS
    };

    enum LightIds {
        NUM_LIGHTS
    };

    /** Implement IComposite
     */
    static std::shared_ptr<IComposite> getDescription() {
        return std::make_shared<Compressor2Description<TBase>>();
    }

    /**
     * Main processing entry point. Called every sample
     */
    //void step() override;
    void process(const typename TBase::ProcessArgs& args) override;

    void onSampleRateChange() override;

    float getGainReductionDb() const;
    static const std::vector<std::string>& ratios();
    static const std::vector<std::string>& ratiosLong();
    static std::function<double(double)> getSlowAttackFunction() {
        return AudioMath::makeFunc_Exp(0, 1, .05, 30);
    }

    static std::function<double(double)> getSlowReleaseFunction() {
        return AudioMath::makeFunc_Exp(0, 1, 100, 1600);
    }
    static std::function<double(double)> getSlowThresholdFunction() {
        return AudioMath::makeFunc_Exp(0, 10, .1, 10);
    }

    int getNumChannels() const {
        return numChannels_m;
    }
    float getChannelGain(int ch) const;

    Cmprsr& _getComp(int bank);
    const CompressorParmHolder& _getHolder() { return compParams; }
    float_4 _getWet(int bank) const { return wetLevel[bank]; }
    float_4 _getEn(int bank) const { return enabled[bank]; }
    float_4 _getG(int bank) const { return makeupGain[bank]; }

private:
    CompressorParmHolder compParams;

    Cmprsr compressors[4];
    void setupLimiter();
    void initAllParams();
    void stepn();
    void pollAttackRelease();
    void updateAttackAndRelease(int bank);
    void pollThresholdAndRatio();
    void updateThresholdAndRatio(int bank);
    void pollWetDry();
    void updateWetDry(int bank);
    void pollBypassed();
    void updateBypassed(int bank);
    void pollMakeupGain();
    void updateMakeupGain(int bank);

    int numChannels_m = 0;
    int numBanks_m = 0;
    int currentBank_m = -1;
    int currentSubChannel_m = -1;
    unsigned int currentChannel_m = -1;  // which of the 16 channels we are editing ATM.

    float_4 wetLevel[4] = {0};
    float_4 dryLevel[4] = {0};
    float_4 enabled[4] = {0};
    float_4 makeupGain[4] = { 1 };

   // float_4 makeupGain_m = 1;
    Divider divn;

    // we could unify this stuff with the ui stuff, above.
    LookupTableParams<float> attackFunctionParams;
    LookupTableParams<float> releaseFunctionParams;
    LookupTableParams<float> thresholdFunctionParams;

    std::shared_ptr<LookupTableParams<float>> panL = ObjectCache<float>::getMixerPanL();
    std::shared_ptr<LookupTableParams<float>> panR = ObjectCache<float>::getMixerPanR();

    float lastRawMakeupGain = -1;
    float lastRawMix = -1;
    float lastRawA = -1;
    float lastRawR = -1;
    float lastRawThreshold = -1;
    float lastRawRatio = -1;
    int lastNumChannels = -1;
    bool lastNotBypassed = false;
};

template <class TBase>
inline const std::vector<std::string>& Compressor2<TBase>::ratios() {
    return Cmprsr::ratios();
}

template <class TBase>
inline const std::vector<std::string>& Compressor2<TBase>::ratiosLong() {
    return Cmprsr::ratiosLong();
}

template <class TBase>
inline void Compressor2<TBase>::init() {
    setupLimiter();
    divn.setup(32, [this]() {
        this->stepn();
    });

    LookupTableFactory<float>::makeGenericExpTaper(64, attackFunctionParams, 0, 1, .05, 30);
    LookupTableFactory<float>::makeGenericExpTaper(64, releaseFunctionParams, 0, 1, 100, 1600);
    LookupTableFactory<float>::makeGenericExpTaper(64, thresholdFunctionParams, 0, 10, .1, 10);
    initAllParams();
}

/**
 * This function should set all members of param holders to defaults,
 * and then refresh all the compressors with those values
 */
template <class TBase>
inline void Compressor2<TBase>::initAllParams() {
    auto icomp = Compressor2<TBase>::getDescription();
    for (int i = 0; i < 16; ++i) {
        compParams.setAttack(i, icomp->getParam(ATTACK_PARAM).def);
        compParams.setRelease(i, icomp->getParam(RELEASE_PARAM).def);
        compParams.setRatio(i, int(std::round(icomp->getParam(RATIO_PARAM).def)));
        compParams.setThreshold(i, icomp->getParam(THRESHOLD_PARAM).def);
        compParams.setMakeupGain(i, icomp->getParam(MAKEUPGAIN_PARAM).def);
        compParams.setEnabled(i, bool(std::round(icomp->getParam(NOTBYPASS_PARAM).def)));
        compParams.setWetDry(i, icomp->getParam(WETDRY_PARAM).def);
        compParams.setEnabled(i, icomp->getParam(NOTBYPASS_PARAM).def);
        compParams.setMakeupGain(i, icomp->getParam(MAKEUPGAIN_PARAM).def);
    }

    for (int bank = 0; bank < 4; ++bank) {
        updateAttackAndRelease(bank);
        updateThresholdAndRatio(bank);
        updateWetDry(bank);
        updateMakeupGain(bank);
        updateBypassed(bank);
        updateMakeupGain(bank);

        // TODO: put all the update here
    }
}

template <class TBase>
inline Cmprsr& Compressor2<TBase>::_getComp(int bank) {
    return compressors[bank];
}

/*
 * call chain:
 *      Compressor2
 *      Cmprsr
 *      Cmprsr::gain
 */
template <class TBase>
inline float Compressor2<TBase>::getChannelGain(int ch) const {
    const int bank = ch / 4;
    const int subChan = ch - bank * 4;
    // TODO:db
    float_4 g = compressors[bank].getGain();
    float gainReduction = g[subChan];
    return gainReduction;
}

template <class TBase>
inline float Compressor2<TBase>::getGainReductionDb() const {
    float_4 minGain_4 = 1;
    if (bypassed) {
        return 0;
    }

    for (int bank = 0; bank < numBanks_m; ++bank) {
        minGain_4 = SimdBlocks::min(minGain_4, compressors[bank].getGain());
    }

    float minGain = minGain_4[0];
    minGain = std::min(minGain, minGain_4[1]);
    minGain = std::min(minGain, minGain_4[2]);
    minGain = std::min(minGain, minGain_4[3]);
    auto r = AudioMath::db(minGain);
    return -r;
}

template <class TBase>
inline void Compressor2<TBase>::stepn() {
    SqInput& inPort = TBase::inputs[LAUDIO_INPUT];
    SqOutput& outPort = TBase::outputs[LAUDIO_OUTPUT];

    numChannels_m = inPort.channels;
    outPort.setChannels(numChannels_m);

    numBanks_m = (numChannels_m / 4) + ((numChannels_m % 4) ? 1 : 0);

    currentChannel_m = -1 + int(std::round(TBase::params[CHANNEL_PARAM].value));
    assert(currentChannel_m >= 0);
    assert(currentChannel_m <= 15);
    currentBank_m = currentChannel_m / 4;
    currentSubChannel_m = currentChannel_m % 4;

    pollAttackRelease();
    pollWetDry();
    pollMakeupGain();
    pollThresholdAndRatio();
    pollBypassed();
}

template <class TBase>
inline void Compressor2<TBase>::pollMakeupGain() {
    const float g = Compressor2<TBase>::params[MAKEUPGAIN_PARAM].value;
    if (g != lastRawMakeupGain) {
        lastRawMakeupGain = g;
        compParams.setMakeupGain(currentChannel_m, g);
        updateMakeupGain(currentBank_m);
    }
}

template <class TBase>
inline void Compressor2<TBase>::updateMakeupGain(int bank) {
    const float_4 rawMakeupGain = compParams.getMakeupGains(bank);
    float_4 g;
    for (int i = 0; i < 4; ++i) {
        g[i] = float(AudioMath::gainFromDb(rawMakeupGain[i]));
    }
    makeupGain[bank] = g;
}

template <class TBase>
inline void Compressor2<TBase>::pollBypassed() {
    const bool notByp = bool(std::round(Compressor2<TBase>::params[NOTBYPASS_PARAM].value));
    if (notByp != lastNotBypassed) {
        lastNotBypassed = notByp;
        compParams.setEnabled(currentChannel_m, notByp);
        updateBypassed(currentBank_m);
    }
}

template <class TBase>
inline void Compressor2<TBase>::updateBypassed(int bank) {
    enabled[bank] = compParams.getEnableds(bank);
}

template <class TBase>
inline void Compressor2<TBase>::updateAttackAndRelease(int bank) {
    float_4 a, r;

    float_4 rawA_4 = compParams.getAttacks(bank);
    float_4 rawR_4 = compParams.getReleases(bank);
    for (int i = 0; i < 4; ++i) {
        const float rawAttack = rawA_4[i];
        const float rawRelease = rawR_4[i];
        const float attack = LookupTable<float>::lookup(attackFunctionParams, rawAttack);
        const float release = LookupTable<float>::lookup(releaseFunctionParams, rawRelease);
        a[i] = attack;
        r[i] = release;
    }

    compressors[bank].setTimesPoly(
        a,
        r,
        TBase::engineGetSampleTime());
}

template <class TBase>
inline void Compressor2<TBase>::pollAttackRelease() {
    const float rawAttack = Compressor2<TBase>::params[ATTACK_PARAM].value;
    const float rawRelease = Compressor2<TBase>::params[RELEASE_PARAM].value;
    const bool reduceDistortion = true;

    if (rawAttack != lastRawA || rawRelease != lastRawR) {
        lastRawA = rawAttack;
        lastRawR = rawRelease;

        compParams.setAttack(currentChannel_m, rawAttack);
        compParams.setRelease(currentChannel_m, rawRelease);
        updateAttackAndRelease(currentBank_m);
    }
}

template <class TBase>
inline void Compressor2<TBase>::pollThresholdAndRatio() {
    const float rawThreshold = Compressor2<TBase>::params[THRESHOLD_PARAM].value;
    const float rawRatio = Compressor2<TBase>::params[RATIO_PARAM].value;
    if (rawThreshold != lastRawThreshold || rawRatio != lastRawRatio) {
        lastRawThreshold = rawThreshold;
        lastRawRatio = rawRatio;
        compParams.setThreshold(currentChannel_m, rawThreshold);
        compParams.setRatio(currentChannel_m, int(std::round(rawRatio)));
        updateThresholdAndRatio(currentBank_m);
    }
}

template <class TBase>
inline void Compressor2<TBase>::pollWetDry() {
    const float rawWetDry = Compressor2<TBase>::params[WETDRY_PARAM].value;
    if (rawWetDry != lastRawMix) {
        lastRawMix = rawWetDry;
        compParams.setWetDry(currentChannel_m, rawWetDry);
        updateWetDry(currentBank_m);
    }
}

template <class TBase>
inline void Compressor2<TBase>::updateThresholdAndRatio(int bank) {
    auto rawRatios_4 = compParams.getRatios(bank);
    float_4 rawThresholds_4 = compParams.getThresholds(bank);
    float_4 th;
    Cmprsr::Ratios r[4];
  
    for (int i=0; i< 4; ++i) {
         const float threshold = LookupTable<float>::lookup(thresholdFunctionParams, rawThresholds_4[i]);
         th[i] = threshold;
         r[i] =  Cmprsr::Ratios(rawRatios_4[i]);
    }

    compressors[bank].setThresholdPoly(th);
    compressors[bank].setCurvePoly(r);
}

template <class TBase>
inline void Compressor2<TBase>::updateWetDry(int bank) {
    float_4 rawWetDry = compParams.getWetDryMixs(bank);
    float_4 w = 0;
    float_4 d = 0;
    for (int i = 0; i < 4; ++i) {
        w[i] = LookupTable<float>::lookup(*panR, rawWetDry[i], true);
        d[i] = LookupTable<float>::lookup(*panL, rawWetDry[i], true);
        w[i] *= w[i];
        d[i] *= d[i];
    }
    wetLevel[bank] = w;
    dryLevel[bank] = d;
}

template <class TBase>
inline void Compressor2<TBase>::process(const typename TBase::ProcessArgs& args) {
    divn.step();

    SqInput& inPort = TBase::inputs[LAUDIO_INPUT];
    SqOutput& outPort = TBase::outputs[LAUDIO_OUTPUT];


    // TODO: bypassed per channel - need to completely re-do
    for (int bank = 0; bank < numBanks_m; ++bank) {
        const int baseChannel = bank * 4;
        const float_4 en = compParams.getEnableds(bank);
        simd_assertMask(en);
        const float_4 input = inPort.getPolyVoltageSimd<float_4>(baseChannel);
        const float_4 wetOutput = compressors[bank].step(input) * makeupGain[bank];
        const float_4 mixedOutput = wetOutput * wetLevel[bank] + input * dryLevel[bank];
       
        const float_4 out = SimdBlocks::ifelse(en, mixedOutput, input);
      //  SQINFO("\nbank=%d input=%s wet=%s", bank, toStr(input).c_str(), toStr(wetOutput).c_str());
      //  SQINFO("en=%s, true=%s", toStr(en).c_str(), toStr(SimdBlocks::maskTrue()).c_str());
      //  SQINFO("output=%s", toStr(out).c_str());
        outPort.setVoltageSimd(out, baseChannel);
    }
}

// TODO: do we still need this old init function? combine with other?
template <class TBase>
inline void Compressor2<TBase>::setupLimiter() {
    for (int i = 0; i < 4; ++i) {
        compressors[i].setTimes(1, 100, TBase::engineGetSampleTime(), false);
        //   compressorsR[i].setTimes(1, 100, TBase::engineGetSampleTime(), false);
    }
}

template <class TBase>
inline void Compressor2<TBase>::onSampleRateChange() {
    setupLimiter();
}

template <class TBase>
int Compressor2Description<TBase>::getNumParams() {
    return Compressor2<TBase>::NUM_PARAMS;
}

template <class TBase>
inline IComposite::Config Compressor2Description<TBase>::getParam(int i) {
    Config ret(0, 1, 0, "");
    switch (i) {
        case Compressor2<TBase>::ATTACK_PARAM:
            // .8073 too low .8075 too much
            ret = {0, 1, .8074f, "Attack time"};
            break;
        case Compressor2<TBase>::RELEASE_PARAM:
            ret = {0, 1, .25f, "Release time"};
            break;
        case Compressor2<TBase>::THRESHOLD_PARAM:
            ret = {0, 10, 10, "Threshold"};
            break;
        case Compressor2<TBase>::RATIO_PARAM:
            ret = {0, 8, 3, "Compression ratio"};
            break;
        case Compressor2<TBase>::MAKEUPGAIN_PARAM:
            ret = {0, 40, 0, "Makeup gain"};
            break;
        case Compressor2<TBase>::NOTBYPASS_PARAM:
            ret = {0, 1, 1, "Effect bypass"};
            break;
        case Compressor2<TBase>::WETDRY_PARAM:
            ret = {-1, 1, 1, "dry/wet mix"};
            break;
        case Compressor2<TBase>::CHANNEL_PARAM:
            ret = {1, 16, 1, "edit channel"};
            break;
        default:
            assert(false);
    }
    return ret;
}
