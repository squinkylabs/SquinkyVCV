
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
#include "engine/Port.hpp"

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
        compressors[0].setIsPolyCV(true);
        compressors[1].setIsPolyCV(true);
        compressors[2].setIsPolyCV(true);
        compressors[3].setIsPolyCV(true);
    }

    Compressor2() : TBase() {
        compressors[0].setIsPolyCV(true);
        compressors[1].setIsPolyCV(true);
        compressors[2].setIsPolyCV(true);
        compressors[3].setIsPolyCV(true);
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
        STEREO_PARAM,
        LABELS_PARAM,
        SIDECHAIN_PARAM,
        SIDECHAIN_ALL_PARAM,
        // EXPERIMENT_PARAM,
        NUM_PARAMS
    };

    enum InputIds {
        LAUDIO_INPUT,
        SIDECHAIN_INPUT,
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

    int getNumVUChannels() const {
        const int ch = currentStereo_m ? (numChannels_m + 1) / 2 : numChannels_m;
        // SQINFO("get num vu will ret %d, st=%d, rawch =%d", ch, currentStereo_m, numChannels_m);
        return ch;
    }
    float getChannelGain(int ch) const;
    CompressorParamHolder& getParamHolder() { return compParams; }

    void ui_paste(CompressorParamChannel* pasteData) { pastePointer = pasteData; }
    void ui_setAllChannelsToCurrent() { setAllChannelsSameFlag = true; }
    void ui_initCurrentChannel() {
        initCurrentChannelFlag = true;
    }

    Cmprsr& _getComp(int bank);

    float_4 _getWet(int bank) const { return wetLevel[bank]; }
    float_4 _getEn(int bank) const { return enabled[bank]; }
    float_4 _getG(int bank) const { return makeupGain[bank]; }
    void _initParamOnAllChannels(int param, float value);

    void initAllParams();
    void initCurrentChannelParams();

private:
    CompressorParamHolder compParams;

    Cmprsr compressors[4];
    void setupLimiter();
    void stepn();
    void pollAttackRelease();
    void updateAttackAndRelease(int bank);
    void pollThresholdAndRatio();
    void updateThresholdAndRatio(int bank);
    void pollWetDry();
    void updateWetDry(int bank);
    void pollBypassed();
    void pollSidechainEnabled();
    void updateBypassed(int bank);
    //void updateSidechainEnabled(int bank);
    void pollMakeupGain();
    void updateMakeupGain(int bank);
    void updateCurrentChannel();
    void pollStereo();
    void makeAllSettingsStereo();
    void setLinkAllBanks(bool);
    void updateAllChannels();
    void pollUI();

    /**
     * numChannels_m is the total mono channels. So even in
     * stereo-8 mode, there can still be 0..15 of these
     */
    int numChannels_m = 0;
    int numBanks_m = 0;
    int currentBank_m = -1;

    /**  
     * Which of the channels we are editing ATM.
     * In mono this will be from 0..15
     * in stereo this will be from 0..7
     */
    int currentChannel_m = -1;
    int currentStereo_m = -1;

    float_4 wetLevel[4] = {0};
    float_4 dryLevel[4] = {0};
    float_4 enabled[4] = {0};
    float_4 makeupGain[4] = {1};

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
    int lastChannelCount = -1;
    bool lastNotBypassed = false;
    bool lastSidechainEnabled = false;

    std::atomic<bool> setAllChannelsSameFlag = {false};
    std::atomic<bool> initCurrentChannelFlag = {false};
    std::atomic<CompressorParamChannel*> pastePointer = {nullptr};

    std::shared_ptr<IComposite> compDescription = {Compressor2<TBase>::getDescription()};
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
 * set all the params to the right setting for the new channel
 */
template <class TBase>
inline void Compressor2<TBase>::updateCurrentChannel() {
    //SQINFO("update current channel called with stereo = %d", currentStereo_m);
    // SQINFO("update current channel about to set thre to %f curch=%d\n", compParams.getThreshold(currentChannel_m), currentChannel_m);

    const int sourceParamChannel = currentStereo_m ? (currentChannel_m * 2) : currentChannel_m;
    assert(sourceParamChannel < 16);
    Compressor2<TBase>::params[ATTACK_PARAM].value = compParams.getAttack(sourceParamChannel);
    Compressor2<TBase>::params[RELEASE_PARAM].value = compParams.getRelease(currentChannel_m);
    Compressor2<TBase>::params[THRESHOLD_PARAM].value = compParams.getThreshold(sourceParamChannel);
    Compressor2<TBase>::params[RATIO_PARAM].value = float(compParams.getRatio(sourceParamChannel));
    Compressor2<TBase>::params[MAKEUPGAIN_PARAM].value = compParams.getMakeupGain(sourceParamChannel);
    Compressor2<TBase>::params[NOTBYPASS_PARAM].value = compParams.getEnabled(sourceParamChannel);
    Compressor2<TBase>::params[WETDRY_PARAM].value = compParams.getWetDryMix(sourceParamChannel);
    Compressor2<TBase>::params[SIDECHAIN_PARAM].value = compParams.getSidechainEnabled(sourceParamChannel);
    assert(getParamHolder().getNumParams() == 8);
}

/**
 * This function should set all members of param holders to defaults,
 * and then refresh all the compressors with those values
 */
template <class TBase>
inline void Compressor2<TBase>::initAllParams() {
    auto icomp = compDescription;
    for (int i = 0; i < 16; ++i) {
        compParams.setAttack(i, icomp->getParam(ATTACK_PARAM).def);
        compParams.setRelease(i, icomp->getParam(RELEASE_PARAM).def);
        compParams.setRatio(i, int(std::round(icomp->getParam(RATIO_PARAM).def)));
        compParams.setThreshold(i, icomp->getParam(THRESHOLD_PARAM).def);
        compParams.setMakeupGain(i, icomp->getParam(MAKEUPGAIN_PARAM).def);
        compParams.setEnabled(i, bool(std::round(icomp->getParam(NOTBYPASS_PARAM).def)));
        compParams.setWetDry(i, icomp->getParam(WETDRY_PARAM).def);
        compParams.setSidechainEnabled(i, bool(std::round(icomp->getParam(SIDECHAIN_PARAM).def)));
        assert(getParamHolder().getNumParams() == 8);
    }

    TBase::params[ATTACK_PARAM].value = icomp->getParam(ATTACK_PARAM).def;
    TBase::params[RELEASE_PARAM].value = icomp->getParam(RELEASE_PARAM).def;
    TBase::params[RATIO_PARAM].value = icomp->getParam(RATIO_PARAM).def;
    TBase::params[THRESHOLD_PARAM].value = icomp->getParam(THRESHOLD_PARAM).def;
    TBase::params[MAKEUPGAIN_PARAM].value = icomp->getParam(MAKEUPGAIN_PARAM).def;
    TBase::params[NOTBYPASS_PARAM].value = icomp->getParam(NOTBYPASS_PARAM).def;
    TBase::params[WETDRY_PARAM].value = icomp->getParam(WETDRY_PARAM).def;
    TBase::params[SIDECHAIN_PARAM].value = icomp->getParam(SIDECHAIN_PARAM).def;
    assert(getParamHolder().getNumParams() == 8);

    updateAllChannels();
}

template <class TBase>
inline void Compressor2<TBase>::_initParamOnAllChannels(int param, float value) {
    TBase::params[param].value = value;

    for (int i = 0; i < 16; ++i) {
        switch (param) {
            case NOTBYPASS_PARAM:
                compParams.setEnabled(i, value);
                break;
            case RATIO_PARAM:
                compParams.setRatio(i, int(std::round(value)));
                break;
            default:

                assert(false);
        }
    }
    updateAllChannels();
}

template <class TBase>
inline void Compressor2<TBase>::updateAllChannels() {
    //SQINFO("updateAllChannels");
    for (int bank = 0; bank < 4; ++bank) {
        updateAttackAndRelease(bank);
        updateThresholdAndRatio(bank);
        updateWetDry(bank);
        updateMakeupGain(bank);
        updateBypassed(bank);
        // updateSidechainEnabled(bank);
        updateMakeupGain(bank);
        // TODO: put all the update here
    }
}

template <class TBase>
inline void Compressor2<TBase>::makeAllSettingsStereo() {
    // SQINFO("make all settings stereo");
    for (int i = 0; i < 8; ++i) {
        const int left = i * 2;
        const int right = left + 1;

        {
            const float a = .5f * (compParams.getAttack(left) + compParams.getAttack(right));
            compParams.setAttack(left, a);
            compParams.setAttack(right, a);
        }
        {
            const float r = .5f * (compParams.getRelease(left) + compParams.getRelease(right));
            compParams.setRelease(left, r);
            compParams.setRelease(right, r);
        }
        {
            const int ratio = compParams.getRatio(left);
            compParams.setRatio(right, ratio);
        }
        {
            const float th = .5f * (compParams.getThreshold(left) + compParams.getThreshold(right));
            compParams.setThreshold(left, th);
            compParams.setThreshold(right, th);
        }
        {
            const float m = .5f * (compParams.getMakeupGain(left) + compParams.getMakeupGain(right));
            compParams.setMakeupGain(left, m);
            compParams.setMakeupGain(right, m);
        }
        {
            const bool b = compParams.getEnabled(left);
            compParams.setEnabled(right, b);
        }
        {
            const bool b = compParams.getSidechainEnabled(left);
            compParams.setSidechainEnabled(right, b);
        }
        {
            const float w = .5f * (compParams.getWetDryMix(left) + compParams.getWetDryMix(right));
            compParams.setWetDry(left, w);
            compParams.setWetDry(right, w);
        }
        assert(getParamHolder().getNumParams() == 8);
    }

    // I'm not sure this is needed here?
    updateAllChannels();
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
    float gain = 1;

    if (currentStereo_m > 0) {
        const int channelLeft = 2 * ch;
        if (!compParams.getEnabled(channelLeft)) {
            return 1;  // no reduction
        }
        const int bank = channelLeft / 4;
        const int subChanL = channelLeft - bank * 4;

        const float_4 g = compressors[bank].getGain();
        const float gainL = g[subChanL];
        const float gainR = g[subChanL + 1];

        gain = std::min(gainL, gainR);
        //SQINFO("st ch=%d, b=%d chl=%d sub=%d ret=%.2f", ch, bank, channelLeft, subChanL, gain);
    } else {
        if (!compParams.getEnabled(ch)) {
            return 1;  // no reduction
        }
        const int bank = ch / 4;
        const int subChan = ch - bank * 4;

        float_4 g = compressors[bank].getGain();
        gain = g[subChan];
    }
    return gain;
}

template <class TBase>
inline void Compressor2<TBase>::pollUI() {
    bool update = false;
    CompressorParamHolder& holder = getParamHolder();
    if (setAllChannelsSameFlag) {
        update = true;
        setAllChannelsSameFlag.store(false);
        const int monoChannel = (currentStereo_m > 0) ? currentChannel_m * 2 : currentChannel_m;

        for (int i = 0; i < 16; ++i) {
            if (i != monoChannel) {
                holder.copy(i, monoChannel);
            }
        }
    }

    if (pastePointer) {
        const CompressorParamChannel* ptr = pastePointer;
        pastePointer = nullptr;
        TBase::params[ATTACK_PARAM].value = ptr->attack;
        TBase::params[RELEASE_PARAM].value = ptr->release;
        TBase::params[THRESHOLD_PARAM].value = ptr->threshold;
        TBase::params[RATIO_PARAM].value = float(ptr->ratio);
        TBase::params[MAKEUPGAIN_PARAM].value = ptr->makeupGain;
        TBase::params[NOTBYPASS_PARAM].value = ptr->enabled ? 1.f : 0.f;
        TBase::params[SIDECHAIN_PARAM].value = ptr->sidechainEnabled ? 1.f : 0.f;
        TBase::params[WETDRY_PARAM].value = ptr->attack;
        assert(getParamHolder().getNumParams() == 8);
#if 0
        update = true;
        if (currentStereo_m > 0) {
            ptr->copyToHolder(holder, current
           assert(false);
        }
#endif
    }

    if (initCurrentChannelFlag) {
        initCurrentChannelFlag.store(false);

        auto icomp = compDescription;
        TBase::params[ATTACK_PARAM].value = icomp->getParam(ATTACK_PARAM).def;
        TBase::params[RELEASE_PARAM].value = icomp->getParam(RELEASE_PARAM).def;
        TBase::params[RATIO_PARAM].value = icomp->getParam(RATIO_PARAM).def;
        TBase::params[THRESHOLD_PARAM].value = icomp->getParam(THRESHOLD_PARAM).def;
        TBase::params[MAKEUPGAIN_PARAM].value = icomp->getParam(MAKEUPGAIN_PARAM).def;
        TBase::params[NOTBYPASS_PARAM].value = icomp->getParam(NOTBYPASS_PARAM).def;
        TBase::params[SIDECHAIN_PARAM].value = icomp->getParam(SIDECHAIN_PARAM).def;
        TBase::params[WETDRY_PARAM].value = icomp->getParam(WETDRY_PARAM).def;
        assert(getParamHolder().getNumParams() == 8);
    }

    if (update) {
        updateAllChannels();
    }
}

template <class TBase>
inline void Compressor2<TBase>::stepn() {
    pollUI();
    pollStereo();

    SqInput& inPort = TBase::inputs[LAUDIO_INPUT];
    SqOutput& outPort = TBase::outputs[LAUDIO_OUTPUT];

    numChannels_m = inPort.channels;
    outPort.setChannels(numChannels_m);

    numBanks_m = (numChannels_m / 4) + ((numChannels_m % 4) ? 1 : 0);

    currentChannel_m = -1 + int(std::round(TBase::params[CHANNEL_PARAM].value));
    assert(currentChannel_m >= 0);
    assert(currentChannel_m <= 15);

    // if stereo setting makes current channel invalid, fix it
    if ((currentStereo_m > 0) && (currentChannel_m > 7)) {
        currentChannel_m = 7;
    }

    if (currentStereo_m) assert(currentChannel_m <= 7);
    currentBank_m = currentStereo_m ? (currentChannel_m / 2) : (currentChannel_m / 4);

    if (currentChannel_m != lastChannelCount) {
        lastChannelCount = currentChannel_m;
        updateCurrentChannel();
    }

    pollAttackRelease();
    pollWetDry();
    pollMakeupGain();
    pollThresholdAndRatio();
    pollBypassed();
    pollSidechainEnabled();
}

template <class TBase>
inline void Compressor2<TBase>::pollStereo() {
    const int stereo = int(std::round(Compressor2<TBase>::params[STEREO_PARAM].value));
    if (stereo != currentStereo_m) {
        currentStereo_m = stereo;
        if (currentStereo_m > 0) {
            makeAllSettingsStereo();
        }
        setLinkAllBanks(stereo == 2);
    }
}

template <class TBase>
inline void Compressor2<TBase>::setLinkAllBanks(bool linked) {
    for (int i = 0; i < 4; ++i) {
        compressors[i].setLinked(linked);
    }
}

template <class TBase>
inline void Compressor2<TBase>::pollMakeupGain() {
    const float g = Compressor2<TBase>::params[MAKEUPGAIN_PARAM].value;
    if (g != lastRawMakeupGain) {
        lastRawMakeupGain = g;
        if (!currentStereo_m) {
            compParams.setMakeupGain(currentChannel_m, g);
        } else {
            compParams.setMakeupGain(2 * currentChannel_m, g);
            compParams.setMakeupGain(2 * currentChannel_m + 1, g);
        }
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
        if (!currentStereo_m) {
            compParams.setEnabled(currentChannel_m, notByp);
        } else {
            compParams.setEnabled(2 * currentChannel_m, notByp);
            compParams.setEnabled(2 * currentChannel_m + 1, notByp);
        }
        updateBypassed(currentBank_m);
    }
}

template <class TBase>
inline void Compressor2<TBase>::pollSidechainEnabled() {
    const bool sc = bool(std::round(Compressor2<TBase>::params[SIDECHAIN_PARAM].value));
    if (sc != lastSidechainEnabled) {
        lastSidechainEnabled = sc;
        if (!currentStereo_m) {
            compParams.setSidechainEnabled(currentChannel_m, sc);
        } else {
            compParams.setSidechainEnabled(2 * currentChannel_m, sc);
            compParams.setSidechainEnabled(2 * currentChannel_m + 1, sc);
        }
        // updateSidechainEnabled(currentBank_m);
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
    //   const bool reduceDistortion = true;

    if (rawAttack != lastRawA || rawRelease != lastRawR) {
        lastRawA = rawAttack;
        lastRawR = rawRelease;

        if (!currentStereo_m) {
            compParams.setAttack(currentChannel_m, rawAttack);
            compParams.setRelease(currentChannel_m, rawRelease);
        } else {
            assert(currentChannel_m < 8);
            compParams.setAttack(2 * currentChannel_m, rawAttack);
            compParams.setAttack(2 * currentChannel_m + 1, rawAttack);
            compParams.setRelease(2 * currentChannel_m, rawRelease);
            compParams.setRelease(2 * currentChannel_m + 1, rawRelease);
        }
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

        if (!currentStereo_m) {
            compParams.setThreshold(currentChannel_m, rawThreshold);
            compParams.setRatio(currentChannel_m, int(std::round(rawRatio)));
        } else {
            compParams.setThreshold(2 * currentChannel_m, rawThreshold);
            compParams.setThreshold(2 * currentChannel_m + 1, rawThreshold);
            compParams.setRatio(2 * currentChannel_m, int(std::round(rawRatio)));
            compParams.setRatio(2 * currentChannel_m + 1, int(std::round(rawRatio)));
        }
        updateThresholdAndRatio(currentBank_m);
    }
}

template <class TBase>
inline void Compressor2<TBase>::pollWetDry() {
    const float rawWetDry = Compressor2<TBase>::params[WETDRY_PARAM].value;
    if (rawWetDry != lastRawMix) {
        lastRawMix = rawWetDry;
        if (!currentStereo_m) {
            compParams.setWetDry(currentChannel_m, rawWetDry);
        } else {
            compParams.setWetDry(2 * currentChannel_m, rawWetDry);
            compParams.setWetDry(2 * currentChannel_m + 1, rawWetDry);
        }
        updateWetDry(currentBank_m);
    }
}

template <class TBase>
inline void Compressor2<TBase>::updateThresholdAndRatio(int bank) {
    auto rawRatios_4 = compParams.getRatios(bank);
    float_4 rawThresholds_4 = compParams.getThresholds(bank);
    float_4 th;
    Cmprsr::Ratios r[4];

    for (int i = 0; i < 4; ++i) {
        const float threshold = LookupTable<float>::lookup(thresholdFunctionParams, rawThresholds_4[i]);
        th[i] = threshold;
        r[i] = Cmprsr::Ratios(rawRatios_4[i]);
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
    SqInput& scPort =  TBase::inputs[SIDECHAIN_INPUT];

    // TODO: bypassed per channel - need to completely re-do
    for (int bank = 0; bank < numBanks_m; ++bank) {
        const int baseChannel = bank * 4;
        const float_4 en = compParams.getEnableds(bank);
        simd_assertMask(en);

        int32_4 en2 = en;
        const float_4 input = inPort.getPolyVoltageSimd<float_4>(baseChannel);
        //  SQINFO("i = %d en2= %s", bank, toStr(en2).c_str());
        if (!en2[0] && !en2[1] && !en2[2] && !en2[3]) {
            outPort.setVoltageSimd(input, baseChannel);
            // SQINFO("bypassed");
        } else {
            const float_4 scIn = scPort.getPolyVoltageSimd<float_4>(baseChannel);
            const float_4 scEnabled = compParams.getSidechainEnableds(bank);
            const float_4 detectorInput = SimdBlocks::ifelse(scEnabled, scIn, input);
            const float_4 wetOutput = compressors[bank].stepPoly(input, detectorInput) * makeupGain[bank];
            const float_4 mixedOutput = wetOutput * wetLevel[bank] + input * dryLevel[bank];

            const float_4 out = SimdBlocks::ifelse(en, mixedOutput, input);
            //  SQINFO("\nbank=%d input=%s wet=%s", bank, toStr(input).c_str(), toStr(wetOutput).c_str());
            //  SQINFO("en=%s, true=%s", toStr(en).c_str(), toStr(SimdBlocks::maskTrue()).c_str());
            //  SQINFO("output=%s", toStr(out).c_str());
            outPort.setVoltageSimd(out, baseChannel);
        }
    }
}

// TODO: do we still need this old init function? combine with other?
template <class TBase>
inline void Compressor2<TBase>::setupLimiter() {
    for (int i = 0; i < 4; ++i) {
        compressors[i].setTimesPoly(1, 100, TBase::engineGetSampleTime());
    }
}

template <class TBase>
inline void Compressor2<TBase>::onSampleRateChange() {
    // should probably just reset cache here??
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
            ret = {0, 1, 0, "Effect bypass"};
            break;
        case Compressor2<TBase>::WETDRY_PARAM:
            ret = {-1, 1, 1, "dry/wet mix"};
            break;
        case Compressor2<TBase>::CHANNEL_PARAM:
            ret = {1, 16, 1, "edit channel"};
            break;
        case Compressor2<TBase>::STEREO_PARAM:
            ret = {0, 2, 2, "stereo"};
            break;
        case Compressor2<TBase>::LABELS_PARAM:
            ret = {0, 2, 0, "labels"};
            break;
        case Compressor2<TBase>::SIDECHAIN_PARAM:
            ret = {0, 1, 0, "sidechain enable"};
            break;
        case Compressor2<TBase>::SIDECHAIN_ALL_PARAM:
            ret = {0, 1, 0, "sidechain all"};
            break;
#if 0
        case Compressor2<TBase>::EXPERIMENT_PARAM:
            ret = {0, 2, 0, "experiment"};
            break;
#endif
        default:
            assert(false);
    }
    return ret;
}
