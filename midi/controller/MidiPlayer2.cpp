
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

}

void MidiPlayer2::reset()
{
    isReset = true;
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
    assert(false);      // need to play to pass next test
}