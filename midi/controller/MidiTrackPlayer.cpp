#include "MidiTrackPlayer.h"

#include <stdio.h>

MidiTrackPlayer::MidiTrackPlayer(std::shared_ptr<IMidiPlayerHost4> host, int track) :
  voiceAssigner(voices, 16)
{
    for (int i = 0; i < 16; ++i) {
        MidiVoice& vx = voices[i];
        vx.setHost(host.get());
        vx.setTrack(track);
        vx.setIndex(i);
    }
}

void MidiTrackPlayer::resetAllVoices(bool clearGates)
{
    printf("mtp resetAllVoices\n");
        for (int i = 0; i < numVoices; ++i) {
        voices[i].reset(clearGates);
    }
}
