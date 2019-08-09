
#include "IMidiPlayerHost.h"
#include "MidiLock.h"
#include "MidiPlayer2.h"
#include "MidiSong.h"
#include "TimeUtils.h"


MidiPlayer2::MidiPlayer2(std::shared_ptr<IMidiPlayerHost> host, std::shared_ptr<MidiSong> song) :
    host(host),
    song(song),
    voiceAssigner(voices, 16),
    track(song->getTrack(0))
{
    for (int i = 0; i < 16; ++i) {
        MidiVoice& vx = voices[i];
        vx.setHost(host.get());
        vx.setIndex(i);
    }
//    loopParams = nullptr;
}

void MidiPlayer2::setSong(std::shared_ptr<MidiSong> newSong)
{
    // Since these calls come in on the UI thread, the UI must have locked us before.
    assert(song->lock->locked());
    assert(newSong->lock->locked());
    song = newSong;
    track = song->getTrack(0);
}

void MidiPlayer2::reset(bool clearGates)
{
    isReset = true;
    isResetGates = clearGates;
}

void MidiPlayer2::setNumVoices(int voices)
{
    numVoices = voices;
    voiceAssigner.setNumVoices(voices);
}

void MidiPlayer2::setSampleCountForRetrigger(int samples)
{
    for (int i = 0; i < maxVoices; ++i) {
        voices[i].setSampleCountForRetrigger(samples);
    }
}

void MidiPlayer2::updateSampleCount(int numElapsed)
{
    for (int i = 0; i < numVoices; ++i) {
        voices[i].updateSampleCount(numElapsed);
    }
}


double MidiPlayer2::getCurrentLoopIterationStart() const
{
    return currentLoopIterationStart;
}

void MidiPlayer2::updateToMetricTime(double metricTime, float quantizationInterval)
{
#ifdef _MLOG
    printf("MidiPlayer::updateToMetricTime metrict=%.2f, quantizInt=%.2f\n", metricTime, quantizationInterval);
#endif
    assert(quantizationInterval != 0);

    if (!isPlaying) {
        return;
    }
    const bool acquiredLock = song->lock->playerTryLock();
    if (acquiredLock) {
        if (song->lock->dataModelDirty()) {
            reset(false);
        }
        updateToMetricTimeInternal(metricTime, quantizationInterval);
        song->lock->playerUnlock();
    } else {
        reset(false);
        host->onLockFailed();
    }
}

void MidiPlayer2::updateToMetricTimeInternal(double metricTime, float quantizationInterval)
{
    metricTime = TimeUtils::quantize(metricTime, quantizationInterval, true);
    // If we had a conflict and needed to reset, then
    // start all over from beginning. Or, if reset initiated by user.
    if (isReset) {
       //  printf("\n******  player proc reset\n");
        curEvent = track->begin();
        resetAllVoices(isResetGates);
        isReset = false;
        isResetGates = false;
        currentLoopIterationStart = 0;
    }


    // To implement loop start, we just push metric time up to where we want to start.
    // TODO: skip over initial stuff?
    
    if (song->getSubrangeLoop().enabled) {
   // if (loopParams && loopParams.load()->enabled) {
        metricTime += song->getSubrangeLoop().startTime;
    }
     // keep processing events until we are caught up
    while (playOnce(metricTime, quantizationInterval)) {

    }
}

bool MidiPlayer2::playOnce(double metricTime, float quantizeInterval)
{
#ifdef _MLOG
    printf("MidiPlayer::playOnce metrict=%.2f, quantizInt=%.2f\n", metricTime, quantizeInterval);
#endif
    bool didSomething = false;

    didSomething = pollForNoteOff(metricTime);
    if (didSomething) {
        return true;
    }

    // push the start time up by loop start, so that event t==loop start happens at start of loop
    const double eventStartUnQuantized = (currentLoopIterationStart + curEvent->first);

    // Treat loop end just like track end. loop back around
    // when we pass then end.
    if (song->getSubrangeLoop().enabled) {
        auto loopEnd = song->getSubrangeLoop().endTime + currentLoopIterationStart;
        if (loopEnd <= metricTime) {
            currentLoopIterationStart += (song->getSubrangeLoop().endTime - song->getSubrangeLoop().startTime);
            curEvent = track->begin();
            return true;
        }
    }

    const double eventStart = TimeUtils::quantize(eventStartUnQuantized, quantizeInterval, true);
    if (eventStart <= metricTime) {
        MidiEventPtr event = curEvent->second;
        switch (event->type) {
            case MidiEvent::Type::Note:
            {
                MidiNoteEventPtr note = safe_cast<MidiNoteEvent>(event);
  
                // find a voice to play
                MidiVoice* voice = voiceAssigner.getNext(note->pitchCV);
                assert(voice);

                // play the note
                double quantizedNoteEnd = TimeUtils::quantize(note->duration + eventStart, quantizeInterval, false);
                voice->playNote(note->pitchCV, float(eventStart), float(quantizedNoteEnd));
                ++curEvent;
            }
            break;
            case MidiEvent::Type::End:
                // for now, should loop.
                currentLoopIterationStart += curEvent->first;
                curEvent = track->begin();
                break;
            default:
                assert(false);
        }
        didSomething = true;
    }
    return didSomething;
}

bool MidiPlayer2::pollForNoteOff(double metricTime)
{
    bool didSomething = false;
    for (int i = 0; i < numVoices; ++i) {
        bool b = voices[i].updateToMetricTime(metricTime);
        if (b) {
            didSomething = true;;
        }
    }
    return didSomething;
}

void MidiPlayer2::resetAllVoices(bool clearGates)
{
    for (int i = 0; i < numVoices; ++i) {
        voices[i].reset(clearGates);
    }
}