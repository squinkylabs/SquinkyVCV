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

void MidiPlayer4::setSong(std::shared_ptr<MidiSong4> newSong)
{

    assert(song->lock->locked());
    assert(newSong->lock->locked());
  //  song = newSong;
  //  track = song->getTrack(0);

    song = newSong;
    for (int i = 0; i<MidiSong4::numTracks; ++i) {
        trackPlayers[i]->setSong(song, i);
    }
}

 MidiSong4Ptr MidiPlayer4::getSong()
{
    return trackPlayers[0]->getSong();
}

void MidiPlayer4::updateToMetricTime(double metricTime, float quantizationInterval, bool running)
{
#if defined(_MLOG) && 1
    printf("MidiPlayer4::updateToMetricTime metrict=%.2f, quantizInt=%.2f\n", metricTime, quantizationInterval);
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
    metricTime = TimeUtils::quantize(metricTime, quantizationInterval, true);
    // If we had a conflict and needed to reset, then
    // start all over from beginning. Or, if reset initiated by user.
    if (isReset) {

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

double MidiPlayer4::getCurrentLoopIterationStart(int track) const
{
    auto tkPlayer = trackPlayers[track];
    return tkPlayer->getCurrentLoopIterationStart();
}

void MidiPlayer4::reset(bool clearGates)
{
    isReset = true;
    isResetGates = clearGates;
}

int MidiPlayer4::getSectionIndex(int track) const
{
    return trackPlayers[track]->getSectionIndex();
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
 
void MidiPlayer4::setNumVoices(int track, int numVoices)
{
    // printf("set num vc %d, %d\n", track, numVoices);
    // fflush(stdout);
    assert(track>=0 && track < 4);
    assert(numVoices >=1 && numVoices <= 16);
  
    trackPlayers[track]->setNumVoices(numVoices);

}

void MidiPlayer4::setSampleCountForRetrigger(int count)
{
     for (int i=0; i < MidiSong4::numTracks; ++i) {
        auto trackPlayer = trackPlayers[i];
        assert(trackPlayer);
        trackPlayer->setSampleCountForRetrigger(count);
    }
}

void MidiPlayer4::updateSampleCount(int numElapsed)
{
     for (int i=0; i < MidiSong4::numTracks; ++i) {
        auto trackPlayer = trackPlayers[i];
        assert(trackPlayer);
        trackPlayer->updateSampleCount(numElapsed);
    }
}