
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
    return voices + 0;          // very dumb!
}