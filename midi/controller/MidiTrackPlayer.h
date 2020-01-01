#pragma once

#include "MidiVoice.h"
#include "MidiVoiceAssigner.h"
class IMidiPlayerHost4;

class MidiTrackPlayer
{
public:
    MidiTrackPlayer();
    void resetAllVoices(bool clearGates);
private:
    int numVoices = 1;      // up to 16

    static const int maxVoices = 16;
    MidiVoice voices[maxVoices];
    MidiVoiceAssigner voiceAssigner;

};
