
#pragma once

#include <assert.h>
#include <memory>
#include <cmath>

#include "Divider.h"
#include "GateTrigger.h"
#include "IComposite.h"
#include "IMidiPlayerHost.h"
#include "MidiPlayer4.h"
#include "SeqClock.h"

// #define _MLOG

namespace rack {
    namespace engine {
        struct Module;
    }
}

using Module = ::rack::engine::Module;

template <class TBase>
class Seq4Description : public IComposite
{
public:
    Config getParam(int i) override;
    int getNumParams() override;
};

template <class TBase>
class Seq4 : public TBase
{
public:
    template <class Tx>
    friend class SeqHost4;

    Seq4(Module * module, MidiSong4Ptr song) :
        TBase(module),
        runStopProcessor(true)
    {
        init(song);
    }

    Seq4(MidiSong4Ptr song) : 
        TBase(), 
        runStopProcessor(true)
    {
        init(song);
    }

    MidiSong4Ptr getSong()
    {
        return player->getSong();
    }

    /**
     * Set new song, perhaps after loading a new patch
     */
    void setSong(MidiSong4Ptr);

    /**
    * re-calc everything that changes with sample
    * rate. Also everything that depends on baseFrequency.
    *
    * Only needs to be called once.
    */
   // void init();

    enum ParamIds
    {
        CLOCK_INPUT_PARAM,
        NUM_VOICES0_PARAM,
        NUM_VOICES_PARAM = NUM_VOICES0_PARAM,
        NUM_VOICES1_PARAM,
        NUM_VOICES2_PARAM,
        NUM_VOICES3_PARAM,
        RUNNING_PARAM,
        TRIGGER_IMMEDIATE_PARAM,
        PADSELECT0_PARAM,           // these pad select params are only used for automation
        PADSELECT1_PARAM,
        PADSELECT2_PARAM,
        PADSELECT3_PARAM,
        PADSELECT4_PARAM,
        PADSELECT5_PARAM,
        PADSELECT6_PARAM,
        PADSELECT7_PARAM,
        PADSELECT8_PARAM,
        PADSELECT9_PARAM,
        PADSELECT10_PARAM,
        PADSELECT11_PARAM,
        PADSELECT12_PARAM,
        PADSELECT13_PARAM,
        PADSELECT14_PARAM,
        PADSELECT15_PARAM,
        NUM_PARAMS
    };

    enum InputIds
    {
        CLOCK_INPUT,
        RESET_INPUT,
        RUN_INPUT,
        MOD0_INPUT,
        MOD1_INPUT,
        MOD2_INPUT,
        MOD3_INPUT,
        SELECT_CV_INPUT,
        SELECT_GATE_INPUT,
        NUM_INPUTS
    };

    enum OutputIds
    {
        // TODO - 1..4
        CV_OUTPUT,
        CV0_OUTPUT = CV_OUTPUT,
        CV1_OUTPUT,
        CV2_OUTPUT,
        CV3_OUTPUT,
        GATE_OUTPUT,
        GATE0_OUTPUT = GATE_OUTPUT,
        GATE1_OUTPUT,
        GATE2_OUTPUT,
        GATE3_OUTPUT ,
        NUM_OUTPUTS
    };

    enum LightIds
    {
        //GATE_LIGHT,
        RUN_STOP_LIGHT,
        NUM_LIGHTS
    };

    /** Implement IComposite
     */
    static std::shared_ptr<IComposite> getDescription()
    {
        return std::make_shared<Seq4Description<TBase>>();
    }

    /**
     * Main processing entry point. Called every sample
     */
    void step() override;

    /**
     * So far, just for test compatibilty with old player
     */
    #if 0
    float getPlayPosition()
    {
          // NOTE: this calculation is wrong. need subrange loop start, too
        double absTime = clock.getCurMetricTime();
        double loopDuration = player->getCurrentLoopIterationStart();

        // absTime - loop duration is the metric time of the start of the current loop,
        // if the overall loop starts at t=0
        double ret = absTime - loopDuration;

#if 0
        // push it up to take into account subrange looping
        ret += player->getCurrentSubrangeLoopStart();
#endif
        return float(ret);
    }
    #endif

    
    /** This should be called on audio thread
     * (but is it??)
     */
    void toggleRunStop()
    {
         runStopRequested = true;
    }

    void onSampleRateChange();
    static std::vector<std::string> getClockRates();
    static std::vector<std::string> getPolyLabels();

    /**
     * return 0 if not playing
     * section number (1..4) if playing
     */
    int  getPlayStatus(int track) const;
    void setNextSectionRequest(int track, int section);
    int getNextSectionRequest(int track) const;

    /**
     * Provide direct access so we don't have to add a zillion
     * "pass thru" APIs.
     */
    MidiTrackPlayerPtr getTrackPlayer(int track);
private:
    GateTrigger runStopProcessor;
    std::shared_ptr<MidiPlayer4> player;
    SeqClock clock;
    Divider div;
    bool runStopRequested = false;
    bool wasRunning = false;

    bool isRunning() const;
    void init(MidiSong4Ptr);
    void serviceRunStop();
    void allGatesOff();
    void resetClock();
    void serviceSelCV();
    /**
     * called by the divider every 'n' step calls
     */
    void stepn(int n);

    bool lastGate[16] = {false};

};

template <class TBase>
class SeqHost4 : public IMidiPlayerHost4
{
public:
    SeqHost4(Seq4<TBase>* s) : seq(s)
    {
    }

    void setGate(int track, int voice, bool gate) override
    {
        assert(track >= 0 && track < 4);
#if defined(_MLOG)
        printf("host::setGate(%d) = (%d, %.2f) t=%f\n", 
            voice, 
            gate,
            seq->outputs[Seq4<TBase>::GATE_OUTPUT0 + track].voltages[voice],
            seq->getPlayPosition()); fflush(stdout);
#endif
        seq->outputs[Seq4<TBase>::GATE0_OUTPUT + track].voltages[voice] = gate ? 10.f : 0.f;
    }

    void setCV(int track, int voice, float cv) override
    {
        assert(track >= 0 && track < 4);
#if defined(_MLOG)
        printf("*** host::setCV(%d) = (%d, %.2f) t=%f\n", 
            voice, 
            seq->outputs[Seq4<TBase>::CV_OUTPUT0 + track].voltages[voice] > 5,
            cv,
            seq->getPlayPosition()); fflush(stdout);
#endif
        seq->outputs[Seq4<TBase>::CV0_OUTPUT + track].voltages[voice] = cv;
    }
    void onLockFailed() override
    {

    }
    void resetClock() override
    {
        seq->resetClock();
    }
private:
    Seq4<TBase>* const seq;
};

template <class TBase>
void  Seq4<TBase>::init(MidiSong4Ptr song)
{ 
    std::shared_ptr<IMidiPlayerHost4> host = std::make_shared<SeqHost4<TBase>>(this);
    player = std::make_shared<MidiPlayer4>(host, song);
   // audition = std::make_shared<MidiAudition>(host);

    div.setup(4, [this] {
        this->stepn(div.getDiv());
     });
    onSampleRateChange();
    player->setPorts( TBase::inputs.data() + MOD0_INPUT, TBase::params.data() + TRIGGER_IMMEDIATE_PARAM);
}

template <class TBase>
void Seq4<TBase>::onSampleRateChange()
{
    float secondsPerRetrigger = 1.f / 1000.f;
    float samplePerTrigger = secondsPerRetrigger * this->engineGetSampleRate();
    player->setSampleCountForRetrigger((int) samplePerTrigger);
  //  audition->setSampleTime(this->engineGetSampleTime());
}

template <class TBase>
void  Seq4<TBase>::serviceSelCV()
{
  
    const int activeChannels = std::min(TBase::inputs[SELECT_CV_INPUT].getChannels(), TBase::inputs[SELECT_GATE_INPUT].getChannels()); 
    
    for (int i=0; i < activeChannels; ++i) {

        const bool gate =  TBase::inputs[SELECT_GATE_INPUT].getVoltage(i) > 2;

        // printf("gate[%d] = %.2f\n", i, TBase::inputs[SELECT_GATE_INPUT].getVoltage(i));
         fflush(stdout);
        if (gate != lastGate[i]) {
              printf("sel %d\n", gate); fflush(stdout);
            lastGate[i] = gate;
            if (gate) {
                const float cv = TBase::inputs[SELECT_CV_INPUT].getVoltage(i);
                auto pitch = PitchUtils::cvToPitch(cv);
                printf("octave = %d, semi=%d\n", pitch.first, pitch.second); fflush(stdout);
            }
        }
    }
}

template <class TBase>
void  Seq4<TBase>::stepn(int n)
{
    player->step();
    serviceRunStop();
    serviceSelCV();

    // first process all the clock input params
    const SeqClock::ClockRate clockRate = SeqClock::ClockRate((int) std::round(TBase::params[CLOCK_INPUT_PARAM].value));
    //const float tempo = TBase::params[TEMPO_PARAM].value;
    clock.setup(clockRate, 0, TBase::engineGetSampleTime());

    // and the clock input
    const float extClock = TBase::inputs[CLOCK_INPUT].getVoltage(0);

    // now call the clock 
    const float reset = TBase::inputs[RESET_INPUT].getVoltage(0);
    const bool running = isRunning();
    int samplesElapsed = n;

    // Our level sensitive reset will get turned into an edge in here
    SeqClock::ClockResults results = clock.update(samplesElapsed, extClock, running, reset);
    if (results.didReset) {
        player->reset(true);
        allGatesOff();          // turn everything off on reset, just in case of stuck notes.
    }

    player->updateToMetricTime(results.totalElapsedTime, float(clock.getMetricTimePerClock()), running);

    // copy the current voice number to the poly ports
    for (int i=0; i<4; ++i) {
        const int numVoices = (int) std::round(TBase::params[NUM_VOICES0_PARAM+i].value + 1);
        TBase::outputs[CV0_OUTPUT + i].channels = numVoices;
        TBase::outputs[GATE0_OUTPUT + i].channels = numVoices;
        player->setNumVoices(i, numVoices);
    }

    if (!running && wasRunning) {
        allGatesOff();
    }
    wasRunning = running;

    // light the gate LED is any voices playing
#if 0
    bool isGate = false;
    for (int i=0; i<numVoices; ++i) {
        isGate = isGate || (TBase::outputs[GATE_OUTPUT].voltages[i] > 5);
    }
    TBase::lights[GATE_LIGHT].value = isGate;
#endif

    player->updateSampleCount(n);
}

template <class TBase>
void  Seq4<TBase>::setSong(MidiSong4Ptr newSong)
{
    player->setSong(newSong);
}

template <class TBase>
void  Seq4<TBase>::serviceRunStop()
{
    runStopProcessor.go(TBase::inputs[RUN_INPUT].getVoltage(0));
    if (runStopProcessor.trigger() || runStopRequested) { 
        runStopRequested = false;
        bool curValue = isRunning();
        curValue = !curValue;
        TBase::params[RUNNING_PARAM].value = curValue ? 1.f : 0.f;
    }
    TBase::lights[RUN_STOP_LIGHT].value = TBase::params[RUNNING_PARAM].value;
}

template <class TBase>
inline void Seq4<TBase>::step()
{
    div.step();
}

template <class TBase>
bool Seq4<TBase>::isRunning() const
{
    bool running = TBase::params[RUNNING_PARAM].value > .5;
    player->setRunningStatus(running);
    return running;
}

template <class TBase>
MidiTrackPlayerPtr Seq4<TBase>::getTrackPlayer(int track)
{
    return player->getTrackPlayer(track);
}

template <class TBase>
int  Seq4<TBase>::getPlayStatus(int track) const
{
    if (!isRunning()) {
        return 0;
    }
    return player->getSection(track);
}

template <class TBase>
void  Seq4<TBase>::setNextSectionRequest(int track, int section)
{
    player->setNextSectionRequest(track, section);
}

template <class TBase>
int  Seq4<TBase>::getNextSectionRequest(int track) const
{
    return player->getNextSectionRequest(track);
}

template <class TBase>
inline void Seq4<TBase>::allGatesOff()
{
    for (int output=0; output < 4; ++output) {
        for (int i = 0; i < 16; ++i) {
            
            TBase::outputs[GATE0_OUTPUT + output].voltages[i] = 0;
        }  
    }
}

template <class TBase>
inline void Seq4<TBase>::resetClock()
{
    clock.reset(false);
}

template <class TBase>
inline std::vector<std::string> Seq4<TBase>::getClockRates()
{
    return SeqClock::getClockRates();
}

template <class TBase>
inline std::vector<std::string> Seq4<TBase>::getPolyLabels()
{
    return { "1", "2", "3", "4",
        "5", "6", "7", "8",
        "9", "10", "11", "12",
        "13", "14", "15", "16",
    };
}

template <class TBase>
int Seq4Description<TBase>::getNumParams()
{
    return Seq4<TBase>::NUM_PARAMS;
}

template <class TBase>
inline IComposite::Config Seq4Description<TBase>::getParam(int i)
{
    Config ret(0, 1, 0, "");
    switch (i) {
        case Seq4<TBase>::CLOCK_INPUT_PARAM:
            {     
            float low = 0;
            float high = int(SeqClock::ClockRate::NUM_CLOCKS) + 1;
            ret = {low, high, low, "Clock Rate"};
            }
            break;
        case Seq4<TBase>::RUNNING_PARAM:
            ret = {0, 1, 1, "Running"};
            break;
        case Seq4<TBase>::NUM_VOICES0_PARAM:
            ret = {0, 15, 0, "Polyphony 1"};
            break;
        case Seq4<TBase>::NUM_VOICES1_PARAM:
            ret = {0, 15, 0, "Polyphony 2"};
            break;
        case Seq4<TBase>::NUM_VOICES2_PARAM:
            ret = {0, 15, 0, "Polyphony 3"};
            break;
        case Seq4<TBase>::NUM_VOICES3_PARAM:
            ret = {0, 15, 0, "Polyphony 4"};
            break;
        case Seq4<TBase>::TRIGGER_IMMEDIATE_PARAM:
            ret = {0, 1, 0, "Trigger Immediate"};
            break;
        case Seq4<TBase>::PADSELECT0_PARAM:
            ret = { 0, 1, 0, "Select 1" };       
            break;
        case Seq4<TBase>::PADSELECT1_PARAM:
            ret = { 0, 1, 0, "Select 2" };
            break;
        case Seq4<TBase>::PADSELECT2_PARAM:
            ret = { 0, 1, 0, "Select 3" };
            break;
        case Seq4<TBase>::PADSELECT3_PARAM:
            ret = { 0, 1, 0, "Select 4" };
            break;
        case Seq4<TBase>::PADSELECT4_PARAM:
            ret = { 0, 1, 0, "Select 5" };
            break;
        case Seq4<TBase>::PADSELECT5_PARAM:
            ret = { 0, 1, 0, "Select 6" };
            break;
        case Seq4<TBase>::PADSELECT6_PARAM:
            ret = { 0, 1, 0, "Select 7" };
            break;
        case Seq4<TBase>::PADSELECT7_PARAM:
            ret = { 0, 1, 0, "Select 8" };
            break;
        case Seq4<TBase>::PADSELECT8_PARAM:
            ret = { 0, 1, 0, "Select 9" };
            break;
        case Seq4<TBase>::PADSELECT9_PARAM:
            ret = { 0, 1, 0, "Select 10" };
            break;
        case Seq4<TBase>::PADSELECT10_PARAM:
            ret = { 0, 1, 0, "Select 11" };
            break;
        case Seq4<TBase>::PADSELECT11_PARAM:
            ret = { 0, 1, 0, "Select 12" };
            break;
        case Seq4<TBase>::PADSELECT12_PARAM:
            ret = { 0, 1, 0, "Select 13" };
            break;
        case Seq4<TBase>::PADSELECT13_PARAM:
            ret = { 0, 1, 0, "Select 14" };
            break;
        case Seq4<TBase>::PADSELECT14_PARAM:
            ret = { 0, 1, 0, "Select 15" };
            break;
        case Seq4<TBase>::PADSELECT15_PARAM:
            ret = { 0, 1, 0, "Select 16" };
            break;
        default:
            assert(false);
    }
    return ret;
}


