#pragma once

#include "Divider.h"
#include "IComposite.h"
#include "MidiPlayer.h"
#include "MidiSong.h"
#include "SeqClock.h"

#define _PLAY2

#ifdef __V1x
namespace rack {
    namespace engine {
        struct Module;
    }
}
using Module = rack::engine::Module;
#else
namespace rack {
    struct Module;
};
using Module = rack::Module;
#endif

template <class TBase>
class SeqDescription : public IComposite
{
public:
    Config getParam(int i) override;
    int getNumParams() override;
};



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

    /**
     * Set new song, perhaps after loading a new patch
     */
    void setSong(MidiSongPtr);

    enum ParamIds
    {
        CLOCK_INPUT_PARAM,
        TEMPO_PARAM,
        RUN_STOP_PARAM,             // the switch the user pushes
        PLAY_SCROLL_PARAM,
        RUNNING_PARAM,              // the invisible param that stores the run 
        NUM_VOICES_PARAM,
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

    void stop()
    {
        player->stop();
    }

    bool isRunning();

    float getPlayPosition()
    {
        double absTime = clock.getCurMetricTime();
        double loopDuration = player->getLoopStart();
        double ret = absTime - loopDuration;
        return ret;
    }

    static std::vector<std::string> getClockRates();
    static std::vector<std::string> getPolyLabels();
private:
    GateTrigger runStopProcessor;
    void init(MidiSongPtr);
    void serviceRunStop();

    std::shared_ptr<MidiPlayer> player;
    SeqClock clock;
    Divider div;
    bool runStopRequested = false;

   

    /**
     * called by the divider every 'n' step calls
     */
    void stepn(int n);
};

template <class TBase>
class SeqHost : public IPlayerHost
{
public:
    SeqHost(Seq<TBase>* s) : seq(s)
    {
    }
    void setGate(bool gate) override
    {
        seq->outputs[Seq<TBase>::GATE_OUTPUT].value = gate ? 10.f : 0.f;
    }
    void setCV(float cv) override
    {
        seq->outputs[Seq<TBase>::CV_OUTPUT].value = cv;
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
    std::shared_ptr<IPlayerHost> host = std::make_shared<SeqHost<TBase>>(this);
    player = std::make_shared<MidiPlayer>(host, song);
    div.setup(4, [this] {
        this->stepn(div.getDiv());
     });
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
    return TBase::params[RUNNING_PARAM].value > 5;
}

template <class TBase>
void  Seq<TBase>::serviceRunStop()
{
    runStopProcessor.go(TBase::inputs[RUN_INPUT].value);
    if (runStopProcessor.trigger() || runStopRequested) { 
        runStopRequested = false;
        bool curValue = isRunning();
        curValue = !curValue;
        TBase::params[RUNNING_PARAM].value = curValue ? 10.f : 0.f;
    }
    TBase::lights[RUN_STOP_LIGHT].value = TBase::params[RUNNING_PARAM].value;
}

template <class TBase>
void  Seq<TBase>::stepn(int n)
{
    serviceRunStop();
    // first process all the clock input params
    const int clockRate = (int) std::round(TBase::params[CLOCK_INPUT_PARAM].value);
    const float tempo = TBase::params[TEMPO_PARAM].value;
    clock.setup(clockRate, tempo, TBase::engineGetSampleTime());

    // and the clock input
    const float extClock = TBase::inputs[CLOCK_INPUT].value;

    // now call the clock (internal only, for now

    const float reset = TBase::inputs[RESET_INPUT].value;
    int samplesElapsed = n;
    SeqClock::ClockResults results = clock.update(samplesElapsed, extClock, isRunning(), reset);
    if (results.didReset) {
        player->reset();
    }
    player->updateToMetricTime(results.totalElapsedTime);

    TBase::lights[GATE_LIGHT].value = TBase::outputs[GATE_OUTPUT].value;
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
            ret = {0, 5, 0, "Clock Rate"};
            break;
        case Seq<TBase>::TEMPO_PARAM:
            ret = {40, 200, 120, "Tempo"};
            break;
        case Seq<TBase>::RUN_STOP_PARAM:
            ret = {0, 1, 0, "Run/Stop"};
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
        default:
            assert(false);
    }
    return ret;
}


