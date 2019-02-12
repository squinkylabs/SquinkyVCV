#pragma once

#include "GateTrigger.h"
#include "IComposite.h"
#include "MidiPlayer.h"
#include "MidiSong.h"


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

    Seq(struct Module * module) : TBase(module), gateTrigger(true)
    {
        init();
    }
    Seq() : TBase(), gateTrigger(true)
    {
        init();
    }

    enum ParamIds
    {
        CLOCK_INPUT_PARAM,
        TEMPO_PARAM,
        NUM_PARAMS
    };

    enum InputIds
    {
        CLOCK_INPUT,
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

    MidiSongPtr getSong()
    {
        return player->getSong();
    }


    void stop()
    {
        player->stop();
    }

    static std::vector<std::string> getClockRates();
private:
    GateTrigger gateTrigger;
    void init();

    std::shared_ptr<MidiPlayer> player;
};

#if 1
template <class TBase>
class SeqHost : public IPlayerHost
{
public:
    SeqHost(Seq<TBase>* s) : seq(s)
    {
    }
    void setGate(bool gate) override
    {
        //fprintf(stderr, "setGate %d\n", gate); fflush(stderr);
        seq->outputs[Seq<TBase>::GATE_OUTPUT].value = gate ? 10.f : 0.f;
    }
    void setCV(float cv) override
    {
       // fprintf(stderr, "setCV %f\n", cv); fflush(stderr);
        seq->outputs[Seq<TBase>::CV_OUTPUT].value = cv;
    }
    void onLockFailed() override
    {

    }
private:
    Seq<TBase>* const seq;
};
#endif

template <class TBase>
void  Seq<TBase>::init()
{ 
    std::shared_ptr<IPlayerHost> host = std::make_shared<SeqHost<TBase>>(this);
    std::shared_ptr<MidiSong> song = MidiSong::makeTest(MidiTrack::TestContent::eightQNotes, 0);
    player = std::make_shared<MidiPlayer>(host, song);
}

template <class TBase>
void  Seq<TBase>::step()
{
    static float last = -100;
    float rate =  TBase::params[CLOCK_INPUT_PARAM].value;
    if (rate != last) {
      //  printf("new rate %f\n", rate);
      //  fflush(stdout);
        last = rate;
    }
    player->timeElapsed(TBase::engineGetSampleTime());
}

template <class TBase>
inline std::vector<std::string> Seq<TBase>::getClockRates()
{
    return {
        "Internal",
        "64th note",
        "32nd note",
        "16th note",
        "8th note",
        "Quarter"
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
            ret = {0, 5, 2, "Clock Rate"};
            break;
        default:
            assert(false);
    }
    return ret;
}


