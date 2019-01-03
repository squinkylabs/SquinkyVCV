#pragma once

#include "MidiEvent.h"
#include "MidiSong.h"
#include "MidiTrack.h"
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
        curEvent = song->getTrack(0)->begin();
    }

    void timeElapsed(float seconds);
private:
    std::shared_ptr<IPlayerHost> host;
    std::shared_ptr<MidiSong> song;

    float curMetricTime = 0;
    float noteOffTime = -1;
    MidiTrack::const_iterator curEvent;

    /**
     * process the next ready event that is after curMetricTime
     * returns true is something was found
     */
    bool playOnce();
};