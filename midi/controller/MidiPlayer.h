#pragma once

//#include "MidiEvent.h"
//#include "MidiSong.h"
#include "MidiTrack.h"
#include <memory>

class MidiSong;
class MidiSequencer;

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

class TrackPlayer
{
public:
    TrackPlayer(std::shared_ptr<MidiTrack> track);
    ~TrackPlayer();

    void reset()
    {
        isReset = true;
    }
    void setTrack(std::shared_ptr<MidiTrack> newTrack)
    {
        track = newTrack;
    }
    void updateToMetricTime(double seconds, IPlayerHost*);
private:
  
    double noteOffTime = -1;
    MidiTrack::const_iterator curEvent;
    bool isReset = true;
    std::shared_ptr<MidiTrack> track;
    double loopStart = 0;

    /**
     * process the next ready event that is after metricTime
     * returns true is something was found
     */
    bool playOnce(double metricTime, IPlayerHost* host);

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

    MidiPlayer(std::shared_ptr<IPlayerHost> host, std::shared_ptr<MidiSong> song);
    ~MidiPlayer()
    {
        --_mdb;
    }
    void setSong(std::shared_ptr<MidiSong>);

    //void timeElapsed(float seconds);
    void updateToMetricTime(double metricTime);

    std::shared_ptr<MidiSong> getSong()
    {
        return song;
    }

    void stop()
    {
        isPlaying = false;
    }

private:
    std::shared_ptr<IPlayerHost> host;
    std::shared_ptr<MidiSong> song;

    bool isPlaying = true;
    TrackPlayer trackPlayer;
};