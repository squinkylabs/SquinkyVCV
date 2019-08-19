#pragma once

#include "Divider.h"
#include "IComposite.h"

#include "MidiSong.h"
#include "SeqClock.h"

#include "IMidiPlayerHost.h"
#include "MidiAudition.h"
#include "MidiPlayer2.h"

namespace rack {
    namespace engine {
        struct Module;
    }
}
using Module = rack::engine::Module;

template <class TBase>
class SeqDescription : public IComposite
{
public:
    Config getParam(int i) override;
    int getNumParams() override;
};

/**
 *
 */
template <class TBase>
class Seq : public TBase
{
public:
    template <class Tx>
    friend class SeqHost;

    Seq(Module * module, MidiSongPtr song) : TBase(module), runStopProcessor(true)
    {
        init(song);
    }

    Seq(MidiSongPtr song) : TBase(), runStopProcessor(true)
    {
        init(song);
    }

    IMidiPlayerAuditionHostPtr getAuditionHost()
    {
        return audition;
    }

    /**
     * Set new song, perhaps after loading a new patch
     */
    void setSong(MidiSongPtr);

    enum ParamIds
    {
        CLOCK_INPUT_PARAM,
        UNUSED_TEMPO_PARAM,
        UNUSED_RUN_STOP_PARAM,             // the switch the user pushes (actually unused????
        PLAY_SCROLL_PARAM,
        RUNNING_PARAM,              // the invisible param that stores the run 
        NUM_VOICES_PARAM,
        AUDITION_PARAM,
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
        CV_OUTPUT,
        GATE_OUTPUT,
        NUM_OUTPUTS
    };

    enum LightIds
    {
        GATE_LIGHT,
        RUN_STOP_LIGHT,
        NUM_LIGHTS
    };

    void step() override;

    /** This should be called on audio thread
     * (but is it??)
     */
    void toggleRunStop()
    {
        runStopRequested = true;
    }


    /** Implement IComposite
     */
    static std::shared_ptr<IComposite> getDescription()
    {
        return std::make_shared<SeqDescription<TBase>>();
    }

    bool isRunning();

    float getPlayPosition()
    {

        // NOTE: this calculation is wrong. need subrange loop start, too
        double absTime = clock.getCurMetricTime();
        double loopDuration = player->getCurrentLoopIterationStart();

        // absTime - loop duration is the metric time of the start of the current loop,
        // if the overall loop starts at t=0
        double ret = absTime - loopDuration;

        // push it up to take into account subrange looping
        ret += player->getCurrentSubrangeLoopStart();
        return float(ret);
    }

    void onSampleRateChange();

    static std::vector<std::string> getClockRates();
    static std::vector<std::string> getPolyLabels();
private:
    GateTrigger runStopProcessor;
    void init(MidiSongPtr);
    void serviceRunStop();

    std::shared_ptr<MidiAudition> audition;
    SeqClock clock;
    Divider div;
    bool runStopRequested = false;

    bool wasRunning = false;
    std::shared_ptr<MidiPlayer2> player;
    /**
     * called by the divider every 'n' step calls
     */
    void stepn(int n);
};

template <class TBase>
class SeqHost : public IMidiPlayerHost
{
public:
    SeqHost(Seq<TBase>* s) : seq(s)
    {
    }
    void setGate(int voice, bool gate) override
    {
#ifdef _MLOG
        printf("gate(%d) = %d t=%f\n", voice, gate, seq->getPlayPosition()); fflush(stdout);
#endif
        seq->outputs[Seq<TBase>::GATE_OUTPUT].voltages[voice] = gate ? 10.f : 0.f;
    }
    void setCV(int voice, float cv) override
    {
        seq->outputs[Seq<TBase>::CV_OUTPUT].voltages[voice] = cv;
    }
    void onLockFailed() override
    {

    }
private:
    Seq<TBase>* const seq;
};

template <class TBase>
void  Seq<TBase>::init(MidiSongPtr song)
{ 
    std::shared_ptr<IMidiPlayerHost> host = std::make_shared<SeqHost<TBase>>(this);
    player = std::make_shared<MidiPlayer2>(host, song);
    audition = std::make_shared<MidiAudition>(host);

    div.setup(4, [this] {
        this->stepn(div.getDiv());
     });
    onSampleRateChange();
}


template <class TBase>
void Seq<TBase>::onSampleRateChange()
{
    float secondsPerRetrigger = 1.f / 1000.f;
    float samplePerTrigger = secondsPerRetrigger * this->engineGetSampleRate();
    player->setSampleCountForRetrigger((int) samplePerTrigger);
    audition->setSampleTime(this->engineGetSampleTime());
}

template <class TBase>
void  Seq<TBase>::setSong(MidiSongPtr newSong)
{
    player->setSong(newSong);
}

template <class TBase>
void  Seq<TBase>::step()
{
    div.step();
}

template <class TBase>
bool Seq<TBase>::isRunning()
{
    return TBase::params[RUNNING_PARAM].value > .5;
}

template <class TBase>
void  Seq<TBase>::serviceRunStop()
{
    runStopProcessor.go(TBase::inputs[RUN_INPUT].value);
    if (runStopProcessor.trigger() || runStopRequested) { 
        runStopRequested = false;
        bool curValue = isRunning();
        curValue = !curValue;
        TBase::params[RUNNING_PARAM].value = curValue ? 1.f : 0.f;
    }
    TBase::lights[RUN_STOP_LIGHT].value = TBase::params[RUNNING_PARAM].value;
}

template <class TBase>
void  Seq<TBase>::stepn(int n)
{
    serviceRunStop();
    audition->enable(!isRunning() && (TBase::params[AUDITION_PARAM].value > .5f));
    audition->sampleTicksElapsed(n);
    // first process all the clock input params
    const SeqClock::ClockRate clockRate = SeqClock::ClockRate((int) std::round(TBase::params[CLOCK_INPUT_PARAM].value));
    //const float tempo = TBase::params[TEMPO_PARAM].value;
    clock.setup(clockRate, 0, TBase::engineGetSampleTime());

    // and the clock input
    const float extClock = TBase::inputs[CLOCK_INPUT].value;

    
    
    // now call the clock 
    const float reset = TBase::inputs[RESET_INPUT].value;
    const bool running = isRunning();
    int samplesElapsed = n;

    // Our level sensitiev reset will get turned into an edge in here
    SeqClock::ClockResults results = clock.update(samplesElapsed, extClock, running, reset);
    if (results.didReset) {
        player->reset(true);
    }
  //  printf("in step, time = %.2f ext was %.2f isRunning - %d reset = %.2f\n", results.totalElapsedTime, extClock, running, reset);
    player->updateToMetricTime(results.totalElapsedTime, float(clock.getMetricTimePerClock()));

    TBase::lights[GATE_LIGHT].value = TBase::outputs[GATE_OUTPUT].value;

    // copy the current voice number to the poly ports
    const int numVoices = (int) std::round(TBase::params[NUM_VOICES_PARAM].value + 1);
    TBase::outputs[CV_OUTPUT].channels = numVoices;
    TBase::outputs[GATE_OUTPUT].channels = numVoices;
    player->setNumVoices(numVoices);

    if (!running && wasRunning) {
        for (int i = 0; i < numVoices; ++i) {
            TBase::outputs[GATE_OUTPUT].voltages[i] = 0;
        }
    }
    wasRunning = running;

    player->updateSampleCount(n);
}

template <class TBase>
inline std::vector<std::string> Seq<TBase>::getClockRates()
{
    return SeqClock::getClockRates();
}

template <class TBase>
inline std::vector<std::string> Seq<TBase>::getPolyLabels()
{
    return { "1", "2", "3", "4",
        "5", "6", "7", "8",
        "9", "10", "11", "12",
        "13", "14", "15", "16",
    };
}

template <class TBase>
int SeqDescription<TBase>::getNumParams()
{
    return Seq<TBase>::NUM_PARAMS;
}

template <class TBase>
inline IComposite::Config SeqDescription<TBase>::getParam(int i)
{
    Config ret(0, 1, 0, "");
    switch (i) {
        case Seq<TBase>::CLOCK_INPUT_PARAM:
        {
            float low = 0;
            float high = int(SeqClock::ClockRate::NUM_CLOCKS) + 1;
            ret = {low, high, low, "Clock Rate"};
        }
            break;
        case Seq<TBase>::UNUSED_TEMPO_PARAM:
            ret = {40, 200, 120, "Tempo"};
            break;
        case Seq<TBase>::UNUSED_RUN_STOP_PARAM:
            ret = {0, 1, 0, "unused Run/Stop"};
            break;
        case Seq<TBase>::PLAY_SCROLL_PARAM:
            ret = {0, 1, 0, "Scroll during playback"};
            break;
        case Seq<TBase>::RUNNING_PARAM:
            ret = {0, 1, 1, "Running"};
            break;
        case Seq<TBase>::NUM_VOICES_PARAM:
            ret = {0, 15, 0, "Polyphony"};
            break;
        case Seq<TBase>::AUDITION_PARAM:
            ret = {0, 1, 1, "Audition"};
            break;
        default:
            assert(false);
    }
    return ret;
}


