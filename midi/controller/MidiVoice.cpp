
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
        printf("midi voice will subtract %d from %d\n", samples, retriggerSampleCounter);
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
    printf("\nMidiVoice::playNote curt=%f, lastnot=%f\n", currentTime, lastNoteOffTime);
    // do re-triggering, if needed
    if (currentTime == lastNoteOffTime) {
        assert(numSamplesInRetrigger);
        printf(" mv retrigger. interval = %d\n", numSamplesInRetrigger);
        curState = State::ReTriggering;
        setGate(false);
        delayedNotePitch = pitch;
        delayedNoteEndtime = endTime;
        retriggerSampleCounter = numSamplesInRetrigger;
        printf("voice retric count = %d\n", retriggerSampleCounter);
    } else {
        printf("don't retrigger\n");
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

        printf("shutting off note in update, grabbing last = %f\n", noteOffTime);
        printf(" (the note off time was %.2f, metric = %.2f\n", noteOffTime, metricTime);
        // should probably use metric time here - the time it "acutally" played.
        lastNoteOffTime = noteOffTime; 
        //lastNoteOffTime = metricTime;
        noteOffTime = -1;
        curState = State::Idle;
        ret = true;
    }
    return ret;
}

void MidiVoice::reset(bool clearGate)
{
    noteOffTime = -1;           // the absolute metric time when the 
                                // currently playing note should stop
    curPitch = -100;            // the pitch of the last note played in this voice
    lastNoteOffTime = -1;
    curState = State::Idle;
    retriggerSampleCounter = 0;
    if (clearGate) {
        setGate(false);             // and stop the playing CV
    }
}