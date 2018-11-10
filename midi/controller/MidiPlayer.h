#pragma once
#include <memory>

class MidiSong;

class MidiPlayer
{
public:
    class IPlayerHost
    {
    public:
        virtual void setGate(bool) = 0;
        virtual void setCV(float) = 0;
    };

    MidiPlayer(std::shared_ptr<IPlayerHost> host, std::shared_ptr<MidiSong> song) :
            host(host), song(song)
    {

    }
private:
    std::shared_ptr<IPlayerHost> host;
    std::shared_ptr<MidiSong> song;
};