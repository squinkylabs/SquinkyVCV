
#pragma once

#include "Divider.h"
#include "Cmprsr.h"
#include "LookupTableFactory.h"
#include "ObjectCache.h"
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

/**
 *  * After poll 16 -> 32
 * 1 ch lim: 9.2
 * 1 ch curve: 17.7
 * 16 ch lim: 16.1
 * 16 ch curve: 111
 * 16h ch lim no dist:  17.9
 * 
 * After lookups for knobs:
 * 1 ch lim: 14.8
 * 1 ch curve: 23.1
 * 16 ch lim: 21.6
 * 16 ch curve: 117.5
 * 16h ch lim no dist:  23.9
 * 
 * 
 * after obvious perf:
 * 
 * 1 ch lim: 17.4
 * 1 ch curve: 25.5
 * 16 ch lim: 24.1
 * 16 ch curve: 119
 * 16h ch lim no dist: 25.6
 *  
 * 1 ch lim: 21
 * 1 ch curve: 42.9
 * 16 ch lim: 27.1
 * 16 ch curve: 123
 * 16h ch lim no dist: 28.6
 */
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
        REDUCEDISTORTION_PARAM,
        WETDRY_PARAM,
        NUM_PARAMS
    };

    enum InputIds
    {
        LAUDIO_INPUT,
        RAUDIO_INPUT,
        NUM_INPUTS
    };

    enum OutputIds
    {
        LAUDIO_OUTPUT,
        RAUDIO_OUTPUT,
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


    static std::function<double(double)> getSlowAttackFunction() {
        return AudioMath::makeFunc_Exp(0, 1, .1, 30);
    }

    static std::function<double(double)> getSlowReleaseFunction() {
        return AudioMath::makeFunc_Exp(0, 1, 100, 1600);
    }
    static std::function<double(double)> getSlowThresholdFunction() {
        return AudioMath::makeFunc_Exp(0, 10, .1, 10);
    }


   // float lookupAttack(float raw) const;
   // float lookupRelease(float raw) const;
   // float lookupthreshold(float raw) const;

private:

    Cmprsr compressors[4];
    void setupLimiter();
    //void stepm();
    void stepn();
    void pollAttackRelease();

    int numChannels_m = 0;
    int numBanks_m = 0;
    float_4 wetLevel = 0;
    float_4 dryLevel = 0;
    float_4 makeupGain_m = 1;
 //   Divider divm;
    Divider divn;

#if 0
    std::function<double(double)> attackFunction = getAttackFunction();
    std::function<double(double)> releaseFunction = getReleaseFunction();
    std::function<double(double)> thresholdFunction = getThresholdFunction();
#endif
    // we could unify this stuff with the ui stuff, above.
    LookupTableParams<float> attackFunctionParams;
    LookupTableParams<float> releaseFunctionParams;
    LookupTableParams<float> thresholdFunctionParams;

    std::shared_ptr<LookupTableParams<float>> panL = ObjectCache<float>::getMixerPanL();
    std::shared_ptr<LookupTableParams<float>> panR = ObjectCache<float>::getMixerPanR();
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
    divn.setup(32, [this]() {
        this->stepn();
    });

    LookupTableFactory<float>::makeGenericExpTaper(64, attackFunctionParams, 0, 1, .1, 30);
    LookupTableFactory<float>::makeGenericExpTaper(64, releaseFunctionParams, 0, 1, 100, 1600);
    LookupTableFactory<float>::makeGenericExpTaper(64, thresholdFunctionParams, 0, 10, .1, 10);
}

template <class TBase>
inline void Compressor<TBase>::stepn()
{
    SqInput& inPort = TBase::inputs[LAUDIO_INPUT];
    SqOutput& outPort = TBase::outputs[LAUDIO_OUTPUT];
    numChannels_m = inPort.channels;
    outPort.setChannels(numChannels_m);
    numBanks_m = (numChannels_m / 4) + ((numChannels_m % 4) ? 1 : 0);   
    // printf("\n****** after stepm banks = %d numch=%d\n", numBanks_m, numChannels_m);

    pollAttackRelease();

    const float rawWetDry = Compressor<TBase>::params[WETDRY_PARAM].value;
    wetLevel = LookupTable<float>::lookup(*panR, rawWetDry, true);
    dryLevel = LookupTable<float>::lookup(*panL, rawWetDry, true);
    wetLevel *= wetLevel;
    dryLevel *= dryLevel;

    const float rawMakeupGain = Compressor<TBase>::params[MAKEUPGAIN_PARAM].value;
    makeupGain_m = AudioMath::gainFromDb(rawMakeupGain);

   // const float threshold = thresholdFunction(Compressor<TBase>::params[THRESHOLD_PARAM].value);
    const float threshold = LookupTable<float>::lookup(thresholdFunctionParams, Compressor<TBase>::params[THRESHOLD_PARAM].value);
    const float rawRatio = Compressor<TBase>::params[RATIO_PARAM].value;
  
    Cmprsr::Ratios ratio = Cmprsr::Ratios(int(std::round(rawRatio)));
    for (int i = 0; i<4; ++i) {
        compressors[i].setThreshold(threshold);
        compressors[i].setCurve(ratio);

        if (i < numBanks_m) {
            const int baseChannel = i * 4;
            const int chanThisBank = std::min(4, numChannels_m - baseChannel);
            // printf("bank %d num ch = %d\n", i, chanThisBank); fflush(stdout);
            compressors[i].setNumChannels(chanThisBank);
        }
    }
}

template <class TBase>
inline void Compressor<TBase>::pollAttackRelease()
{
    #if 0
    const float attackRaw = Compressor<TBase>::params[ATTACK_PARAM].value;
    const float releaseRaw = Compressor<TBase>::params[RELEASE_PARAM].value;

    const float attack = lookupAttack(attackRaw);
    const float release = lookupRelease(releaseRaw);

#endif
    const float attack = LookupTable<float>::lookup(attackFunctionParams, Compressor<TBase>::params[ATTACK_PARAM].value);
    const float release = LookupTable<float>::lookup(releaseFunctionParams, Compressor<TBase>::params[RELEASE_PARAM].value);
   // printf("in poll, raw=%f,%f a=%f r=%f\n", attackRaw, releaseRaw, attack, release); fflush(stdout);

    const bool reduceDistortion = bool ( std::round(Compressor<TBase>::params[REDUCEDISTORTION_PARAM].value));
    for (int i = 0; i<4; ++i) {
        compressors[i].setTimes(attack, release, TBase::engineGetSampleTime(), reduceDistortion);
    }
}

template <class TBase>
inline void Compressor<TBase>::process(const typename TBase::ProcessArgs& args)
{
    divn.step();

    SqInput& inPort = TBase::inputs[LAUDIO_INPUT];
    SqOutput& outPort = TBase::outputs[LAUDIO_OUTPUT];
    // SqOutput& debugPort = TBase::outputs[DEBUG_OUTPUT];
    
    for (int bank = 0; bank < numBanks_m; ++bank) {
        const int baseChannel = bank * 4;
        const float_4 input = inPort.getPolyVoltageSimd<float_4>(baseChannel);
        const float_4 wetOutput = compressors[bank].step(input) * makeupGain_m;
        const float_4 mixedOutput = wetOutput * wetLevel + input * dryLevel;

        outPort.setVoltageSimd(mixedOutput, baseChannel);

        // const float_4 env = compressors[bank]._lag().get();
        // debugPort.setVoltageSimd(env, baseChannel);
    }
}

template <class TBase>
inline void Compressor<TBase>::setupLimiter()
{
    for (int i = 0; i<4; ++i) {
        compressors[i].setTimes(1, 100, TBase::engineGetSampleTime(), false);
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
            ret = {0, 8, 3, "Compression ratio"};
            break;
         case Compressor<TBase>::MAKEUPGAIN_PARAM:
            ret = {0, 40, 0, "Makeup gain"};
            break;
        case Compressor<TBase>::REDUCEDISTORTION_PARAM:
            ret = {0, 1, 0, "IM Distortion suppression"};
            break;
        case Compressor<TBase>::WETDRY_PARAM:
            ret = {-1, 1, 1, "dry/wet mix"};
            break;
        default:
            assert(false);
    }
    return ret;
}


