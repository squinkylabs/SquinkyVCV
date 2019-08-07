
#pragma once

#include <assert.h>
#include <memory>
#include "IComposite.h"
#include "PitchUtils.h"

namespace rack {
    namespace engine {
        struct Module;
    }
}
using Module = rack::engine::Module;


template <class TBase>
class DrumTriggerDescription : public IComposite
{
public:
    Config getParam(int i) override;
    int getNumParams() override;
};

template <class TBase>
class DrumTrigger : public TBase
{
public:

    DrumTrigger(Module * module) : TBase(module)
    {
    }
    DrumTrigger() : TBase()
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
        TEST_PARAM,
        NUM_PARAMS
    };

    enum InputIds
    {
        CV_INPUT,
        GATE_INPUT,
        NUM_INPUTS
    };

    enum OutputIds
    {
        GATE0_OUTPUT,
        GATE1_OUTPUT,
        GATE2_OUTPUT,
        GATE3_OUTPUT,
        GATE4_OUTPUT,
        GATE5_OUTPUT,
        GATE6_OUTPUT,
        GATE7_OUTPUT,
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
        return std::make_shared<BlankDescription<TBase>>();
    }

    /**
     * Main processing entry point. Called every sample
     */
    void step() override;

    static float base()
    {
        return 3;
    }

    static const int numChannels = NUM_OUTPUTS;
private:

    
    bool lastGate[numChannels] = {false};

};


template <class TBase>
inline void DrumTrigger<TBase>::init()
{
}

template <class TBase>
inline void DrumTrigger<TBase>::step()
{
    // iterator over the 8 input channels we monitor
    for (int i = 0; i < numChannels; ++i) {
        const float cv = inputs[CV_INPUT].voltages[i];
        int index = PitchUtils::cvToSemitone(cv) - 84;
        if (index >= 0 && index < numChannels) {
            // here we have a pitch that we care about
            const bool gInput = inputs[GATE_INPUT].voltages[i] > 5;
            if (gInput != lastGate[index]) {
                lastGate[index] = gInput;
                if (!gInput) {
                    outputs[GATE0_OUTPUT + index].value = 0;
                } else {
                    outputs[GATE0_OUTPUT + index].value = 10;
                }
            }
        }
    }
}

#if 0
template <class TBase>
inline void DrumTrigger<TBase>::step()
{
    for (int i = 0; i < numChannels; ++i) {
        const bool g = inputs[GATE_INPUT].voltages[i] > 5;
        if (g != lastGate[i]) {
            lastGate[i] = g;
            if (!g) {
                outpus[GATE_OUTPUS].volages[i] = 0;         // if gate went low, turn off
            } else {
                const float cv = inputs[CV_INPUT].voltages[i];
                int x = PitchUtils::cvToSemitone(cv - base());
                if (x)
            }
        }
    }
}
#endif

template <class TBase>
int DrumTriggerDescription<TBase>::getNumParams()
{
    return DrumTrigger<TBase>::NUM_PARAMS;
}

template <class TBase>
inline IComposite::Config DrumTriggerDescription<TBase>::getParam(int i)
{
    Config ret(0, 1, 0, "");
    switch (i) {
        case DrumTrigger<TBase>::TEST_PARAM:
            ret = {-1.0f, 1.0f, 0, "Test"};
            break;
        default:
            assert(false);
    }
    return ret;
}


