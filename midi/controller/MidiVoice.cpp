
#include "IMidiPlayerHost.h"
#include "MidiVoice.h"

#include <assert.h>
#include <stdio.h>

MidiVoice::State MidiVoice::state() const
{
    return curState;
}

void MidiVoice::setHost(IMidiPlayerHost4* ph)
{
    host = ph;
}

void MidiVoice::setIndex(int i)
{
    index = i;
}

void MidiVoice::setTrack(int i)
{
    track = i;
}

void MidiVoice::setGate(bool g)
{
   // printf("mv::setGate(%d) %d\n ", index, g);
    host->setGate(track, index, g);
}

void MidiVoice::setCV(float cv)
{
    host->setCV(track, index, cv);
}

float MidiVoice::pitch() const
{
    return curPitch;
}

void MidiVoice::_setState(State s)
{
    curState = s;
}

void MidiVoice::setSampleCountForRetrigger(int samples)
{
    numSamplesInRetrigger = samples;
}

void MidiVoice::updateSampleCount(int samples)
{
    if (retriggerSampleCounter) {
#ifdef _MLOG
        printf("midi voice will subtract %d from %d\n", samples, retriggerSampleCounter);
#endif
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
#ifdef _MLOG
    printf("\nMidiVoice[%d]::playNote curt=%f, end time = %f, lastnot=%f\n", index, currentTime, endTime, lastNoteOffTime);
#endif
    // do re-triggering, if needed
    if (currentTime == lastNoteOffTime) {
        assert(numSamplesInRetrigger);
#ifdef _MLOG
        printf(" mv retrigger. interval = %d\n", numSamplesInRetrigger);
#endif
        curState = State::ReTriggering;

       // printf("gate low in normal gate off logic\n");
        setGate(false);
        delayedNotePitch = pitch;
        delayedNoteEndtime = endTime;
        retriggerSampleCounter = numSamplesInRetrigger;
#ifdef _MLOG
        printf("voice retric count = %d\n", retriggerSampleCounter);
#endif
    } else {
#ifdef _MLOG
        printf("don't retrigger\n");
#endif
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
#ifdef _MLOG
        printf("shutting off note in MidiVoice::updateToMetricTime, grabbing last = %f (cur NoteOff time) \n", noteOffTime);
        printf(" (the note off time was %.2f, metric = %.2f\n", noteOffTime, metricTime);
#endif
       // printf("gate off in normal update\n");
        setGate(false);
        // should probably use metric time here - the time it "actually" played.
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
#ifdef _MLOG
    printf("reset midi voice %d, will forget last note off time\n", index);
#endif    
    noteOffTime = -1;           // the absolute metric time when the 
                                // currently playing note should stop
    curPitch = -100;            // the pitch of the last note played in this voice
    lastNoteOffTime = -1;
    curState = State::Idle;
    retriggerSampleCounter = 0;
    if (clearGate) {
       // printf("gate off from reset call\n");
        setGate(false);             // and stop the playing CV
    }
}

int MidiVoice::_getIndex() const
{
    return index;
}