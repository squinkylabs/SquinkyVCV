
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

private:

    Cmprsr compressors[4];
    void setupLimiter();
    void stepm();

    int numChannels_m = 0;
    int numBanks_m = 0;
    Divider divm;

};


template <class TBase>
inline void Compressor<TBase>::init()
{
    setupLimiter();
    divm.setup(16, [this]() {
        this->stepm();
    });
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
   
    SqInput& inPort = TBase::inputs[AUDIO_INPUT];
    SqOutput& outPort = TBase::outputs[AUDIO_OUTPUT];
    
    for (int bank = 0; bank < numBanks_m; ++bank) {
        const int baseChannel = bank * 4;
        const float_4 input = inPort.getPolyVoltageSimd<float_4>(baseChannel);
        const float_4 output = compressors[bank].step(input);
        outPort.setVoltageSimd(output, baseChannel);

     //   printf("bank=%d, ch=%d\n", bank, baseChannel);
     //   printf("input = %s output=%s\n", toStr(input).c_str(), toStr(output).c_str());

        float_4 debug = compressors[bank]._lag()._memory();
        Compressor<TBase>::outputs[DEBUG_OUTPUT].setVoltageSimd(debug, baseChannel);
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
            ret = {1.0f, 1000.0f, 10, "Threshold"};
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
        default:
            assert(false);
    }
    return ret;
}


