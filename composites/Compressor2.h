
#pragma once

#include <assert.h>

#include <memory>

#include "Cmprsr.h"
#include "Divider.h"
#include "IComposite.h"
#include "LookupTableFactory.h"
#include "ObjectCache.h"
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
        //  REDUCEDISTORTION_PARAM,
        NOTBYPASS_PARAM,
        WETDRY_PARAM,
        NUM_PARAMS
    };

    enum InputIds {
        LAUDIO_INPUT,
        RAUDIO_INPUT,
        NUM_INPUTS
    };

    enum OutputIds {
        LAUDIO_OUTPUT,
        RAUDIO_OUTPUT,
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

private:
    Cmprsr compressorsL[4];
    Cmprsr compressorsR[4];
    void setupLimiter();
    void stepn();
    void pollAttackRelease();

    int numChannelsL_m = 0;
    int numBanksL_m = 0;
    int numChannelsR_m = 0;
    int numBanksR_m = 0;

    float_4 wetLevel = 0;
    float_4 dryLevel = 0;
    float_4 makeupGain_m = 1;
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
    float lastThreshold = -1;
    float lastRatio = -1;
    bool bypassed = false;
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
}

template <class TBase>
inline float Compressor2<TBase>::getGainReductionDb() const {
    float_4 minGain_4 = 1;
    if (bypassed) {
        return 0;
    }

    for (int bank = 0; bank < numBanksL_m; ++bank) {
        minGain_4 = SimdBlocks::min(minGain_4, compressorsL[bank].getGain());
    }
    for (int bank = 0; bank < numBanksR_m; ++bank) {
        minGain_4 = SimdBlocks::min(minGain_4, compressorsR[bank].getGain());
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
    SqInput& inPortL = TBase::inputs[LAUDIO_INPUT];
    SqOutput& outPortL = TBase::outputs[LAUDIO_OUTPUT];
    SqInput& inPortR = TBase::inputs[RAUDIO_INPUT];
    SqOutput& outPortR = TBase::outputs[RAUDIO_OUTPUT];

    numChannelsL_m = inPortL.channels;
    numChannelsR_m = inPortR.channels;
    outPortL.setChannels(numChannelsL_m);
    outPortR.setChannels(numChannelsR_m);

    numBanksL_m = (numChannelsL_m / 4) + ((numChannelsL_m % 4) ? 1 : 0);
    numBanksR_m = (numChannelsR_m / 4) + ((numChannelsR_m % 4) ? 1 : 0);
    // printf("\n****** after stepm banks = %d numch=%d\n", numBanks_m, numChannels_m);

    pollAttackRelease();

    const float rawWetDry = Compressor2<TBase>::params[WETDRY_PARAM].value;
    if (rawWetDry != lastRawMix) {
        lastRawMix = rawWetDry;
        wetLevel = LookupTable<float>::lookup(*panR, rawWetDry, true);
        dryLevel = LookupTable<float>::lookup(*panL, rawWetDry, true);
        wetLevel *= wetLevel;
        dryLevel *= dryLevel;
    }

    const float rawMakeupGain = Compressor2<TBase>::params[MAKEUPGAIN_PARAM].value;
    if (lastRawMakeupGain != rawMakeupGain) {
        lastRawMakeupGain = rawMakeupGain;
        makeupGain_m = float(AudioMath::gainFromDb(rawMakeupGain));
    }

    const float threshold = LookupTable<float>::lookup(thresholdFunctionParams, Compressor2<TBase>::params[THRESHOLD_PARAM].value);
    const float rawRatio = Compressor2<TBase>::params[RATIO_PARAM].value;
    if (lastThreshold != threshold || lastRatio != rawRatio) {
        lastThreshold = threshold;
        lastRatio = rawRatio;
        Cmprsr::Ratios ratio = Cmprsr::Ratios(int(std::round(rawRatio)));
        for (int i = 0; i < 4; ++i) {
            compressorsL[i].setThreshold(threshold);
            compressorsR[i].setThreshold(threshold);
            compressorsL[i].setCurve(ratio);
            compressorsR[i].setCurve(ratio);

            if (i < numBanksL_m) {
                const int baseChannel = i * 4;
                const int chanThisBankL = std::min(4, numChannelsL_m - baseChannel);
                compressorsL[i].setNumChannels(chanThisBankL);
            }
            if (i < numBanksR_m) {
                const int baseChannel = i * 4;
                const int chanThisBankR = std::min(4, numChannelsR_m - baseChannel);
                compressorsR[i].setNumChannels(chanThisBankR);
            }
        }
    }

    bypassed = !bool(std::round(Compressor2<TBase>::params[NOTBYPASS_PARAM].value));
    // printf("notbypass value = %f, bypassed = %d\n", Compressor2<TBase>::params[NOTBYPASS_PARAM].value, bypassed); fflush(stdout);
}

template <class TBase>
inline void Compressor2<TBase>::pollAttackRelease() {
    const float rawAttack = Compressor2<TBase>::params[ATTACK_PARAM].value;
    const float rawRelease = Compressor2<TBase>::params[RELEASE_PARAM].value;
    const bool reduceDistortion = true;

    if (rawAttack != lastRawA || rawRelease != lastRawR) {
        lastRawA = rawAttack;
        lastRawR = rawRelease;

        const float attack = LookupTable<float>::lookup(attackFunctionParams, rawAttack);
        const float release = LookupTable<float>::lookup(releaseFunctionParams, rawRelease);

        for (int i = 0; i < 4; ++i) {
            compressorsL[i].setTimes(attack, release, TBase::engineGetSampleTime(), reduceDistortion);
            compressorsR[i].setTimes(attack, release, TBase::engineGetSampleTime(), reduceDistortion);
        }
    }
}

template <class TBase>
inline void Compressor2<TBase>::process(const typename TBase::ProcessArgs& args) {
    divn.step();

    SqInput& inPortL = TBase::inputs[LAUDIO_INPUT];
    SqOutput& outPortL = TBase::outputs[LAUDIO_OUTPUT];
    SqInput& inPortR = TBase::inputs[RAUDIO_INPUT];
    SqOutput& outPortR = TBase::outputs[RAUDIO_OUTPUT];

    if (bypassed) {
        for (int bank = 0; bank < numBanksL_m; ++bank) {
            const int baseChannel = bank * 4;
            const float_4 input = inPortL.getPolyVoltageSimd<float_4>(baseChannel);
            outPortL.setVoltageSimd(input, baseChannel);
        }
        for (int bank = 0; bank < numBanksR_m; ++bank) {
            const int baseChannel = bank * 4;
            const float_4 input = inPortR.getPolyVoltageSimd<float_4>(baseChannel);
            outPortR.setVoltageSimd(input, baseChannel);
        }
        return;
    }

    for (int bank = 0; bank < numBanksL_m; ++bank) {
        const int baseChannel = bank * 4;
        const float_4 input = inPortL.getPolyVoltageSimd<float_4>(baseChannel);
        const float_4 wetOutput = compressorsL[bank].step(input) * makeupGain_m;
        const float_4 mixedOutput = wetOutput * wetLevel + input * dryLevel;

        outPortL.setVoltageSimd(mixedOutput, baseChannel);
    }
    for (int bank = 0; bank < numBanksR_m; ++bank) {
        const int baseChannel = bank * 4;
        const float_4 input = inPortR.getPolyVoltageSimd<float_4>(baseChannel);
        const float_4 wetOutput = compressorsR[bank].step(input) * makeupGain_m;
        const float_4 mixedOutput = wetOutput * wetLevel + input * dryLevel;

        outPortR.setVoltageSimd(mixedOutput, baseChannel);
    }
}

// TODO: do we still need this old init function? combine with other?
template <class TBase>
inline void Compressor2<TBase>::setupLimiter() {
    for (int i = 0; i < 4; ++i) {
        compressorsL[i].setTimes(1, 100, TBase::engineGetSampleTime(), false);
        compressorsR[i].setTimes(1, 100, TBase::engineGetSampleTime(), false);
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
        default:
            assert(false);
    }
    return ret;
}
