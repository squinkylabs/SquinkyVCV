
#include "IMidiPlayerHost.h"
#include "MidiVoice.h"

/*
class MidiVoice
{
public:
    void setHost(IMidiPlayerHost*);

/
    void playNote(float pitch, float duration);


    bool isPlaying() const;
    float pitch() const;
private:
    double noteOffTime = -1;        // the absolute metric time when the 
                                    // currently playing note should stop
    float pitch = -100;              // the pitch of the last note played in this voice
};
*/


MidiVoice::State MidiVoice::state() const
{
    return curState;
}

void MidiVoice::setHost(IMidiPlayerHost* ph)
{
    host = ph;
}

void MidiVoice::playNote(float pitch, float endTime)
{
    this->curPitch = pitch;
    this->noteOffTime = endTime;

    // This is over-simplified - doesn't account for what if a note was already playing

    this->curState = State::Playing;
    host->setCV(0, pitch);
    host->setGate(0, true);
}