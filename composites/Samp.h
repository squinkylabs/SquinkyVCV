
#pragma once

#include "CompiledInstrument.h"
#include "Divider.h"
#include "Sampler4vx.h"
#include "SqPort.h"
#include "SInstrument.h"
#include "WaveLoader.h"

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
    SInstrumentPtr instrument;
    WaveLoaderPtr waves;

    bool lastGate[16];
    Divider divn;
    int numChannels_m = 1;

    void stepn();

    void setupSamplesDummy();

};


template <class TBase>
inline void Samp<TBase>::init()
{
    divn.setup(32, [this]() {
        this->stepn();
    });

    for (int i = 0; i<16; ++i) {
        lastGate[i] = false;
    }
    setupSamplesDummy();
}

template <class TBase>
inline void Samp<TBase>::setupSamplesDummy()
{
#if 0
    SInstrumentPtr inst = std::make_shared<SInstrument>();
    WaveLoaderPtr w = std::make_shared<WaveLoader>();
    ci::CompiledInstrument cinst(inst);
    //inst->_setTestMode();
#endif
   // const char* p = R"foo(D:\samples\UprightPianoKW-small-SFZ-20190703\samples\C4vH.wav)foo";

    SInstrumentPtr inst = std::make_shared<SInstrument>();
   // const char* p = R"foo(D:\samples\UprightPianoKW-small-SFZ-20190703\UprightPianoKW-small-20190703.sfz)foo";
  //  const char* pRoot = R"foo(D:\samples\UprightPianoKW-small-SFZ-20190703\)foo";

    // small piano
    static char* p =  R"foo(D:\samples\K18-Upright-Piano\K18-Upright-Piano.sfz)foo"; 
    static char* pRoot =  R"foo(D:\samples\K18-Upright-Piano\)foo"; 
    auto err = SParse::goFile(p, inst);
    assert(err.empty());

    ci::CompiledInstrumentPtr cinst = ci::CompiledInstrument::make(inst);
    waves = std::make_shared<WaveLoader>();

   // assert(false);
   // w->load(p);
    cinst->setWaves(waves, pRoot);
    for (int i=0; i<4; ++i) {
        playback[i].setPatch(cinst);
    }

    fprintf(stderr, "about load waves\n");
    waves->load();
    fprintf(stderr, "loaded waves\n");
    WaveLoader::WaveInfoPtr info = waves->getInfo(1);
    assert(info->valid);

    for (int i=0; i<4; ++i) {
        playback[i].setLoader(waves);
        playback[i].setNumVoices(4);
       // playback[i].setLoader(waves);       // why twice?
    }
}


template <class TBase>
inline void Samp<TBase>::stepn()
{
    SqInput& inPort = TBase::inputs[PITCH_INPUT];
    SqOutput& outPort = TBase::outputs[AUDIO_OUTPUT];
    numChannels_m = inPort.channels;
    outPort.setChannels(numChannels_m);
   // printf("just set to %d channels\n", numChannels_m); fflush(stdout);
}

template <class TBase>
inline void Samp<TBase>::process(const typename TBase::ProcessArgs& args)
{
    divn.step();

    int numBanks = numChannels_m / 4;
    if (numBanks * 4 < numChannels_m) {
        numBanks++;
    }

    for (int bank = 0; bank < numBanks; ++bank) {
        for (int iSub=0; iSub<4; ++iSub) {
            const int channel = iSub + bank * 4;
            const bool gate = TBase::inputs[GATE_INPUT].getVoltage(channel) > 1;   
            if (gate != lastGate[channel]) { 
                if (gate) {
                    const float pitchCV = TBase::inputs[PITCH_INPUT].getVoltage(channel);
                    const int midiPitch = 60 + int(std::floor(pitchCV * 12));
            
                    const int midiVelocity = int(TBase::inputs[VELOCITY_INPUT].value * 12);
                    playback[bank].note_on(iSub, midiPitch, midiVelocity);
                    printf("send note on to bank %d sub%d pitch %d\n", bank, iSub, midiPitch); fflush(stdout);
                } else {
                    playback[bank].note_off(iSub);
                } 
                lastGate[channel] = gate;
            } 
        }
        auto output = playback[bank].step();
        TBase::outputs[AUDIO_OUTPUT].setVoltageSimd(output, bank * 4);
    }
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


