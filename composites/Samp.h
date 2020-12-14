
#pragma once

#include "Sampler4vx.h"

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
class SampDescription : public IComposite
{
public:
    Config getParam(int i) override;
    int getNumParams() override;
};

template <class TBase>
class Samp : public TBase
{
public:

    Samp(Module * module) : TBase(module)
    {
    }
    Samp() : TBase()
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
        PITCH_INPUT,
        VELOCITY_INPUT,
        GATE_INPUT,
        NUM_INPUTS
    };

    enum OutputIds
    {
        AUDIO_OUTPUT,
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
        return std::make_shared<SampDescription<TBase>>();
    }

    /**
     * Main processing entry point. Called every sample
     */
    //void step() override;
    void process(const typename TBase::ProcessArgs& args) override;

private:

    Sampler4vx playback[4];         // 16 voices of polyphony

    bool lastGate[16];

};


template <class TBase>
inline void Samp<TBase>::init()
{
    for (int i = 0; i<16; ++i) {
        lastGate[i] = false;
    }
}

template <class TBase>
inline void Samp<TBase>::process(const typename TBase::ProcessArgs& args)
{
    // mono, for now
    bool gate = TBase::inputs[GATE_INPUT].value > 1;
    if (gate != lastGate[0]) {
       
        lastGate[0] = gate;
        if (gate) {
            const float pitchCV = TBase::inputs[PITCH_INPUT].value;
            const int midiPitch = 60 + int(std::floor(pitchCV * 12));
           
            // void note_on(int channel, int midiPitch, int midiVVelocity);
            const int midiVelocity = int(TBase::inputs[VELOCITY_INPUT].value * 12);
             printf("input = %f pi=%d v=%d\n", pitchCV, midiPitch, midiVelocity); fflush(stdout);
            playback[0].note_on(0, midiPitch, midiVelocity);
        } else {
            playback[0].note_off(0);
        }

    }
    auto output = playback[0].step();
    TBase::outputs[AUDIO_OUTPUT].setVoltageSimd(output, 0);
}

template <class TBase>
int SampDescription<TBase>::getNumParams()
{
    return Samp<TBase>::NUM_PARAMS;
}

template <class TBase>
inline IComposite::Config SampDescription<TBase>::getParam(int i)
{
    Config ret(0, 1, 0, "");
    switch (i) {
        case Samp<TBase>::TEST_PARAM:
            ret = {-1.0f, 1.0f, 0, "Test"};
            break;
        default:
            assert(false);
    }
    return ret;
}


