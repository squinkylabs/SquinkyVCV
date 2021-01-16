
#pragma once

#include <assert.h>

#include <memory>

#include "SqPort.h"

#include "CompiledInstrument.h"
#include "Divider.h"
#include "IComposite.h"
#include "ManagedPool.h"
#include "SInstrument.h"
#include "Sampler4vx.h"
#include "SimdBlocks.h"
#include "SqLog.h"

#include "ThreadClient.h"
#include "ThreadServer.h"
#include "ThreadSharedState.h"
#include "WaveLoader.h"

#if defined(_MSC_VER)
//   #define ARCH_WIN
#endif

namespace rack {
namespace engine {
struct Module;
}
}  // namespace rack
using Module = ::rack::engine::Module;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class SampMessage : public ThreadMessage {
public:
    SampMessage() : ThreadMessage(Type::SAMP) {
    }

    std::string pathToSfz;

    // returned to the composite
    CompiledInstrumentPtr instrument;
    WaveLoaderPtr waves;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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

    virtual ~Samp() {
        thread.reset();  // kill the threads before deleting other things
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
        return _isSampleLoaded;
    }
    /**
     * Main processing entry point. Called every sample
     */
    //void step() override;
    void process(const typename TBase::ProcessArgs& args) override;

private:
    Sampler4vx playback[4];  // 16 voices of polyphony
                             // SInstrumentPtr instrument;
                             // WaveLoaderPtr waves;
    SampMessage* currentPatchMessage = nullptr;

    float_4 lastGate4[4];
    Divider divn;
    int numChannels_m = 1;

    std::unique_ptr<ThreadClient> thread;
    std::string patchRequest;
    bool _isSampleLoaded = false;

    bool lastGate = false;  // just for test now

    /**
     * Messages moved between thread, messagePool, and crossFader
     * as new noise slopes are requested in response to CV/knob changes.
     */
    ManagedPool<SampMessage, 2> messagePool;

    void step_n();

    // void setupSamplesDummy();
    void commonConstruct();
    void servicePendingPatchRequest();
    void serviceMessagesReturnedToComposite();
    void setNewPatch();

    // server thread stuff
    // void servicePatchLoader();
};

template <class TBase>
inline void Samp<TBase>::init() {
    divn.setup(32, [this]() {
        this->step_n();
    });

    for (int i = 0; i < 4; ++i) {
        lastGate4[i] = float_4(0);
    }
    //  setupSamplesDummy();
}


// Called when a patch has come back from thread server
template <class TBase>
inline void Samp<TBase>::setNewPatch() {
    assert(currentPatchMessage);
    if (!currentPatchMessage->instrument || !currentPatchMessage->waves) {
        SQWARN("Patch Loader could not load path");
        _isSampleLoaded = false;
        return;
    }
    for (int i = 0; i < 4; ++i) {
        playback[i].setPatch(currentPatchMessage->instrument);
        playback[i].setLoader(currentPatchMessage->waves);
        playback[i].setNumVoices(4);
    }
    _isSampleLoaded = true;
}

template <class TBase>
inline void Samp<TBase>::step_n() {
    SqInput& inPort = TBase::inputs[PITCH_INPUT];
    SqOutput& outPort = TBase::outputs[AUDIO_OUTPUT];
    numChannels_m = inPort.channels;
    outPort.setChannels(numChannels_m);
    servicePendingPatchRequest();
    serviceMessagesReturnedToComposite();
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
                    playback[bank].note_on(iSub, midiPitch, midiVelocity, args.sampleRate);
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

///////////////////////////////////////////////////////////////////////////////////////////////////////////

class SampServer : public ThreadServer {
public:
    SampServer(std::shared_ptr<ThreadSharedState> state) : ThreadServer(state) {
    }

    void handleMessage(ThreadMessage* msg) override {
        assert(msg->type == ThreadMessage::Type::SAMP);
        SampMessage* smsg = static_cast<SampMessage*>(msg);
        // SQINFO("server got a message!\n");
        parsePath(smsg);

        SInstrumentPtr inst = std::make_shared<SInstrument>();

        // now load it, and then return it.
        auto err = SParse::goFile(fullPath.c_str(), inst);
        if (!err.empty()) {
            SQWARN("parsing error in sfz: %s\n", err.c_str());
            sendMessageToClient(msg);
            return;
        }

        // TODO: need a way for compiler to return error;
        CompiledInstrumentPtr cinst = CompiledInstrument::make(inst);
        WaveLoaderPtr waves = std::make_shared<WaveLoader>();

        cinst->setWaves(waves, samplePath);

        // TODO: need a way for wave loader to return error/
        waves->load();
        WaveLoader::WaveInfoPtr info = waves->getInfo(1);
        assert(info->valid);

        smsg->instrument = cinst;
        smsg->waves = waves;
        SQINFO("loader thread returning happy");

        sendMessageToClient(msg);
    }

private:
    std::string samplePath;
    std::string fullPath;

    void parsePath(SampMessage* msg) {
        fullPath = msg->pathToSfz;
        WaveLoader::makeAllSeparatorsNative(fullPath);

        const auto pos = fullPath.rfind(WaveLoader::nativeSeparator());
        if (pos == std::string::npos) {
            SQWARN("failed to parse path to samples: %s\n", fullPath.c_str());
            fflush(stdout);
            return;
        }

        samplePath = fullPath.substr(0, pos) + WaveLoader::nativeSeparator();
        SQINFO("sample base path %s", samplePath.c_str());
    }
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////

template <class TBase>
void Samp<TBase>::commonConstruct() {
    //  crossFader.enableMakeupGain(true);
    std::shared_ptr<ThreadSharedState> threadState = std::make_shared<ThreadSharedState>();
    std::unique_ptr<ThreadServer> server(new SampServer(threadState));

    std::unique_ptr<ThreadClient> client(new ThreadClient(threadState, std::move(server)));
    this->thread = std::move(client);
};

template <class TBase>
void Samp<TBase>::servicePendingPatchRequest() {
    if (patchRequest.empty()) {
        return;
    }

    if (messagePool.empty()) {
        SQWARN("message pool empty at 346");
        return;
    }

    // OK, we are ready to send a message!
    SampMessage* msg = messagePool.pop();
    msg->pathToSfz = this->patchRequest;
    patchRequest.clear();

    bool sent = thread->sendMessage(msg);
    if (sent) {
    } else {
        WARN("Unable to sent message to server.");
        messagePool.push(msg);
    }
    SQINFO("send message to server");
}
template <class TBase>
void Samp<TBase>::serviceMessagesReturnedToComposite() {
    // see if any messages came back for us
    ThreadMessage* newMsg = thread->getMessage();
    if (newMsg) {
        assert(newMsg->type == ThreadMessage::Type::SAMP);
        SampMessage* smsg = static_cast<SampMessage*>(newMsg);
        if (currentPatchMessage) {
            messagePool.push(currentPatchMessage);
            fflush(stderr); fflush(stdout);
        }
        currentPatchMessage = smsg;        
        setNewPatch();
    }
}

