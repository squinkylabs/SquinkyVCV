
#pragma once

#include <assert.h>

#include <memory>

#include "CompiledInstrument.h"
#include "Divider.h"
#include "IComposite.h"
#include "ManagedPool.h"
#include "SInstrument.h"
#include "Sampler4vx.h"
#include "SimdBlocks.h"
#include "SqPort.h"
#include "ThreadClient.h"
#include "ThreadServer.h"
#include "ThreadSharedState.h"
#include "WaveLoader.h"

namespace rack {
namespace engine {
struct Module;
}
}  // namespace rack
using Module = ::rack::engine::Module;

template <class TBase>
class SampDescription : public IComposite {
public:
    Config getParam(int i) override;
    int getNumParams() override;
};

template <class TBase>
class Samp : public TBase {
public:
    Samp(Module* module) : TBase(module) {
        commonConstruct();
    }
    Samp() : TBase() {
        commonConstruct();
    }
    virtual ~Samp()
    {
        thread.reset();     // kill the threads before deleting other things
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
        PITCH_INPUT,
        VELOCITY_INPUT,
        GATE_INPUT,
        NUM_INPUTS
    };

    enum OutputIds {
        AUDIO_OUTPUT,
        NUM_OUTPUTS
    };

    enum LightIds {
        NUM_LIGHTS
    };

    /** Implement IComposite
     */
    static std::shared_ptr<IComposite> getDescription() {
        return std::make_shared<SampDescription<TBase>>();
    }

    void setNewSamples(const std::string& s) {
        patchRequest = s;
    }

    bool _sampleLoaded() {
        return false;
    }
    /**
     * Main processing entry point. Called every sample
     */
    //void step() override;
    void process(const typename TBase::ProcessArgs& args) override;

private:
    Sampler4vx playback[4];  // 16 voices of polyphony
    SInstrumentPtr instrument;
    WaveLoaderPtr waves;

    float_4 lastGate4[4];
    Divider divn;
    int numChannels_m = 1;

    std::unique_ptr<ThreadClient> thread;
    std::string patchRequest;

    bool lastGate = false;  // just for test now

    void step_n();

    void setupSamplesDummy();
    void commonConstruct();
    void servicePendingPatchRequest();
    void servicePatchLoader();
};

template <class TBase>
inline void Samp<TBase>::init() {
    divn.setup(32, [this]() {
        this->step_n();
    });

    for (int i = 0; i < 4; ++i) {
        lastGate4[i] = float_4(0);
    }
    setupSamplesDummy();
}



#if 0
template <class TBase>
inline void Samp<TBase>::setNewSamples(const std::string& s) {
#ifdef ARCH_WIN
    auto separator = '\\';
#else
    auto separator = '/';
#endif

    auto pos = s.rfind(separator);
    if (pos == std::string::npos) {
        printf("failed to parse path: %s\n", s.c_str());
        fflush(stdout);
        return;
    }

    std::string path = s.substr(0, pos) + separator;
    std::string fname = s.substr(pos + 1);
    printf("path = %s\n", path.c_str());
    printf("name = %s\n", fname.c_str());
    fflush(stdout);
}
#endif

template <class TBase>
inline void Samp<TBase>::setupSamplesDummy() {
    SInstrumentPtr inst = std::make_shared<SInstrument>();

    // tinny piano
    // const char* p = R"foo(D:\samples\UprightPianoKW-small-SFZ-20190703\UprightPianoKW-small-20190703.sfz)foo";
    //  const char* pRoot = R"foo(D:\samples\UprightPianoKW-small-SFZ-20190703\)foo";

    // small piano, with vel keyswitch
    static const char* p = R"foo(D:\samples\K18-Upright-Piano\K18-Upright-Piano.sfz)foo";
    static const char* pRoot = R"foo(D:\samples\K18-Upright-Piano\)foo";

    // snare drum
    // static const char* p =  R"foo(D:\samples\SalamanderDrumkit\snare.sfz)foo";
    // static const char* pRoot =  R"foo(D:\samples\SalamanderDrumkit\)foo";
    auto err = SParse::goFile(p, inst);
    assert(err.empty());

    CompiledInstrumentPtr cinst = CompiledInstrument::make(inst);
    waves = std::make_shared<WaveLoader>();

    cinst->setWaves(waves, pRoot);
    for (int i = 0; i < 4; ++i) {
        playback[i].setPatch(cinst);
    }

    fprintf(stderr, "about load waves\n");
    waves->load();
    fprintf(stderr, "loaded waves\n");
    WaveLoader::WaveInfoPtr info = waves->getInfo(1);
    assert(info->valid);

    for (int i = 0; i < 4; ++i) {
        playback[i].setLoader(waves);
        playback[i].setNumVoices(4);
    }
}

template <class TBase>
inline void Samp<TBase>::step_n() {
    SqInput& inPort = TBase::inputs[PITCH_INPUT];
    SqOutput& outPort = TBase::outputs[AUDIO_OUTPUT];
    numChannels_m = inPort.channels;
    outPort.setChannels(numChannels_m);
    // printf("just set to %d channels\n", numChannels_m); fflush(stdout);

    servicePendingPatchRequest();
    servicePatchLoader();
}

template <class TBase>
inline void Samp<TBase>::process(const typename TBase::ProcessArgs& args) {
    divn.step();

    int numBanks = numChannels_m / 4;
    if (numBanks * 4 < numChannels_m) {
        numBanks++;
    }
    assert(numBanks < 4);
    for (int bank = 0; bank < numBanks; ++bank) {
        // prepare 4 gates. note that ADSR / Sampler4vx must see simd mask (0 or nan)
        // but our logic needs to see numbers (we use 1 and 0).
        Port& p = TBase::inputs[GATE_INPUT];
        float_4 g = p.getVoltageSimd<float_4>(bank * 4);
        float_4 gmask = (g > float_4(1));
        float_4 gate4 = SimdBlocks::ifelse(gmask, float_4(1), float_4(0));
        ;

        float_4 lgate4 = lastGate4[bank];

        for (int iSub = 0; iSub < 4; ++iSub) {
            if (gate4[iSub] != lgate4[iSub]) {
                if (gate4[iSub]) {
                    // printf("new gate on %d:%d gatevalue=%f\n", bank, iSub, gate4[iSub]); fflush(stdout);
                    assert(bank < 4);
                    const int channel = iSub + bank * 4;
                    const float pitchCV = TBase::inputs[PITCH_INPUT].getVoltage(channel);
                    const int midiPitch = 60 + int(std::floor(pitchCV * 12));

                    // printf("raw vel input = %f\n", TBase::inputs[VELOCITY_INPUT].getVoltage(channel));
                    int midiVelocity = int(TBase::inputs[VELOCITY_INPUT].getVoltage(channel) * 12.7f);
                    if (midiVelocity < 1) {
                        midiVelocity = 1;
                    }
                    playback[bank].note_on(iSub, midiPitch, midiVelocity);
                    // printf("send note on to bank %d sub%d pitch %d\n", bank, iSub, midiPitch); fflush(stdout);
                } else {
                    playback[bank].note_off(iSub);
                    // printf("new gate off %d:%d value = %f\n", bank, iSub, gate4[iSub]); fflush(stdout);
                }
            }
        }
        auto output = playback[bank].step(gmask, args.sampleTime);
        TBase::outputs[AUDIO_OUTPUT].setVoltageSimd(output, bank * 4);
        lastGate4[bank] = gate4;
    }
}

template <class TBase>
int SampDescription<TBase>::getNumParams() {
    return Samp<TBase>::NUM_PARAMS;
}

template <class TBase>
inline IComposite::Config SampDescription<TBase>::getParam(int i) {
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


class SampMessage : public ThreadMessage {
public:
    // set by the composite
    std::string patchPath;
    std::string basePath;


    // returned to the composite
    SInstrumentPtr instrument;
    WaveLoaderPtr waves;

};

class SampServer : public ThreadServer {
public:
    SampServer(std::shared_ptr<ThreadSharedState> state) : ThreadServer(state) {
    }

    void handleMessage(ThreadMessage*) override {
        assert(false);
    }
};

template <class TBase>
void Samp<TBase>::commonConstruct() {
    //  crossFader.enableMakeupGain(true);
    std::shared_ptr<ThreadSharedState> threadState = std::make_shared<ThreadSharedState>();
    std::unique_ptr<ThreadServer> server(new SampServer(threadState));

    std::unique_ptr<ThreadClient> client(new ThreadClient(threadState, std::move(server)));
    this->thread = std::move(client);
}


template <class TBase>
void Samp<TBase>::servicePendingPatchRequest() {
    assert(patchRequest.empty());
}

template <class TBase>
void Samp<TBase>::servicePatchLoader() {
    
}
