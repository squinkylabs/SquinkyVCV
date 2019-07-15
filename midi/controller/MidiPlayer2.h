#pragma once

#include <memory>

class MidiSong;
class IMidiPlayerHost;

#include "MidiTrack.h"
#include "MidiVoice.h"
#include "MidiVoiceAssigner.h"

class MidiPlayer2
{
public:
    MidiPlayer2(std::shared_ptr<IMidiPlayerHost> host, std::shared_ptr<MidiSong> song);
    void setSong(std::shared_ptr<MidiSong> song);

    void updateToMetricTime(double metricTime);

    void setNumVoices(int voices);

    /**
     * resets all internal playback state.
     * @param clearGate will set the host's gate low, if true
     */
    void reset(bool clearGates);
    void stop();
    double getLoopStart() const;

    void setSampleCountForRetrigger(int);
    void updateSampleCount(int numElapsed);


private:
    std::shared_ptr<IMidiPlayerHost> host;
    std::shared_ptr<MidiSong> song;

    static const int maxVoices = 16;
    MidiVoice voices[maxVoices];
    MidiVoiceAssigner voiceAssigner;

    /***************************************
     * Variables  to play one track
     */

    MidiTrack::const_iterator curEvent;

    /**
     * when starting, or when reset by lock contention
     */
    bool isReset = true;
    bool isResetGates = false;

    bool isPlaying = true;
    double loopStart = 0;
    int numVoices=1;

    std::shared_ptr<MidiTrack> track;

   
    void updateToMetricTimeInternal(double);
    bool playOnce(double metricTime);
    bool pollForNoteOff(double metricTime);
    void resetAllVoices(bool clearGates);
};
