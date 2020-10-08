
#pragma once

#include "Divider.h"
#include "Cmprsr.h"
#include "SqPort.h"

#include <assert.h>
#include <memory>
#include "IComposite.h"

namespace rack {
    namespace engine {
        struct Module;
    }
}
using Module = ::rack::engine::Module;


template <class TBase>
class CompressorDescription : public IComposite
{
public:
    Config getParam(int i) override;
    int getNumParams() override;
};

template <class TBase>
class Compressor : public TBase
{
public:

    Compressor(Module * module) : TBase(module)
    {
    }
    Compressor() : TBase()
    {
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
        ATTACK_PARAM,
        RELEASE_PARAM,
        THRESHOLD_PARAM,
        RATIO_PARAM,
        MAKEUPGAIN_PARAM,
        SECRET_PARAM,
        WETDRY_PARAM,
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
        DEBUG_OUTPUT,
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
        return std::make_shared<CompressorDescription<TBase>>();
    }

    /**
     * Main processing entry point. Called every sample
     */
    //void step() override;
    void process(const typename TBase::ProcessArgs& args) override;

    void onSampleRateChange() override;

    static std::vector<std::string> ratios();

    static std::function<double(double)> getAttackFunction() {
        return AudioMath::makeFunc_Exp(0, 1, .1, 30);
    }

    static std::function<double(double)> getReleaseFunction() {
        return AudioMath::makeFunc_Exp(0, 1, 100, 1600);
    }
    static std::function<double(double)> getThresholdFunction() {
        return AudioMath::makeFunc_Exp(0, 10, .1, 10);
    }

private:

    Cmprsr compressors[4];
    void setupLimiter();
    void stepm();
    void stepn();
    void pollAttackRelease();

    int numChannels_m = 0;
    int numBanks_m = 0;
    Divider divm;
    Divider divn;

    std::function<double(double)> attackFunction = getAttackFunction();
    std::function<double(double)> releaseFunction = getReleaseFunction();
    std::function<double(double)> thresholdFunction = getThresholdFunction();
};

template <class TBase>
inline std::vector<std::string> Compressor<TBase>::ratios()
{
    return Cmprsr::ratios();
}

template <class TBase>
inline void Compressor<TBase>::init()
{
    setupLimiter();
    divm.setup(16, [this]() {
        this->stepm();
    });
    divn.setup(4, [this]() {
        this->stepn();
    });
}

template <class TBase>
inline void Compressor<TBase>::stepn()
{
    float threshold = thresholdFunction(Compressor<TBase>::params[THRESHOLD_PARAM].value);
  
    float rawRatio = Compressor<TBase>::params[RATIO_PARAM].value;
    Cmprsr::Ratios ratio = Cmprsr::Ratios(int(std::round(rawRatio)));
    for (int i = 0; i<4; ++i) {
        compressors[i].setThreshold(threshold);
        compressors[i].setCurve(ratio);
    }
}

template <class TBase>
inline void Compressor<TBase>::stepm()
{
    SqInput& inPort = TBase::inputs[AUDIO_INPUT];
    SqOutput& outPort = TBase::outputs[AUDIO_OUTPUT];
    numChannels_m = inPort.channels;
    outPort.setChannels(numChannels_m);
    numBanks_m = (numChannels_m / 4) + ((numChannels_m % 4) ? 1 : 0);   
    // printf("\n****** after stepm banks = %d numch=%d\n", numBanks_m, numChannels_m);

    pollAttackRelease();
}

template <class TBase>
inline void Compressor<TBase>::pollAttackRelease()
{
    const float attackRaw = Compressor<TBase>::params[ATTACK_PARAM].value;
    const float releaseRaw = Compressor<TBase>::params[RELEASE_PARAM].value;

    const float attack = attackFunction(attackRaw);
    const float release = releaseFunction(releaseRaw);
   // printf("in poll, raw=%f,%f a=%f r=%f\n", attackRaw, releaseRaw, attack, release); fflush(stdout);

    for (int i = 0; i<4; ++i) {
        compressors[i].setTimes(attack, release, TBase::engineGetSampleTime());
    }
}

template <class TBase>
inline void Compressor<TBase>::process(const typename TBase::ProcessArgs& args)
{
    divm.step();
    divn.step();

    SqInput& inPort = TBase::inputs[AUDIO_INPUT];
    SqOutput& outPort = TBase::outputs[AUDIO_OUTPUT];
    SqOutput& debugPort = TBase::outputs[DEBUG_OUTPUT];
    
    for (int bank = 0; bank < numBanks_m; ++bank) {
        const int baseChannel = bank * 4;
        const float_4 input = inPort.getPolyVoltageSimd<float_4>(baseChannel);
        const float_4 output = compressors[bank].step(input);
        outPort.setVoltageSimd(output, baseChannel);

        const float_4 env = compressors[bank]._lag().get();
        debugPort.setVoltageSimd(env, baseChannel);
    }
}

template <class TBase>
inline void Compressor<TBase>::setupLimiter()
{
    for (int i = 0; i<4; ++i) {
        compressors[i].setTimes(1, 100, TBase::engineGetSampleTime());
    }
}

template <class TBase>
inline void Compressor<TBase>::onSampleRateChange()
{
    setupLimiter();
}

template <class TBase>
int CompressorDescription<TBase>::getNumParams()
{
    return Compressor<TBase>::NUM_PARAMS;
}

template <class TBase>
inline IComposite::Config CompressorDescription<TBase>::getParam(int i)
{
    Config ret(0, 1, 0, "");
    switch (i) {
        case Compressor<TBase>::ATTACK_PARAM:
            ret = {0, 1, .1, "Attack time"};
            break;
         case Compressor<TBase>::RELEASE_PARAM:
            ret = {0, 1, .1, "Release time"};
            break;
         case Compressor<TBase>::THRESHOLD_PARAM:
            ret = {0, 10, 1, "Threshold"};
            break;
         case Compressor<TBase>::RATIO_PARAM:
            ret = {0, 4, 0, "Compression ratio"};
            break;
         case Compressor<TBase>::MAKEUPGAIN_PARAM:
            ret = {1.0f, 1000.0f, 10, "Makeup gain"};
            break;
        case Compressor<TBase>::SECRET_PARAM:
            ret = {0, 1, 0, "IM Distortion supression"};
            break;
        case Compressor<TBase>::WETDRY_PARAM:
            ret = {0, 1, 0, "wet/dry mix"};
            break;
        default:
            assert(false);
    }
    return ret;
}


