#include "IMidiPlayerHost.h"
#include "MidiLock.h"
#include "MidiPlayer4.h"
#include "MidiSong4.h"
#include "MidiTrackPlayer.h"
#include "TimeUtils.h"

MidiPlayer4::MidiPlayer4(std::shared_ptr<IMidiPlayerHost4> host, std::shared_ptr<MidiSong4> song) :
    song(song),
    host(host)
{
//MidiTrackPlayerPtr
    for (int i = 0; i<MidiSong4::numTracks; ++i) {
        trackPlayers.push_back( std::make_shared<MidiTrackPlayer>(host, i, song));
    }
}

void MidiPlayer4::setSong(std::shared_ptr<MidiSong4> song)
{
    printf("setSong nimp\n");
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

void MidiPlayer4::updateToMetricTimeInternal(double metricTime, float quantizationInterval)
{
    printf("updateToMetricTimeInternal \n");
    metricTime = TimeUtils::quantize(metricTime, quantizationInterval, true);
    // If we had a conflict and needed to reset, then
    // start all over from beginning. Or, if reset initiated by user.
    if (isReset) {
        printf("\nupdatetometrictimeinternal  player proc reset. We need to do this in the track players?\n");
 
        for (int i=0; i < MidiSong4::numTracks; ++i) {
            auto trackPlayer = trackPlayers[i];
            trackPlayer->reset();
        }
        // curEvent = track->begin();
        resetAllVoices(isResetGates);
        //voiceAssigner.reset();
        isReset = false;
        isResetGates = false;
        // currentLoopIterationStart = 0;
    }


    // To implement loop start, we just push metric time up to where we want to start.
    // TODO: skip over initial stuff?
#if 0   // player 4 has no subrange, right?
    if (song->getSubrangeLoop().enabled) {
   // if (loopParams && loopParams.load()->enabled) {
        metricTime += song->getSubrangeLoop().startTime;
    }
#endif
     // keep processing events until we are caught up
    for (int i=0; i < MidiSong4::numTracks; ++i) {
        auto trackPlayer = trackPlayers[i];
        assert(trackPlayer);
        while (trackPlayer->playOnce(metricTime, quantizationInterval)) {
        }
    }
}

double MidiPlayer4::getCurrentLoopIterationStart() const
{
    printf("getCurrentLoopIterationStart nimp\n");
    return 0;
}

 void MidiPlayer4::reset(bool clearGates)
 {
    printf("reset nimp\n");
    isReset = true;
    isResetGates = clearGates;
 }

 void MidiPlayer4::resetAllVoices(bool clearGates)
{
    for (int i = 0; i<MidiSong4::numTracks; ++i) {
        auto tkPlayer = trackPlayers[i];
        if (tkPlayer) {
            tkPlayer->resetAllVoices(clearGates);
        }
    }
}
 