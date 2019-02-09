#pragma once

#include "MidiEvent.h"
#include "MidiSong.h"
#include "MidiTrack.h"
#include <memory>

class MidiSong;

/**
 * Abstract out the player host so that we can more
 * easily test the player.
 */
class IPlayerHost
{
public:
    virtual void setGate(bool) = 0;
    virtual void setCV(float) = 0;
    virtual void onLockFailed() = 0;
};


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


    MidiPlayer(std::shared_ptr<IPlayerHost> host, std::shared_ptr<MidiSong> song) :
        host(host), song(song)
    {
        ++_mdb;
        curEvent = song->getTrack(0)->begin();
    }
    ~MidiPlayer()
    {
        --_mdb;
    }

    void timeElapsed(float seconds);

    MidiSongPtr getSong()
    {
        return song;
    }

    void stop()
    {
        isPlaying = false;
    }

private:
    std::shared_ptr<IPlayerHost> host;
    MidiSongPtr song;

    float curMetricTime = 0;
    float noteOffTime = -1;
    MidiTrack::const_iterator curEvent;
    bool isPlaying = true;

    /**
     * process the next ready event that is after curMetricTime
     * returns true is something was found
     */
    bool playOnce();
};