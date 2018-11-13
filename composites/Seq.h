#pragma once

#include "GateTrigger.h"
#include "MidiPlayer.h"



template <class TBase>
class Seq : public TBase
{
public:
    Seq(struct Module * module) : TBase(module),  gateTrigger(true)
    {
        init();
    }
    Seq() : TBase(),  gateTrigger(true)
    {
        init();
    }

    enum ParamIds
    {
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
        NUM_LIGHTS
    };

    void step() override;
private:
    GateTrigger gateTrigger; 
    void init();

    std::shared_ptr<MidiPlayer> player;
};

#if 1
template <class TBase>
class SeqHost : public MidiPlayer::IPlayerHost
{
public:
    SeqHost(Seq<TBase>* s) : seq(s)
    {
    }
    void setGate(bool gate) override
    {
        fprintf(stderr, "setGate %d\n", gate); fflush(stderr);

    }
    void setCV(float cv) override
    {
        fprintf(stderr, "setCV %f\n", cv); fflush(stderr);
    }
private:
    Seq<TBase>* const seq;
};
#endif

template <class TBase>
void  Seq<TBase>::init()
{
    //  MidiPlayer(std::shared_ptr<IPlayerHost> host, std::shared_ptr<MidiSong> song) 
    std::shared_ptr<MidiPlayer::IPlayerHost> host = std::make_shared<SeqHost<TBase>>(this);
    
   //std::shared_ptr<MidiPlayer::IPlayerHost> host;
    std::shared_ptr<MidiSong> song = MidiSong::makeTest1();
    player = std::make_shared<MidiPlayer>(host, song);
}

template <class TBase>
void  Seq<TBase>::step()
{
    player->timeElapsed( engineGetSampleTime());
}



