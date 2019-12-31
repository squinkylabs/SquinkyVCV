#include "IMidiPlayerHost.h"
#include "MidiLock.h"
#include "MidiPlayer4.h"
#include "MidiSong4.h"
#include "MidiTrackPlayer.h"

MidiPlayer4::MidiPlayer4(std::shared_ptr<IMidiPlayerHost4> host, std::shared_ptr<MidiSong4> song) :
    song(song),
    host(host)
{
//MidiTrackPlayerPtr
    for (int i = 0; i<MidiSong4::numTracks; ++i) {
        trackPlayers.push_back( std::make_shared<MidiTrackPlayer>());
    }
}

void MidiPlayer4::updateToMetricTime(double metricTime, float quantizationInterval, bool running)
{
#if defined(_MLOG) && 0
    printf("MidiPlayer::updateToMetricTime metrict=%.2f, quantizInt=%.2f\n", metricTime, quantizationInterval);
#endif
    assert(quantizationInterval != 0);

    if (!running) {
        // If seq is paused, leave now so we don't act on the dirty flag when stopped.
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

void MidiPlayer4::updateToMetricTimeInternal(double, float)
{
    printf("updateToMetricTimeInternal nimp\n");
}

double MidiPlayer4::getCurrentLoopIterationStart() const
{
    printf("getCurrentLoopIterationStart nimp\n");
    return 0;
}

 void MidiPlayer4::reset(bool clearGates)
 {
     printf("reset nimp\n");
 }