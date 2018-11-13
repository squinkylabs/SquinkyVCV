#pragma once

#include "GateTrigger.h"
#include "MidiPlayer.h"

class SeqHost : public MidiPlayer::IPlayerHost
{
public:
    void setGate(bool) override
    {

    }
    void setCV(float) override
    {
        
    }
};

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
private:
    GateTrigger gateTrigger; 
    void init();

    std::shared_ptr<MidiPlayer> player;
};

template <class TBase>
void  Seq<TBase>::init()
{
    //  MidiPlayer(std::shared_ptr<IPlayerHost> host, std::shared_ptr<MidiSong> song) 
    std::shared_ptr<SeqHost> host = std::make_shared<SeqHost>();
    MidiSongPtr song = MidiSong::makeTest1();
    player = std::make_shared<MidiPlayer>(host, song);
}



