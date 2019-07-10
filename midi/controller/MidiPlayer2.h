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

    // need some UT for these
    void reset();
    void stop();
    double getLoopStart() const;

private:
    std::shared_ptr<IMidiPlayerHost> host;
    std::shared_ptr<MidiSong> song;

    MidiVoice voices[16];
    MidiVoiceAssigner voiceAssigner;

    /***************************************
     * Variables  to play one track
     */

    MidiTrack::const_iterator curEvent;

    /**
     * when starting, or when reset by lock contention
     */
    bool isReset = true;

    bool isPlaying = true;
    double loopStart = 0;
    int numVoices=1;

    std::shared_ptr<MidiTrack> track;

   
    void updateToMetricTimeInternal(double);
    bool playOnce(double metricTime);
    bool pollForNoteOff(double metricTime);
    void resetAllVoices();
};
