#include "MidiTrackPlayer.h"

#include <stdio.h>

MidiTrackPlayer::MidiTrackPlayer() :
  voiceAssigner(voices, 16)
{
    printf("mtp nimp\n");
    #if 0
    for (int i = 0; i < 16; ++i) {
        MidiVoice& vx = voices[i];
        vx.setHost(host.get());
        vx.setIndex(i);
    }
    #endif
}

void MidiTrackPlayer::resetAllVoices(bool clearGates)
{
    printf("mtp resetAllVoices\n");
        for (int i = 0; i < numVoices; ++i) {
        voices[i].reset(clearGates);
    }
}
