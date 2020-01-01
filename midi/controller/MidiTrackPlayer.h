#pragma once

#include "MidiVoice.h"
#include "MidiVoiceAssigner.h"

#include <memory>

class IMidiPlayerHost4;

class MidiTrackPlayer
{
public:
    MidiTrackPlayer(std::shared_ptr<IMidiPlayerHost4> host, int trackIndex);
    void resetAllVoices(bool clearGates);
private:
    int numVoices = 1;      // up to 16
   // const int trackIndex;

    static const int maxVoices = 16;
    MidiVoice voices[maxVoices];
    MidiVoiceAssigner voiceAssigner;

};
