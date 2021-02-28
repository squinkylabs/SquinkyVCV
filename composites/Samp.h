
#pragma once

#include <assert.h>

#include <memory>

#include "CompiledInstrument.h"
#include "Divider.h"
#include "IComposite.h"
#include "InstrumentInfo.h"
#include "ManagedPool.h"
#include "SamplerSharedState.h"
#include "SInstrument.h"
#include "Sampler4vx.h"
#include "SamplerErrorContext.h"
#include "SimdBlocks.h"
#include "SqLog.h"
#include "SqPort.h"
#include "ThreadClient.h"
#include "ThreadServer.h"
#include "ThreadSharedState.h"
#include "WaveLoader.h"

#if defined(_MSC_VER)
//   #define ARCH_WIN
#endif

#define _ATOM

namespace rack {
namespace engine {
struct Module;
}
}  // namespace rack
using Module = ::rack::engine::Module;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * This is the message that is passed from Samp's audio thread
 * to Samp's worker thread. Note that it's a two way communication -
 * auto thread is requesting work, and worker thread is returning data
 */
class SampMessage : public ThreadMessage {
public:
    SampMessage() : ThreadMessage(Type::SAMP) {
    }

    /**
    * full path to sfz file from user
    * This is the sfz that will be parsed and loded by the worker
    */
    std::string* pathToSfz;  // 

                             //  std::string pathToSfz;          // full path to sfz file from user
                             //  std::string globalBase;         // aria base path from user
                             //  std::string defaultPath;        // override from the patch

    /**
     * Used in both directions.
     * plugin->server: these are the old values to be disposed of by server.
     * server->plugin: new values from parsed and loaded patch.
     */
    CompiledInstrumentPtr instrument;
    WaveLoaderPtr waves;

    /**
     * A thread safe way to communicate
     * with the other threads
     */
#ifdef _ATOM
    SamplerSharedStatePtr sharedState;
#endif
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
        DUMMYKS_PARAM,
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

    /**
     * Main processing entry point. Called every sample
     */
    void process(const typename TBase::ProcessArgs& args) override;

    /**
     * functions called from the UI thread.
     */
    InstrumentInfoPtr getInstrumentInfo_UI() {
        CompiledInstrumentPtr inst = gcInstrument;
        InstrumentInfoPtr ret;
        if (inst) {
            ret = inst->getInfo();
        }
        return ret;
    }

    void setNewSamples_UI(const std::string& s) {
        std::string* newValue = new std::string(s);
        std::string* oldValue = patchRequestFromUI.exchange(newValue);
        delete oldValue;
    }

    bool isNewInstrument_UI() {
        bool ret = _isNewInstrument.exchange(false);
        return ret;
    }

    void setSamplePath_UI(const std::string& path) {
        SQWARN("Samp::setSamplePath unused");
    }

    void setKeySwitch_UI(int ks) {
        _nextKeySwitchRequest = ks;
    }

    bool _sampleLoaded() {
        return _isSampleLoaded;
    }

    static int quantize(float pitchCV);

private:
    Sampler4vx playback[4];  // 16 voices of polyphony
                             // SInstrumentPtr instrument;
                             // WaveLoaderPtr waves;


    // here we hold onto a reference to these so we can give it back
    // Actually, I think we reference some of these - should consider updating...
    WaveLoaderPtr gcWaveLoader;
    CompiledInstrumentPtr gcInstrument;

#ifdef _ATOM
    SamplerSharedStatePtr sharedState; 
#else
    a b c // just a test, for now
#endif

    float_4 lastGate4[4];
    Divider divn;
    int numChannels_m = 1;

    std::unique_ptr<ThreadClient> thread;

    // sent in on UI thread (should be atomic)
    //std::string patchRequest;
    std::atomic<std::string*> patchRequestFromUI = {nullptr};
    bool _isSampleLoaded = false;
    std::atomic<bool> _isNewInstrument = {false};
    std::atomic<int> _nextKeySwitchRequest = {-1};

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
    void setNewPatch(SampMessage*);
    void serviceSampleReloadRequest();

    // server thread stuff
    // void servicePatchLoader();
};

template <class TBase>
inline void Samp<TBase>::init() {
#ifdef _ATOM
    sharedState  =  std::make_shared<SamplerSharedState>();
#endif
    divn.setup(32, [this]() {
        this->step_n();
    });

    for (int i = 0; i < 4; ++i) {
        lastGate4[i] = float_4(0);
    }
}

// Called when a patch has come back from thread server
template <class TBase>
inline void Samp<TBase>::setNewPatch(SampMessage* newMessage) {
    SQINFO("Samp::setNewPatch (came back from thread server)");
    assert(newMessage);
    if (!newMessage->instrument || !newMessage->waves) {
        if (!newMessage->instrument) {
            SQWARN("Patch Loader could not load patch.");
        } else if (!newMessage->waves) {
            SQWARN("Patch Loader could not load waves.");
        }
        _isSampleLoaded = false;
    } else {
        _isSampleLoaded = true;
    }
    for (int i = 0; i < 4; ++i) {
        playback[i].setPatch(_isSampleLoaded ? newMessage->instrument : nullptr);
        playback[i].setLoader(_isSampleLoaded ? newMessage->waves : nullptr);
        playback[i].setNumVoices(4);
    }

    // even if just for errors, we do have a new "instrument"
    _isNewInstrument = true;
    SQINFO("Samp::setNewPatch _isNewInstrument");
    this->gcWaveLoader = newMessage->waves;
    this->gcInstrument = newMessage->instrument;

    // We have taken over ownership. This should be non-blocking
    newMessage->waves.reset();
    newMessage->instrument.reset();
}

template <class TBase>
inline void Samp<TBase>::step_n() {
    SqInput& inPort = TBase::inputs[PITCH_INPUT];
    SqOutput& outPort = TBase::outputs[AUDIO_OUTPUT];
    numChannels_m = inPort.channels;
    outPort.setChannels(numChannels_m);
    servicePendingPatchRequest();
    serviceMessagesReturnedToComposite();
    serviceSampleReloadRequest();

    if (_nextKeySwitchRequest >= 1) {
        int midiPitch = _nextKeySwitchRequest;
        _nextKeySwitchRequest = -1;
        // we can send it to any sampler: ks is global
        // can also use fake vel and fake sr
        playback[0].note_on(0, midiPitch, 64, 44100.f);

        // don't do this anymore, now that we have ADSR
        // playback[0].note_off(0);
    }

}

template <class TBase>
inline void  Samp<TBase>::serviceSampleReloadRequest() {
    #ifdef _ATOM
    if (sharedState->au_isSampleReloadRequested()) {
        for (int i=0; i<4; ++i) {
            playback[i].clearSamples();
        }
        sharedState->au_grantSampleReloadRequest();
    }
    #endif
}

template <class TBase>
inline int Samp<TBase>::quantize(float pitchCV) {
    const int midiPitch = 60 + int(std::floor(.5 + pitchCV * 12));
    return midiPitch;
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
        float_4 gate4 = SimdBlocks::ifelse(gmask, float_4(1), float_4(0));      // isn't this pointless?
        float_4 lgate4 = lastGate4[bank];

        if (bank == 0) {
      //      printf("samp, g4 = %s\n", toStr(gate4).c_str());
        }
        for (int iSub = 0; iSub < 4; ++iSub) {
            if (gate4[iSub] != lgate4[iSub]) {
                if (gate4[iSub]) {
                    assert(bank < 4);
                    const int channel = iSub + bank * 4;
                    const float pitchCV = TBase::inputs[PITCH_INPUT].getVoltage(channel);
                    //  const int midiPitch = 60 + int(std::floor(pitchCV * 12));
                    const int midiPitch = quantize(pitchCV);

                    // if velocity not patched, use 64
                    int midiVelocity = 64;
                    if (TBase::inputs[VELOCITY_INPUT].isConnected()) {
                        // if it's mono, just get first chan. otherwise get poly
                        midiVelocity = int(TBase::inputs[VELOCITY_INPUT].getPolyVoltage(channel) * 12.7f);
                        if (midiVelocity < 1) {
                            midiVelocity = 1;
                        }
                    }
#if 0
                    SQINFO("");
                    SQINFO("new gate on %d:%d gatevalue=%f\n", bank, iSub, gate4[iSub]);
                    SQINFO("  pitchcv = %f, midipitch = %d", pitchCV, midiPitch);
#endif
                    playback[bank].note_on(iSub, midiPitch, midiVelocity, args.sampleRate);
                    // printf("send note on to bank %d sub%d pitch %d\n", bank, iSub, midiPitch); fflush(stdout);
                } else {
#ifndef _USEADSR
                    playback[bank].note_off(iSub);
                    // printf("new gate off %d:%d value = %f\n", bank, iSub, gate4[iSub]); fflush(stdout);
#endif
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
        case Samp<TBase>::DUMMYKS_PARAM:
            ret = {-1, 127, -1, "Key Switch"};
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

    // This handle is called when the worker thread (ThreadServer)
    // gets a message. This is the handler for that message
    void handleMessage(ThreadMessage* msg) override {

        // Since Samp only uses one type of message, we can
        // trivailly down-cast to the particular message type
        assert(msg->type == ThreadMessage::Type::SAMP);
        SampMessage* smsg = static_cast<SampMessage*>(msg);

#ifdef _ATOM
        SQINFO("worker about to wait for sample access");
        assert(smsg->sharedState);
        smsg->sharedState->uiw_requestAndWaitForSampleReload();
        SQINFO("worker got sample access");
#endif

        // First thing we do it throw away the old patch data.
        // We couldn't do that on the audio thread, since mem allocation will block the thread
        smsg->waves.reset();
        smsg->instrument.reset();

        parsePath(smsg);

        SInstrumentPtr inst = std::make_shared<SInstrument>();

        // now load it, and then return it.
        auto err = SParse::goFile(fullPath.toString(), inst);
        if (!err.empty()) {
            sendMessageToClient(msg);
            return;
        };

        // TODO: need a way for compiler to return error;
        SamplerErrorContext errc;
        CompiledInstrumentPtr cinst = CompiledInstrument::make(errc, inst);
        errc.dump();
        if (!cinst) {
            sendMessageToClient(msg);
            return;
        }
        WaveLoaderPtr waves = std::make_shared<WaveLoader>();

        //  samplePath += cinst->getDefaultPath();
        samplePath.concat(cinst->getDefaultPath());
        SQINFO("calling setWaves on %s", samplePath.toString().c_str());
        cinst->setWaves(waves, samplePath);
        // TODO: errors from wave loader

        // TODO: need a way for wave loader to return error/
        const bool loadedOK = waves->load();

        smsg->instrument = cinst;
        smsg->waves = loadedOK ? waves : nullptr;
        auto info = cinst->getInfo();

        SQINFO("samp thread back, info error = %s", info->errorMessage.c_str());
        if (info->errorMessage.empty() && !loadedOK) {
            // SQINFO("main error empty");
            // SQINFO("error from ")
            info->errorMessage = waves->lastError;
            SQINFO("returning error message %s", info->errorMessage.c_str());
        }
        //   a b // return error now.
        SQINFO("** loader thread returning %d", loadedOK);

        sendMessageToClient(msg);
    }

private:
    FilePath samplePath;
    //  std::string fullPath;
    //  std::string globalPath;
    FilePath fullPath;  // what gets passed in

    /** parse out the original file location and other info
     * to find the path to the folder containing the samples.
     * this path will then be used to locate all samples.
     */
    void parsePath(SampMessage* msg) {
        SQINFO("parse path 348");
        if (msg->pathToSfz) {
            // maybe we should allow raw strings to come in this way. but it's probably fine
            fullPath = FilePath(*(msg->pathToSfz));
            SQINFO("parse path 351 %s", fullPath.toString());
            SQINFO("about to delete %p", msg->pathToSfz);
            delete msg->pathToSfz;
            msg->pathToSfz = nullptr;
            SQINFO("parse path 354");
        }
#if 0  // when we add this back
        if (!msg->defaultPath.empty()) {
            SQWARN("ignoring patch def = %s", msg->defaultPath.c_str());
        }
#endif

        //FilePath fullPath()
        samplePath = fullPath.getPathPart();

        // If the patch had a path, add that
        //   samplePath += cinst->getDefaultPath();
        //   SQINFO("after def sample base path %s", samplePath.c_str());
        //     std::string composedPath = samplePath
        //  SQINFO("about to set waves to %s. default = %s global = %s\n", samplePath.c_str(), cinst->getDefaultPath().c_str(), globalPath.c_str());
        //if (!cinst->defaultPath())
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
    if (!patchRequestFromUI) {
        return;
    }

    if (messagePool.empty()) {
        SQWARN("enable to request new patch will del");
        auto value = patchRequestFromUI.exchange(nullptr);
        delete value;
        //patchRequest.clear();
        return;
    }

    // OK, we are ready to send a message!
    SampMessage* msg = messagePool.pop();

#ifdef _ATOM
    msg->sharedState = sharedState;
#endif
    msg->pathToSfz = patchRequestFromUI;
    msg->instrument = this->gcInstrument;
    msg->waves = this->gcWaveLoader;

    // Now that we have put together our patch request,
    // and memory deletion request, we can let go
    // of shared resources that we hold.

    // we have passed ownership from Samp to message. So clear
    // out the value in Samp, but don't delete it
    patchRequestFromUI.exchange(nullptr);

    gcInstrument.reset();
    gcWaveLoader.reset();
    for (int i = 0; i < 4; ++i) {
        playback[i].setPatch(nullptr);
        playback[i].setLoader(nullptr);
    }

    bool sent = thread->sendMessage(msg);
    if (sent) {
    } else {
        WARN("Unable to sent message to server.");
        messagePool.push(msg);
    }
}

template <class TBase>
void Samp<TBase>::serviceMessagesReturnedToComposite() {
    // see if any messages came back for us
    ThreadMessage* newMsg = thread->getMessage();
    if (newMsg) {
        assert(newMsg->type == ThreadMessage::Type::SAMP);
        SampMessage* smsg = static_cast<SampMessage*>(newMsg);
        setNewPatch(smsg);
        messagePool.push(smsg);
    }
}
