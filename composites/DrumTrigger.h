
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

#define numTriggerChannels 8

template <class TBase>
class DrumTriggerDescription : public IComposite
{
public:
    Config getParam(int i) override;
    int getNumParams() override;
};

/**
 * First impl uses 3.5%, so CPU isn't much of an issue
 */
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

private:

    class OutputChannelState
    {
    public:
        bool gate = false;
        int inputChannel = 0;
        bool gateThisStep = false;      // only true if the gate was set in the latest step call
    };

    // for a given OUTPUT channel:
    //  gate indicated it that output gate was high
    //  inputChannel indicates which input made gate high
    OutputChannelState state[numTriggerChannels];

    void processInput(int ouputChannel, bool gate, int inputChannel);
};


template <class TBase>
inline void DrumTrigger<TBase>::init()
{
}

// Q: what is the cireteria for turning off a "dead" channel?
//          the channel ouput must be high.
//          but the input channel that made it high now has a different pitch than before,
//          so it is assigned to a different output channel
// one way to imp would be to keep track if which channels were set this call to step(),
// and any outputs that are still high that shouldn't be can get cleared.
template <class TBase>
inline void DrumTrigger<TBase>::processInput(int outputChannel, bool gate, int inputChannel)
{
    if (gate) {
        // remmember if someone found the gate high for this output channel
        state[outputChannel].gateThisStep = gate;
    }
   // fprintf(stderr, "set state[%d].gateThisStep = %d\n", outputChannel, gate);
    if (gate) {
        // gate low to high at this output's pitch,
        // lets raise the gate.
        //    (but we need to see if this the ..,
        if (!state[outputChannel].gate) {
            TBase::outputs[GATE0_OUTPUT + outputChannel].value = 10;
            TBase::lights[LIGHT0 + outputChannel].value = 10;
            state[outputChannel].gate = true;

            // Remember which input made the gate go high.
            // Only that one can turn it off
            state[outputChannel].inputChannel = inputChannel;
        }
    } else {
        if (state[outputChannel].gate && state[outputChannel].inputChannel == inputChannel) {

            // If this output is high, and the input that made it 
            // high is now low, then go low.
            TBase::outputs[GATE0_OUTPUT + outputChannel].value = 0;
            TBase::lights[LIGHT0 + outputChannel].value = 0;

            // TODO: shouldn't this be outputChannel?
            state[inputChannel].gate = false;
           // state[outputChannel].gate = false;
        }
    }
}

template <class TBase>
inline void DrumTrigger<TBase>::step()
{
    for (int i = 0; i < numTriggerChannels; ++i) {
        state[i].gateThisStep = false;
    }
    // iterator over the 8 input channels we monitor
    // Remember: in here 'i' is the input channel,
    // index is the output channel - they are not the same!

    int activeInputs = std::min(numTriggerChannels, int(TBase::inputs[GATE_INPUT].channels));
    activeInputs = std::min(activeInputs, int(TBase::inputs[CV_INPUT].channels));
    for (int i = 0; i < activeInputs; ++i) {
        const float cv = TBase::inputs[CV_INPUT].voltages[i];
        int index = PitchUtils::cvToSemitone(cv) - 48;
        if (index >= 0 && index < numTriggerChannels) {
             // here we have a pitch that we care about
            const bool gInput = TBase::inputs[GATE_INPUT].voltages[i] > 5;
            processInput(index, gInput, i);
        }
    }

    for (int i = 0; i < numTriggerChannels; ++i) {
        if (state[i].gate && !state[i].gateThisStep) {
          //  fprintf(stderr, "new logic kicking in on channel %d\n", i);
            //assert(false);

            // break this out (???)
            TBase::outputs[GATE0_OUTPUT + i].value = 0;
            TBase::lights[LIGHT0 + i].value = 0;
            state[i].gate = false;
        }
    }
}

#if 0 // orig
template <class TBase>
inline void DrumTrigger<TBase>::step()
{
    // iterator over the 8 input channels we monitor
    // Remember: in here 'i' is the input channel,
    // index is the output channel - they are not the same!

    int activeInputs = std::min(numTriggerChannels, int(TBase::inputs[GATE_INPUT].channels));
    activeInputs = std::min(activeInputs, int(TBase::inputs[CV_INPUT].channels));
    for (int i = 0; i < activeInputs; ++i) {
        const float cv = TBase::inputs[CV_INPUT].voltages[i];
        int index = PitchUtils::cvToSemitone(cv) - 48;
        if (index >= 0 && index < numTriggerChannels) {
            // here we have a pitch that we care about
            const bool gInput = TBase::inputs[GATE_INPUT].voltages[i] > 5;
            if (gInput) {
                // gate low to high at this output's pitch,
                // lets raise the gate.
                if (!state[index].gate) {
                    TBase::outputs[GATE0_OUTPUT + index].value = 10;
                    TBase::lights[LIGHT0 + index].value = 10;
                    state[index].gate = true;

                    // Remember which input made the gate go high.
                    // Only that one can turn it off
                    state[index].inputChannel = i;
                }
            } else {
                if (state[index].gate && state[index].inputChannel == i) {

                    // If this output is high, and the input that made it 
                    // high is now low, then go low.
                    TBase::outputs[GATE0_OUTPUT + index].value = 0;
                    TBase::lights[LIGHT0 + index].value = 0;
                    state[index].gate = false;
                }
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


