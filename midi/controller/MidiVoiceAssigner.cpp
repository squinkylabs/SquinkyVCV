
#include "MidiVoiceAssigner.h"

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

}

void MidiVoiceAssigner::setNumVoices(int)
{

}

MidiVoice* MidiVoiceAssigner::getNext(float pitch)
{
    return nullptr;
}