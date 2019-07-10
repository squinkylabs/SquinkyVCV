
#include "MidiVoiceAssigner.h"
#include "MidiVoice.h"

#include <assert.h>

/*
class MidiVoiceAssigner
{
public:
    MidiVoiceAssigner(MidiVoice* vx, int maxVoices);
    void setNumVoices(int);
    MidiVoice* getNext(float pitch);
private:
    MidiVoice* const voices;
    const int maxVoices;
};
*/

MidiVoiceAssigner::MidiVoiceAssigner(MidiVoice* vx, int maxVoices) :
    voices(vx),
    maxVoices(maxVoices)
{
    assert(maxVoices > 0 && maxVoices <= 16);
    for (int i = 0; i < maxVoices; ++i) {
        auto s = voices[i].state();
        assert(s == MidiVoice::State::Idle);
    }
}

void MidiVoiceAssigner::setNumVoices(int)
{

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
    return nextVoice;
}

MidiVoice* MidiVoiceAssigner::getNextReUse(float pitch)
{
    //return voices + 0;          // very dumb!

    // first, look for a voice already playing this pitch
    for (int i = 0; i < maxVoices; ++i) {
        if (voices[i].pitch() == pitch) {
            return voices + i;
        }
    }

    // next, look for any idle voice
    for (int i = 0; i < maxVoices; ++i) {
        if (voices[i].state() == MidiVoice::State::Idle) {
            return voices + i;
        }
    }

    return voices + 0;              // if no idle voices, use the first one. 
}