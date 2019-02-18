
#include "MidiLock.h"
#include "MidiPlayer.h"
#include "MidiSong.h"


MidiPlayer::MidiPlayer(std::shared_ptr<IPlayerHost> host, std::shared_ptr<MidiSong> song) :
    host(host), song(song), trackPlayer(song->getTrack(0))
{
    ++_mdb;
}

void MidiPlayer::setSong(std::shared_ptr<MidiSong> newSong)
{
    // Since these calls come in on the UI thread, the UI must have locked us before.
    assert(song->lock->locked());
    assert(newSong->lock->locked());
    song = newSong;
    trackPlayer.setTrack(newSong->getTrack(0));
}

void MidiPlayer::updateToMetricTime(double metricTime)
{
    if (!isPlaying) {
        return;
    }
    const bool acquiredLock = song->lock->playerTryLock();
    if (acquiredLock) {
        if (song->lock->dataModelDirty()) {
            trackPlayer.reset();
        }
        trackPlayer.updateToMetricTime(metricTime, host.get());
        song->lock->playerUnlock();
    } else {
        trackPlayer.reset();
        host->onLockFailed();
    }
}

TrackPlayer::TrackPlayer(MidiTrackPtr track) : track(track)
{
}

TrackPlayer::~TrackPlayer()
{
   //curEvent = nullptr;
    
}

void TrackPlayer::updateToMetricTime(double time, IPlayerHost* host)
{
    // If we had a conflict and needed to reset, then
    // start all over from beginning
    if (isReset) {
        curEvent = track->begin();
        noteOffTime = -1;
        isReset = false;
        loopStart = 0;
    }
    // keep processing events until we are caught up
    while (playOnce(time, host)) {

    }
}

bool TrackPlayer::playOnce(double metricTime, IPlayerHost* host)
{
    bool didSomething = false;

    if (noteOffTime >= 0 && noteOffTime <= metricTime) {
        host->setGate(false);
        noteOffTime = -1;
        didSomething = true;
    }

    const double eventStart = (loopStart + curEvent->first);
    if (eventStart <= metricTime) {
        MidiEventPtr event = curEvent->second;
        switch (event->type) {
            case MidiEvent::Type::Note:
            {
                MidiNoteEventPtr note = safe_cast<MidiNoteEvent>(event);
                // should now output the note.
                host->setGate(true);
                host->setCV(note->pitchCV);

                // and save off the note-off time.
                noteOffTime = note->duration + eventStart;
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
