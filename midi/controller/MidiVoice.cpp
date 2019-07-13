
#include "IMidiPlayerHost.h"
#include "MidiVoice.h"

#include <assert.h>
#include <stdio.h>

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

float MidiVoice::pitch() const
{
    return curPitch;
}

void MidiVoice::setSampleCountForRetrigger(int samples)
{
    numSamplesInRetrigger = samples;
}

void MidiVoice::updateSampleCount(int samples)
{
    if (retriggerSampleCounter) {
        retriggerSampleCounter -= samples;
        if (retriggerSampleCounter <= 0) {
            retriggerSampleCounter = 0;
            curState = State::Playing;
            setCV(delayedNotePitch);
            noteOffTime = delayedNoteEndtime;
            setGate(true);
        }
    } 
}

void MidiVoice::playNote(float pitch, double currentTime, float endTime)
{
    // do re-triggering, if needed
    if (currentTime == lastNoteOffTime) {
        curState = State::ReTriggering;
        setGate(false);
        delayedNotePitch = pitch;
        delayedNoteEndtime = endTime;
        retriggerSampleCounter = numSamplesInRetrigger;
    } else {
        this->curPitch = pitch;
        this->noteOffTime = endTime;

        this->curState = State::Playing;
        setCV(pitch);
        setGate(true);
    }
}

bool MidiVoice::updateToMetricTime(double metricTime)
{
    bool ret = false;
    if (noteOffTime >= 0 && noteOffTime <= metricTime) {
        setGate(false);
        lastNoteOffTime = noteOffTime;
        noteOffTime = -1;
        curState = State::Idle;
        ret = true;
    }
    return ret;
}

void MidiVoice::reset()
{
    noteOffTime = -1;           // the absolute metric time when the 
                                // currently playing note should stop
    curPitch = -100;            // the pitch of the last note played in this voice
    lastNoteOffTime = -1;
    curState = State::Idle;
    retriggerSampleCounter = 0;
    setGate(false);             // and stop the playing CV
}