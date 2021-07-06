
#pragma once

#include <assert.h>

#include <memory>

#include "CompiledInstrument.h"
#include "Divider.h"
#include "GateDelay.h"
#include "IComposite.h"
#include "InstrumentInfo.h"
#include "LookupTable.h"
#include "ManagedPool.h"
#include "ObjectCache.h"
#include "SInstrument.h"
#include "Sampler4vx.h"
#include "SamplerErrorContext.h"
#include "SamplerSharedState.h"
#include "SimdBlocks.h"
#include "SqLog.h"
#include "SqPort.h"
#include "SqSchmidtTrigger.h"
#include "ThreadClient.h"
#include "ThreadServer.h"
#include "ThreadSharedState.h"
#include "WaveLoader.h"

#if defined(_MSC_VER)
//   #define ARCH_WIN
#endif

#define _ATOM  // use atomic operations.
#define _KS2   // new key-switch with params

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

/*
 * ver2, with delay
 * 16 NOTES         42.9
 * 4 notes mod      13.8
 * 4 notes no mod   12.7
 *
 * ver 1
 * 16 NOTES         35.4
 * 4 notes mod      10.3
 * 4 notes no mod   9.9
 * 
 * 
 */

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
        DUMMYKS_PARAM,  // need to make this real
#ifdef _SAMPFM
        PITCH_PARAM,
        PITCH_TRIM_PARAM,
        LFM_DEPTH_PARAM,
#endif
        VOLUME_PARAM,
        SCHEMA_PARAM,
        TRIGGERDELAY_PARAM,
        OCTAVE_PARAM,
        NUM_PARAMS
    };

    enum InputIds {
        PITCH_INPUT,
        VELOCITY_INPUT,
        GATE_INPUT,
        FM_INPUT,
        LFM_INPUT,
        LFMDEPTH_INPUT,
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

#ifndef _KS2
    void setKeySwitch_UI(int ks) {
        _nextKeySwitchRequest = ks;
    }
#endif

    bool _sampleLoaded() {
        return _isSampleLoaded;
    }

    float getProgressPct() const;

    static int quantize(float pitchCV);

    float _getTranspose(int voice) const;

    void _setupPerfTest();

    void suppressErrors();

private:
    Sampler4vx playback[4];  // 16 voices of polyphony
                             // SInstrumentPtr instrument;
                             // WaveLoaderPtr waves;
    GateDelay<5> gateDelays;
    SqSchmittTrigger trig[4];

    // here we hold onto a reference to these so we can give it back
    // Actually, I think we reference some of these - should consider updating...
    WaveLoaderPtr gcWaveLoader;
    CompiledInstrumentPtr gcInstrument;

#ifdef _ATOM
    SamplerSharedStatePtr sharedState;
#else
    a b c  // just a test, for now
#endif

    // Variables updated every 'n' samples
    int numChannels_n = 1;
    int numBanks_n = 1;
    bool lfmConnected_n = false;
    bool triggerDelayEnabled_n = true;
    float_4 lfmGain_n = {0};
    float rawVolume_n = 0;
    float taperedVolume_n = 0;

    float_4 lastGate4[4];
    Divider divn;

    // I think this goes ins sSampler4vx
    //  std::function<float(float)> expLookup = ObjectCache<float>::getExp2Ex();
    std::shared_ptr<LookupTableParams<float>> audioTaperLookupParams = ObjectCache<float>::getAudioTaper();
    std::shared_ptr<LookupTableParams<float>> bipolarAudioTaperLookupParams = ObjectCache<float>::getBipolarAudioTaper42();

    std::unique_ptr<ThreadClient> thread;

    // sent in on UI thread (should be atomic)
    std::atomic<std::string*> patchRequestFromUI = {nullptr};
    bool _isSampleLoaded = false;
    std::atomic<bool> _isNewInstrument = {false};

#ifdef _KS2
    int lastServicedKeyswitchValue = {-1};
#else
    std::atomic<int> _nextKeySwitchRequest = {-1};
#endif

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
    void serviceKeySwitch();
    void serviceFMMod();

    void updateKeySwitch(int midiPitch);
    // server thread stuff
    // void servicePatchLoader();
};

template <class TBase>
inline void Samp<TBase>::init() {
#ifdef _ATOM
    sharedState = std::make_shared<SamplerSharedState>();
#endif
    divn.setup(32, [this]() {
        this->step_n();
    });

    for (int i = 0; i < 4; ++i) {
        lastGate4[i] = float_4(0);
        playback[i].setIndex(i);
    }
}

template <class TBase>
inline float Samp<TBase>::getProgressPct() const {
    return sharedState->uiw_getProgressPercent();
}

template <class TBase>
inline void Samp<TBase>::suppressErrors() {
    playback[0].suppressErrors();
    playback[1].suppressErrors();
    playback[2].suppressErrors();
    playback[3].suppressErrors();
}

template <class TBase>
inline void Samp<TBase>::_setupPerfTest() {
    SInstrumentPtr inst = std::make_shared<SInstrument>();

    SamplerErrorContext errc;
    CompiledInstrumentPtr cinst = CompiledInstrument::make(errc, inst);
    WaveLoaderPtr w = std::make_shared<WaveLoader>();
    w->_setTestMode(WaveLoader::Tests::DCTenSec);

    cinst->_setTestMode(CompiledInstrument::Tests::MiddleC11);  // I don't know what this test mode does now, but probably not enough?

    SampMessage sm;
    sm.instrument = cinst;
    sm.waves = w;
    setNewPatch(&sm);
}

// Called when a patch has come back from thread server
template <class TBase>
inline void Samp<TBase>::setNewPatch(SampMessage* newMessage) {
    // SQINFO("Samp::setNewPatch (came back from thread server)");
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
    // SQINFO("Samp::setNewPatch _isNewInstrument");
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

    numChannels_n = inPort.channels;
    outPort.setChannels(numChannels_n);
    numBanks_n = numChannels_n / 4;
    if (numBanks_n * 4 < numChannels_n) {
        numBanks_n++;
    }

    float newVolume_n = TBase::params[VOLUME_PARAM].value;
    if (newVolume_n != rawVolume_n) {
        rawVolume_n = newVolume_n;
        taperedVolume_n = 10 * LookupTable<float>::lookup(*audioTaperLookupParams, rawVolume_n / 100);
    }

    triggerDelayEnabled_n = TBase::params[TRIGGERDELAY_PARAM].value > .5;

    servicePendingPatchRequest();
    serviceMessagesReturnedToComposite();
    serviceSampleReloadRequest();
    serviceKeySwitch();
    serviceFMMod();
}

#ifdef _KS2
template <class TBase>
inline void Samp<TBase>::serviceKeySwitch() {
    const int val = int(std::round(TBase::params[DUMMYKS_PARAM].value));
    if (val != lastServicedKeyswitchValue) {
        // SQINFO("comp saw ks param change from %d to %d", lastServicedKeyswitchValue, val);
        lastServicedKeyswitchValue = val;
        playback[0].note_on(0, val, 64, 44100.f);
    }
}

template <class TBase>
inline void Samp<TBase>::updateKeySwitch(int midiPitch) {
    TBase::params[DUMMYKS_PARAM].value = float(midiPitch);
}
#else
template <class TBase>
inline void Samp<TBase>::serviceKeySwitch() {
    if (_nextKeySwitchRequest >= 1) {
        int midiPitch = _nextKeySwitchRequest;
        _nextKeySwitchRequest = -1;
        // we can send it to any sampler: ks is global
        // can also use fake vel and fake sr
        playback[0].note_on(0, midiPitch, 64, 44100.f);
    }
}
#endif

template <class TBase>
inline void Samp<TBase>::serviceFMMod() {
    //------------------- first do the linear FM
    lfmConnected_n = TBase::inputs[LFM_INPUT].isConnected();

    // lfm gain = audio_taper( knob / 10);
    float depth = TBase::params[LFM_DEPTH_PARAM].value;
    depth /= 10.f;  // scale it to 0..1
    depth = LookupTable<float>::lookup(*audioTaperLookupParams, depth);
    lfmGain_n = float_4(depth);  // store as a float_4, since that's what we want in process();

    //------------------ now Exp FM -----------
    // this one is -5 to +5
    const float_4 expPitchOffset = TBase::params[PITCH_PARAM].value;

    // this one is -1 to +1
    const float pitchCVTrimRaw = TBase::params[PITCH_TRIM_PARAM].value;
    const float scaledPitchCVTrim = LookupTable<float>::lookup(*bipolarAudioTaperLookupParams, pitchCVTrimRaw);
    const float_4 pitchCVTrim(scaledPitchCVTrim);
    Port& fmInput = TBase::inputs[FM_INPUT];
    for (int bank = 0; bank < numBanks_n; ++bank) {
        float_4 rawInput = fmInput.getPolyVoltageSimd<float_4>(bank * 4);
        float_4 scaledInput = rawInput * pitchCVTrim;
        float_4 finalBankFM = scaledInput + expPitchOffset;
        playback[bank].setExpFM(finalBankFM);
#if 0
        if (bank == 0) {
            float_4 x = fmInput.getVoltageSimd<float_4>(bank * 4);
            SQINFO("p0=%f, trim=%f cv=%f x=%f res=%f", expPitchOffset[0], expPitchCVTrim[0], rawInput[0], x[0], finalBankFM[0]);
            SQINFO("  simdin=%s", toStr(rawInput).c_str());
            
        }
#endif
    }
}

template <class TBase>
inline float Samp<TBase>::_getTranspose(int voice) const {
    const int bank = voice / 4;
    const int subChannel = voice - bank * 4;
    return playback[bank]._transAmt(subChannel);
}

template <class TBase>
inline void Samp<TBase>::serviceSampleReloadRequest() {
#ifdef _ATOM
    if (sharedState->au_isSampleReloadRequested()) {
        for (int i = 0; i < 4; ++i) {
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

#if 1  // new version with gate delat
template <class TBase>
inline void Samp<TBase>::process(const typename TBase::ProcessArgs& args) {
    //   SQINFO("pin");
    divn.step();

    // is there some "off by one error" here?
    assert(numBanks_n <= 4);

    // Loop for all channels. Does gate detections, runs audio
    for (int bank = 0; bank < numBanks_n; ++bank) {
        // Step 1: gate processing. This doesn't have to run every sample, btw.
        // prepare 4 gates. note that ADSR / Sampler4vx must see simd mask (0 or nan)
        // but our logic needs to see numbers (we use 1 and 0).
        Port& pGate = TBase::inputs[GATE_INPUT];
        float_4 g = pGate.getVoltageSimd<float_4>(bank * 4);
        float_4 gmaskIn = trig[bank].process(g);
        float_4 gmaskOut;
        if (triggerDelayEnabled_n) {
            simd_assertMask(gmaskIn);
            gateDelays.addGates(gmaskIn);
            gmaskOut = gateDelays.getGates();
        } else {
            gmaskOut = gmaskIn;
        }
        // float_4 gmask = (g > float_4(1));
        float_4 gate4 = SimdBlocks::ifelse(gmaskOut, float_4(1), float_4(0));  // isn't this pointless?
        float_4 lgate4 = lastGate4[bank];

        if (bank == 0) {
            //      printf("samp, g4 = %s\n", toStr(gate4).c_str());
        }

        // main loop that processes gates and runs audio
        for (int iSub = 0; iSub < 4; ++iSub) {
            if (gate4[iSub] != lgate4[iSub]) {
                if (gate4[iSub]) {
                    assert(bank < 4);
                    const int channel = iSub + bank * 4;
                    const float pitchCV = TBase::inputs[PITCH_INPUT].getVoltage(channel);
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
                    const bool isKs = playback[bank].note_on(iSub, midiPitch, midiVelocity, args.sampleRate);
                    if (isKs) {
                        updateKeySwitch(midiPitch);
                    }
                    // printf("send note on to bank %d sub%d pitch %d\n", bank, iSub, midiPitch); fflush(stdout);
                }
            }
        }

        // Step 2: LFM processing
        float_4 fm = float_4::zero();
        if (lfmConnected_n) {
            Port& pIn = TBase::inputs[LFM_INPUT];
            Port& pDepth = TBase::inputs[LFMDEPTH_INPUT];
            float_4 depth = pDepth.isConnected() ? pDepth.getPolyVoltageSimd<float_4>(bank * 4) : 10.f;
            depth *= .1f;
            float_4 rawInput = pIn.getPolyVoltageSimd<float_4>(bank * 4);
            fm = rawInput * lfmGain_n * depth;
            // SQINFO("read fm=%s, raw=%s", toStr(fm).c_str(), toStr(rawInput).c_str());
        }

        // Step 3: run the audio
        auto output = playback[bank].step(gmaskOut, args.sampleTime, fm, lfmConnected_n);
        output *= taperedVolume_n;
        TBase::outputs[AUDIO_OUTPUT].setVoltageSimd(output, bank * 4);
        lastGate4[bank] = gate4;
    }
    gateDelays.commit();
}

#else  // Original version here

template <class TBase>
inline void Samp<TBase>::process(const typename TBase::ProcessArgs& args) {
    //   SQINFO("pin");
    divn.step();

    // is there some "off by one error" here?
    assert(numBanks_n <= 4);

    // Loop for all channels. Does gate detections, runs audio
    for (int bank = 0; bank < numBanks_n; ++bank) {
        // Step 1: gate processing. This doesn't have to run every sample, btw.
        // prepare 4 gates. note that ADSR / Sampler4vx must see simd mask (0 or nan)
        // but our logic needs to see numbers (we use 1 and 0).
        Port& pGate = TBase::inputs[GATE_INPUT];
        float_4 g = pGate.getVoltageSimd<float_4>(bank * 4);
        float_4 gmask = (g > float_4(1));
        float_4 gate4 = SimdBlocks::ifelse(gmask, float_4(1), float_4(0));  // isn't this pointless?
        float_4 lgate4 = lastGate4[bank];

        if (bank == 0) {
            //      printf("samp, g4 = %s\n", toStr(gate4).c_str());
        }

        // main loop that processes gates and runs audio
        for (int iSub = 0; iSub < 4; ++iSub) {
            if (gate4[iSub] != lgate4[iSub]) {
                if (gate4[iSub]) {
                    assert(bank < 4);
                    const int channel = iSub + bank * 4;
                    const float pitchCV = TBase::inputs[PITCH_INPUT].getVoltage(channel);
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
                    const bool isKs = playback[bank].note_on(iSub, midiPitch, midiVelocity, args.sampleRate);
                    if (isKs) {
                        updateKeySwitch(midiPitch);
                    }
                    // printf("send note on to bank %d sub%d pitch %d\n", bank, iSub, midiPitch); fflush(stdout);
                }
            }
        }

        // Step 2: LFM processing
        float_4 fm = float_4::zero();
        if (lfmConnected_n) {
            Port& pIn = TBase::inputs[LFM_INPUT];
            Port& pDepth = TBase::inputs[LFMDEPTH_INPUT];
            float_4 depth = pDepth.isConnected() ? pDepth.getPolyVoltageSimd<float_4>(bank * 4) : 10.f;
            depth *= .1f;
            float_4 rawInput = pIn.getPolyVoltageSimd<float_4>(bank * 4);
            fm = rawInput * lfmGain_n * depth;
            // SQINFO("read fm=%s, raw=%s", toStr(fm).c_str(), toStr(rawInput).c_str());
        }

        // Step 3: run the audio
        auto output = playback[bank].step(gmask, args.sampleTime, fm, lfmConnected_n);
        output *= taperedVolume_n;
        TBase::outputs[AUDIO_OUTPUT].setVoltageSimd(output, bank * 4);
        lastGate4[bank] = gate4;
    }
    //    SQINFO("pout");
}
#endif

template <class TBase>
int SampDescription<TBase>::getNumParams() {
    return Samp<TBase>::NUM_PARAMS;
}

template <class TBase>
inline IComposite::Config SampDescription<TBase>::getParam(int i) {
    Config ret(0, 1, 0, "");
    switch (i) {
#ifdef _SAMPFM
        case Samp<TBase>::PITCH_PARAM:
            ret = {-5, 5, 0, "Pitch"};
            break;
        case Samp<TBase>::PITCH_TRIM_PARAM:
            ret = {-1, 1, 0, "Pitch trim"};
            break;

        case Samp<TBase>::LFM_DEPTH_PARAM:
            ret = {0, 10, 0, "LFM Depth"};
            break;
#endif
        case Samp<TBase>::DUMMYKS_PARAM:
            ret = {-1, 127, -1, "Key Switch"};
            break;
        case Samp<TBase>::VOLUME_PARAM:
            ret = {0, 100, 50, "Volume"};
            break;
        case Samp<TBase>::SCHEMA_PARAM:
            ret = {0, 10, 0, "SCHEMA"};
            break;
        case Samp<TBase>::TRIGGERDELAY_PARAM:
            ret = {0, 1, 1, "TRIGGER DELAY"};
            break;
         case Samp<TBase>::OCTAVE_PARAM:
            ret = {0, 10, 4, "Octave"};
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
        // SQINFO("worker about to wait for sample access");
        assert(smsg->sharedState);
        smsg->sharedState->uiw_requestAndWaitForSampleReload();
        // SQINFO("worker got sample access");
#endif

        // First thing we do it throw away the old patch data.
        // We couldn't do that on the audio thread, since mem allocation will block the thread
        smsg->waves.reset();
        smsg->instrument.reset();

        parsePath(smsg);

        SInstrumentPtr inst = std::make_shared<SInstrument>();

        // SQINFO("--- real sampler, calling goFile with %s", fullPath.toString().c_str());
        // now load it, and then return it.
        auto err = SParse::goFile(fullPath, inst);

        //SQINFO("about to compile");
        // TODO: need a way for compiler to return error;
        SamplerErrorContext errc;
        CompiledInstrumentPtr cinst = err.empty() ? CompiledInstrument::make(errc, inst) : CompiledInstrument::make(err);
        // SQINFO("back from comp");
        errc.dump();
        if (!cinst) {
            SQWARN("comp was null (should never happen)");
            sendMessageToClient(msg);
            return;
        }
        //  cinst->_dump(0);
        WaveLoaderPtr waves = std::make_shared<WaveLoader>();
        assert(cinst->getInfo());

        //  samplePath += cinst->getDefaultPath();
        samplePath.concat(cinst->getDefaultPath());
        //SQINFO("calling setWaves on %s", samplePath.toString().c_str());
        cinst->setWaves(waves, samplePath);

        WaveLoader::LoaderState loadedState;
        for (bool done = false; !done;) {
            loadedState = waves->loadNextFile();
            switch (loadedState) {
                case WaveLoader::LoaderState::Progress:
                    smsg->sharedState->uiw_setLoadProgress(waves->getProgressPercent());
                    break;
                case WaveLoader::LoaderState::Done:
                case WaveLoader::LoaderState::Error:
                    done = true;
                    break;
                default:
                    assert(false);
            }
        }

        //SQINFO("preparing to return cinst to caller, err=%d", cinst->isInError());
        smsg->instrument = cinst;
        smsg->waves = loadedState == WaveLoader::LoaderState::Done ? waves : nullptr;

        // this "info" is kept with the compiled instrument.
        // but we can modify it here and the UI will "see" it.
        auto info = cinst->getInfo();
        assert(info);

        if (info->errorMessage.empty() && loadedState != WaveLoader::LoaderState::Done) {
            info->errorMessage = waves->lastError;
        }

        //SQINFO("****** loader thread returning %d", int(loadedState));
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
        if (msg->pathToSfz) {
            // maybe we should allow raw strings to come in this way. but it's probably fine
            fullPath = FilePath(*(msg->pathToSfz));
            delete msg->pathToSfz;
            msg->pathToSfz = nullptr;
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
        SQWARN("Unable to sent message to server.");
        messagePool.push(msg);
    }
}

template <class TBase>
void Samp<TBase>::serviceMessagesReturnedToComposite() {
    // see if any messages came back for us
    ThreadMessage* newMsg = thread->getMessage();
    if (newMsg) {
        // SQINFO("new patch message back from worker thread!");
        assert(newMsg->type == ThreadMessage::Type::SAMP);
        SampMessage* smsg = static_cast<SampMessage*>(newMsg);
        setNewPatch(smsg);
        // SQINFO("new patch message back from worker thread done!");
        messagePool.push(smsg);
        // SQINFO("leave snpm");
    }
}
