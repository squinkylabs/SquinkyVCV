
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
    printf("MidiPLayer::setSong\n"); fflush(stdout);
    // Since these calls come in on the UI thread, the UI must have locked us before.
    assert(song->lock->locked());
    assert(newSong->lock->locked());
    song = newSong;
    trackPlayer.setTrack(newSong->getTrack(0));
}

void MidiPlayer::updateToMetricTime(double metricTime)
{
    static int lastTime = -100;
    int thisTime = int(metricTime);
    if (thisTime != lastTime) {
        printf("player update t = (%f)\n", metricTime);
        lastTime = thisTime;
    }
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
    printf("MidiTrackPlayer ctor, track = %p\n", track.get());
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
        printf("in reset, set start to 0\n");
    }
    // keep processing events until we are caught up
    while (playOnce(time, host)) {

    }
}

bool TrackPlayer::playOnce(double metricTime, IPlayerHost* host)
{
    bool didSomething = false;

    if (noteOffTime >= 0 && noteOffTime <= metricTime) {
        printf("get gate off");
        host->setGate(false);
        noteOffTime = -1;
        didSomething = true;
    }

    static double lastEventStart = -100;
    const double eventStart = (loopStart + curEvent->first);
    if (eventStart != lastEventStart) {
        printf("TrackPlayer::eventSTart = %f loop = %f\n", eventStart, loopStart);
        lastEventStart = eventStart;
    }
    if (eventStart <= metricTime) {
        printf("will play event!\n");
        MidiEventPtr event = curEvent->second;
        switch (event->type) {
            case MidiEvent::Type::Note:
            {
                printf("setting gate high\n");
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
                printf("will play end. track has %d events\n", (int) track->size());
                // for now, should loop.
                // uh oh!
               // assert(false);
               // curMetricTime = 0;
              //  trackPlayStatus.curEvent = song->getTrack(0)->begin();
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
