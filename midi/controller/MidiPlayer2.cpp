
#include "IMidiPlayerHost.h"
#include "MidiLock.h"
#include "MidiPlayer2.h"
#include "MidiSong.h"


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
}

void MidiPlayer2::setSong(std::shared_ptr<MidiSong> newSong)
{
    // Since these calls come in on the UI thread, the UI must have locked us before.
    assert(song->lock->locked());
    assert(newSong->lock->locked());
    song = newSong;
    track = song->getTrack(0);
}

void MidiPlayer2::reset()
{
    isReset = true;
}

void MidiPlayer2::stop()
{
    isPlaying = false;
}

void MidiPlayer2::setNumVoices(int voices)
{
    numVoices = voices;
    voiceAssigner.setNumVoices(voices);
}

double MidiPlayer2::getLoopStart() const
{
    return loopStart;
}

void MidiPlayer2::updateToMetricTime(double metricTime)
{
    if (!isPlaying) {
        return;
    }
    const bool acquiredLock = song->lock->playerTryLock();
    if (acquiredLock) {
        if (song->lock->dataModelDirty()) {
            reset();
        }
        updateToMetricTimeInternal(metricTime);
        song->lock->playerUnlock();
    } else {
        reset();
        host->onLockFailed();
    }
}

void MidiPlayer2::updateToMetricTimeInternal(double metricTime)
{
    // If we had a conflict and needed to reset, then
    // start all over from beginning. Or, if reset initiated by user.
    if (isReset) {
        curEvent = track->begin();
    //    noteOffTime = -1;
        resetAllVoices();
        isReset = false;
        loopStart = 0;
    }
     // keep processing events until we are caught up
    while (playOnce(metricTime)) {

    }
}

bool MidiPlayer2::playOnce(double metricTime)
{
    bool didSomething = false;

    didSomething = pollForNoteOff(metricTime);
    if (didSomething) {
        return true;
    }

    const double eventStart = (loopStart + curEvent->first);
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
                voice->playNote(note->pitchCV, eventStart, note->duration + eventStart);
                ++curEvent;
            }
            break;
            case MidiEvent::Type::End:
                // for now, should loop.
                loopStart += curEvent->first;
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

void MidiPlayer2::resetAllVoices()
{
    for (int i = 0; i < numVoices; ++i) {
        voices[i].reset();
    }
}