
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
        LIGHT0,
        LIGHT1,
        LIGHT2,
        LIGHT3,
        LIGHT4,
        LIGHT5,
        LIGHT6,
        LIGHT7,
        NUM_LIGHTS
    };

    /** Implement IComposite
     */
    static std::shared_ptr<IComposite> getDescription()
    {
        return std::make_shared<DrumTriggerDescription<TBase>>();
    }

    /**
     * Main processing entry point. Called every sample
     */
    void step() override;

    static float base()
    {
        return 0;
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
    printf("\n");
    // iterator over the 8 input channels we monitor
    // Remember: in here 'i' is the input channel,
    // index is the output channel - they are not the same!
    for (int i = 0; i < numChannels; ++i) {
        const float cv = TBase::inputs[CV_INPUT].voltages[i];
        printf("cv = %.2f, semi=%d\n", cv, PitchUtils::cvToSemitone(cv));
        fflush(stdout);
        int index = PitchUtils::cvToSemitone(cv) - 48;
        if (index >= 0 && index < numChannels) {
            // here we have a pitch that we care about
            const bool gInput = TBase::inputs[GATE_INPUT].voltages[i] > 5;
            printf("index=%d i=%d, gInput = %d raw = %f\n", index, i, gInput,
                TBase::inputs[GATE_INPUT].voltages[i]);
            if (gInput != lastGate[index]) {
                lastGate[index] = gInput;
                const float val = gInput ? 10.f : 0.f;
                TBase::outputs[GATE0_OUTPUT + index].value = val;
                TBase::lights[LIGHT0 + index].value = val;
                printf("wrote %.2f to light %d\n", val, index);
            }
        }
    }
}

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


