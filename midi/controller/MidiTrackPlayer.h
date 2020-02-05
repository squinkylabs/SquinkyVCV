#pragma once

#include "MidiTrack.h"
#include "MidiVoice.h"
#include "MidiVoiceAssigner.h"

#include <memory>

class IMidiPlayerHost4;
class MidiSong4;
class MidiTrack;

// #define _MLOG

class MidiTrackPlayer
{
public:
    MidiTrackPlayer(std::shared_ptr<IMidiPlayerHost4> host, int trackIndex, std::shared_ptr<MidiSong4> song);
    void setSong(std::shared_ptr<MidiSong4> newSong, int trackIndex);
    void resetAllVoices(bool clearGates);

    /**
     * play the next event, if possible.
     * return true if event played.
     */
    bool playOnce(double metricTime, float quantizeInterval);

    void reset();
    double getCurrentLoopIterationStart() const;
    void setNumVoices(int numVoices);
    void setSampleCountForRetrigger(int);
    void updateSampleCount(int numElapsed);
    std::shared_ptr<MidiSong4> getSong();

    int getSectionIndex() const;
private:
    //std::shared_ptr<IMidiPlayerHost4> host;
    std::shared_ptr<MidiSong4> song;
    std::shared_ptr<MidiTrack> curTrack;   // need something like array for song4??
    const int trackIndex=0;
    int curSectionIndex = 0;
    /**
     * Variables around voice state
     */
    int numVoices = 1;      // up to 16
    static const int maxVoices = 16;
    MidiVoice voices[maxVoices];
    MidiVoiceAssigner voiceAssigner;

    /**
     * variables for playihng a track
     */
    double currentLoopIterationStart = 0;
    MidiTrack::const_iterator curEvent;
    int sectionLoopCounter = 1;

    bool pollForNoteOff(double metricTime);
    void findFirstTrackSection();

};
