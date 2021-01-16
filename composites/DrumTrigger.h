
#pragma once

#include <assert.h>

#include <memory>

#include "Divider.h"
#include "IComposite.h"
#include "PitchUtils.h"

namespace rack {
namespace engine {
struct Module;
}
}  // namespace rack

using Module = ::rack::engine::Module;

#define numTriggerChannels 8

template <class TBase>
class DrumTriggerDescription : public IComposite {
public:
    Config getParam(int i) override;
    int getNumParams() override;
};

/**
 * First impl uses 3.5%, so CPU isn't much of an issue
 */
template <class TBase>
class DrumTrigger : public TBase {
public:
    DrumTrigger(Module* module) : TBase(module) {
    }
    DrumTrigger() : TBase() {
    }

    /**
    * re-calc everything that changes with sample
    * rate. Also everything that depends on baseFrequency.
    *
    * Only needs to be called once.
    */
    void init();

    enum ParamIds {
        TEST_PARAM,
        NUM_PARAMS
    };

    enum InputIds {
        CV_INPUT,
        GATE_INPUT,
        NUM_INPUTS
    };

    enum OutputIds {
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

    enum LightIds {
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
    static std::shared_ptr<IComposite> getDescription() {
        return std::make_shared<DrumTriggerDescription<TBase>>();
    }

    /**
     * Main processing entry point. Called every sample
     */
    void step() override;
    void stepn(int n);

    static float base() {
        return 0;
    }

private:
    Divider div;
};

template <class TBase>
inline void DrumTrigger<TBase>::init() {
    div.setup(8, [this] {
        this->stepn(div.getDiv());
    });
}
template <class TBase>
inline void DrumTrigger<TBase>::step() {
    div.step();
}

template <class TBase>
inline void DrumTrigger<TBase>::stepn(int n) {
    // for each input channel, the semitone pitch it is at
    int pitches[16] = {-1};
    bool gates[16] = {false};

    int activeInputs = std::min(numTriggerChannels, int(TBase::inputs[GATE_INPUT].channels));
    activeInputs = std::min(activeInputs, int(TBase::inputs[CV_INPUT].channels));
    for (int i = 0; i < activeInputs; ++i) {
        const float cv = TBase::inputs[CV_INPUT].voltages[i];
        const int index = PitchUtils::cvToSemitone(cv) - 48;

        // TODO: use schmidt
        const bool gInput = TBase::inputs[GATE_INPUT].voltages[i] > 1;
        pitches[i] = index;
        gates[i] = gInput;
    }

    bool gateOutputs[numTriggerChannels] = {false};
    for (int inputChannel = 0; inputChannel < activeInputs; ++inputChannel) {
        const int pitch = pitches[inputChannel];

        // only look at pitches that are in range
        if (pitch >= 0 && pitch < numTriggerChannels) {
            const bool gate = gates[inputChannel];
            if (gate) {
                gateOutputs[pitch] = true;
            }
        }
    }

    for (int outputChannel = 0; outputChannel < numTriggerChannels; ++outputChannel) {
        const bool gate = gateOutputs[outputChannel];

        TBase::outputs[GATE0_OUTPUT + outputChannel].setVoltage(gate ? 10.f : 0.f, 0);
        TBase::lights[LIGHT0 + outputChannel].value = gate ? 10.f : 0.f;
    }
}

template <class TBase>
int DrumTriggerDescription<TBase>::getNumParams() {
    return DrumTrigger<TBase>::NUM_PARAMS;
}

template <class TBase>
inline IComposite::Config DrumTriggerDescription<TBase>::getParam(int i) {
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
