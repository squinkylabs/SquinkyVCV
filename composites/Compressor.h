
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

private:

    Cmprsr compressors[4];
    void setupLimiter();
    void stepm();
    void stepn();

    int numChannels_m = 0;
    int numBanks_m = 0;
    Divider divm;
    Divider divn;
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
    // TODO: taper
    const float rawThresh = Compressor<TBase>::params[THRESHOLD_PARAM].value;
    const float thresh = std::max(.01f, rawThresh);
  
    float rawRatio = Compressor<TBase>::params[RATIO_PARAM].value;
    Cmprsr::Ratios ratio = Cmprsr::Ratios(int(std::round(rawRatio)));
    for (int i = 0; i<4; ++i) {
        compressors[i].setThreshold(thresh);
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
}

template <class TBase>
inline void Compressor<TBase>::process(const typename TBase::ProcessArgs& args)
{
    divm.step();
    divn.step();

    SqInput& inPort = TBase::inputs[AUDIO_INPUT];
    SqOutput& outPort = TBase::outputs[AUDIO_OUTPUT];
    
    for (int bank = 0; bank < numBanks_m; ++bank) {
        const int baseChannel = bank * 4;
        const float_4 input = inPort.getPolyVoltageSimd<float_4>(baseChannel);
        const float_4 output = compressors[bank].step(input);
        outPort.setVoltageSimd(output, baseChannel);
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
            ret = {1.0f, 1000.0f, 10, "Attack"};
            break;
         case Compressor<TBase>::RELEASE_PARAM:
            ret = {1.0f, 1000.0f, 10, "Release"};
            break;
         case Compressor<TBase>::THRESHOLD_PARAM:
            ret = {.1f, 5, 1, "Threshold"};
            break;
         case Compressor<TBase>::RATIO_PARAM:
            ret = {0, 4, 0, "Ratio"};
            break;
         case Compressor<TBase>::MAKEUPGAIN_PARAM:
            ret = {1.0f, 1000.0f, 10, "Makeup gain"};
            break;
        case Compressor<TBase>::SECRET_PARAM:
            ret = {0, 1, 0, "Makeup gain"};
            break;
        case Compressor<TBase>::WETDRY_PARAM:
            ret = {0, 1, 0, "wet/dry"};
            break;
        default:
            assert(false);
    }
    return ret;
}


