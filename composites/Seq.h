#pragma once

#include "Divider.h"
#include "GateTrigger.h"
#include "IComposite.h"
#include "MidiPlayer.h"
#include "MidiSong.h"
#include "SeqClock.h"


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

    Seq(struct Module * module, MidiSongPtr song) : TBase(module), gateTrigger(true)
    {
        init(song);
    }
    Seq(MidiSongPtr song) : TBase(), gateTrigger(true)
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
        RUN_STOP_PARAM,
        PLAY_SCROLL_PARAM,
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
        NUM_LIGHTS
    };

    void step() override;


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

    static std::vector<std::string> getClockRates();
private:
    GateTrigger gateTrigger;
    void init(MidiSongPtr);

    std::shared_ptr<MidiPlayer> player;
    SeqClock clock;
    Divider div;

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
   // std::shared_ptr<MidiSong> song = MidiSong::makeTest(MidiTrack::TestContent::empty, 0);
    player = std::make_shared<MidiPlayer>(host, song);
    div.setup(4, [this] {
        this->stepn(div.div());
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
void  Seq<TBase>::stepn(int n)
{
    // first process all the clock input params
    const int clockRate = (int) std::round(TBase::params[CLOCK_INPUT_PARAM].value);
    const float tempo = TBase::params[TEMPO_PARAM].value;
    clock.setup(clockRate, tempo, TBase::engineGetSampleTime());

    // and the clock input
    gateTrigger.go(TBase::inputs[CLOCK_INPUT].value);

    // now call the clock (internal only, for now
    const bool externalClock = gateTrigger.trigger();
    int samplesElapsed = n;
    double t = clock.update(samplesElapsed, externalClock);
    player->updateToMetricTime(t);

    TBase::lights[GATE_LIGHT].value = TBase::outputs[GATE_OUTPUT].value;
}

template <class TBase>
inline std::vector<std::string> Seq<TBase>::getClockRates()
{
    return SeqClock::getClockRates();
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
            ret = {0, 5, 2, "Clock Rate"};
            break;
        case Seq<TBase>::TEMPO_PARAM:
            ret = {40, 200, 120, "Tempo"};
            break;
        case Seq<TBase>::RUN_STOP_PARAM:
            ret = {0, 1, 0, "Run/Stop"};
            break;
        case Seq<TBase>::PLAY_SCROLL_PARAM:
            ret = {0, 2, 0, "Scroll during playback"};
            break;
        default:
            assert(false);
    }
    return ret;
}


