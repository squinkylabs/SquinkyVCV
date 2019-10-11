
#include "MidiVoiceAssigner.h"
#include "MidiVoice.h"

#include <assert.h>
#include <stdio.h>

MidiVoiceAssigner::MidiVoiceAssigner(MidiVoice* vx, int maxVoices) :
    voices(vx),
    maxVoices(maxVoices),
    numVoices(maxVoices)
{
    assert(maxVoices > 0 && maxVoices <= 16);
    for (int i = 0; i < maxVoices; ++i) {
        assert(voices[i].state() == MidiVoice::State::Idle);
    }
}

void MidiVoiceAssigner::reset()
{
    nextVoice = 0;
}

void MidiVoiceAssigner::setNumVoices(int voices)
{
    numVoices = voices;
    assert(numVoices <= maxVoices);
    if (nextVoice >= numVoices) {
        nextVoice = 0;          // make sure it's valid
    }
}

MidiVoice* MidiVoiceAssigner::getNext(float pitch)
{
    MidiVoice * nextVoice = nullptr;
    switch (mode) {
        case Mode::ReUse:
            nextVoice = getNextReUse(pitch);
            break;
        default:
            assert(false);
    }
#if defined(_MLOG)
    printf("MidiVoiceAssigner::getNext(pitch=%.2f), ret voice #%d state=%d\n",
        pitch, nextVoice->_getIndex(), nextVoice->state()); 
    fflush(stdout);
#endif
    return nextVoice;
}

int MidiVoiceAssigner::wrapAround(int vxNum)
{
    if (vxNum >= numVoices) {
        vxNum -= numVoices;
    }
    return vxNum;
}

int MidiVoiceAssigner::advance(int vxNum)
{
    return wrapAround(vxNum + 1);
}

MidiVoice* MidiVoiceAssigner::getNextReUse(float pitch)
{
    assert(numVoices > 0);

    // first, look for a voice already playing this pitch, but idle
    for (int i = 0; i < numVoices; ++i) {
        if ((voices[i].pitch() == pitch) &&
         (voices[i].state() == MidiVoice::State::Idle)) {
            // OK, we found an idle voice at the desired pitch
            if (i == nextVoice) {
                nextVoice = advance(nextVoice);
            }
            return voices + i;
        }
    }

    // next, look for any idle voice, starting at next voice
    for (int i = 0; i < numVoices; ++i) {
        int candidateVoice = wrapAround(i + nextVoice);
     
        if (voices[candidateVoice].state() == MidiVoice::State::Idle) {
            nextVoice = advance(candidateVoice);
            return voices + candidateVoice;
        }
    }

    // If no idle voices, use the next one
    int i = nextVoice;
    nextVoice = advance(nextVoice);
    return voices + i;            
}