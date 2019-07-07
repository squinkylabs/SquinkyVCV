
#include "IMidiPlayerHost.h"
#include "MidiVoice.h"

#include <assert.h>
#include <stdio.h>

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

void MidiVoice::setIndex(int i)
{
    index = i;
}

void MidiVoice::setGate(bool g)
{
    host->setGate(index, g);
}

void MidiVoice::setCV(float cv)
{
    host->setCV(index, cv);
}

void MidiVoice::setSampleCountForRetrigger(int samples)
{
    printf("setSampleCountForRetrigger ng\n");
}

void MidiVoice::updateSampleCount(int samples)
{
    printf("updateSampleCount ng\n");
}

void MidiVoice::playNote(float pitch, double currentTime, float endTime)
{
    // do re-triggering, if needed
    if (currentTime == lastNoteOffTime) {
        curState = State::ReTriggering;
        setGate(false);
        delayedNotePitch = pitch;
        delayedNoteEndtime = endTime;
    } else {
        this->curPitch = pitch;
        this->noteOffTime = endTime;


        this->curState = State::Playing;
        setCV(pitch);
        setGate(true);
    }
}

void MidiVoice::updateToMetricTime(double metricTime)
{
    if (noteOffTime >= 0 && noteOffTime <= metricTime) {
        setGate(false);
        lastNoteOffTime = noteOffTime;
        noteOffTime = -1;
        curState = State::Idle;
    }
}