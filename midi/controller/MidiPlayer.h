#pragma once
#include <memory>

class MidiSong;

/**
 * Need to decide on some units:
 *
 * Pitch = float volts (VCV standard).
 * Metric Time = float, quarter notes.
 * Tempo = float, BPM
 */
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

    void timeElapsed(float seconds);
private:
    std::shared_ptr<IPlayerHost> host;
    std::shared_ptr<MidiSong> song;
};