
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
    * re-calc everything that changes with sample
    * rate. Also everything that depends on baseFrequency.
    *
    * Only needs to be called once.
    */
   // void init();

    enum ParamIds
    {
        CLOCK_INPUT_PARAM,
        NUM_VOICES_PARAM,
        RUNNING_PARAM,
        NUM_PARAMS
    };

    enum InputIds
    {
        CLOCK_INPUT,
        RESET_INPUT,
        RUN_INPUT,
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
        GATE_LIGHT,
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

private:
    GateTrigger runStopProcessor;
    std::shared_ptr<MidiPlayer4> player;
    SeqClock clock;
    Divider div;
    bool runStopRequested = false;
    bool wasRunning = false;

    bool isRunning();
    void init(MidiSong4Ptr);
    void serviceRunStop();
    void allGatesOff();
    /**
     * called by the divider every 'n' step calls
     */
    void stepn(int n);

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
        assert(track == 0);
#if defined(_MLOG)
        printf("host::setGate(%d) = (%d, %.2f) t=%f\n", 
            voice, 
            gate,
            seq->outputs[Seq4<TBase>::CV_OUTPUT].voltages[voice],
            seq->getPlayPosition()); fflush(stdout);
#endif
        seq->outputs[Seq4<TBase>::GATE_OUTPUT].voltages[voice] = gate ? 10.f : 0.f;
    }

    void setCV(int track, int voice, float cv) override
    {
        assert(track == 0);
#if defined(_MLOG)
        printf("*** host::setCV(%d) = (%d, %.2f) t=%f\n", 
            voice, 
            seq->outputs[Seq4<TBase>::GATE_OUTPUT].voltages[voice] > 5,
            cv,
            seq->getPlayPosition()); fflush(stdout);
#endif
        seq->outputs[Seq4<TBase>::CV_OUTPUT].voltages[voice] = cv;
    }
    void onLockFailed() override
    {

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
void  Seq4<TBase>::stepn(int n)
{
     serviceRunStop();

#if 0
    if (TBase::params[STEP_RECORD_PARAM].value > .5f) {
        stepRecordInput.step();
    }

    
    audition->enable(!isRunning() && (TBase::params[AUDITION_PARAM].value > .5f));
    audition->sampleTicksElapsed(n);
#endif
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
    const int numVoices = (int) std::round(TBase::params[NUM_VOICES_PARAM].value + 1);
    TBase::outputs[CV_OUTPUT].channels = numVoices;
    TBase::outputs[GATE_OUTPUT].channels = numVoices;
    player->setNumVoices(numVoices);

    if (!running && wasRunning) {
        allGatesOff();
    }
    wasRunning = running;

    // light the gate LED is any voices playing
    bool isGate = false;
    for (int i=0; i<numVoices; ++i) {
        isGate = isGate || (TBase::outputs[GATE_OUTPUT].voltages[i] > 5);
    }
    TBase::lights[GATE_LIGHT].value = isGate;

    player->updateSampleCount(n);
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
bool Seq4<TBase>::isRunning()
{
    return TBase::params[RUNNING_PARAM].value > .5;
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
            ret = {-1.0f, 1.0f, 0, "Clock input"};
            break;
        case Seq4<TBase>::RUNNING_PARAM:
            ret = {0, 1, 0, "Running"};
            break;
        case Seq4<TBase>::NUM_VOICES_PARAM:
            ret = {0, 15, 0, "Polyphony"};
            break;
        default:
            assert(false);
    }
    return ret;
}


