#include "IMidiPlayerHost.h"
#include "MidiLock.h"
#include "MidiPlayer4.h"
#include "MidiSong4.h"
#include "MidiTrackPlayer.h"
#include "TimeUtils.h"

#include "engine/Port.hpp"


MidiPlayer4::MidiPlayer4(std::shared_ptr<IMidiPlayerHost4> host, std::shared_ptr<MidiSong4> song) :
    song(song),
    host(host)
{
    for (int i = 0; i<MidiSong4::numTracks; ++i) {
        trackPlayers.push_back( std::make_shared<MidiTrackPlayer>(host, i, song));
    }
    step();         // let's get the song registered, etc.. will at least make old tests happy.
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

void MidiPlayer4::setRunningStatus(bool running)
{
    assert(this);
    for (int i = 0; i < MidiSong4::numTracks; ++i) {
        trackPlayers[i]->setRunningStatus(running);
    }
}

void MidiPlayer4::setPorts(Input* inputPorts, Param* triggerImmediate)
{
    for (int i = 0; i < MidiSong4::numTracks; ++i) {
        trackPlayers[i]->setPorts(inputPorts + i, triggerImmediate);
    }
}

void MidiPlayer4::step()
{
 for (int i = 0; i < MidiSong4::numTracks; ++i) {
        trackPlayers[i]->step();
    }
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
#if 0
    static double lastMt = 0;
    static int seq = 0;
    if (metricTime > (lastMt + .65)) {
        lastMt = metricTime;
        // printf("forcing debug reset %d\n", seq++);  fflush(stdout);
        reset(true, false);
    }
#endif

    const bool acquiredLock = song->lock->playerTryLock();
    if (acquiredLock) {
        if (song->lock->dataModelDirty()) {
            reset(true, false);         // reset all the gates. we need to do this, because we
                                        // only replay the current section, so we are probably at a 
                                        // different rotation when reset, than when played straight
        }
        updateToMetricTimeInternal(metricTime, quantizationInterval);
        song->lock->playerUnlock();
    } else {
        reset(true, false);
        host->onLockFailed();
    }
}

void MidiPlayer4::updateToMetricTimeInternal(double metricTime, float quantizationInterval)
{
    metricTime = TimeUtils::quantize(metricTime, quantizationInterval, true);
    // If user (cv) has requested a reset
    if (isReset) {

        for (int i=0; i < MidiSong4::numTracks; ++i) {
            auto trackPlayer = trackPlayers[i];
            trackPlayer->reset(isResetGates, isResetSectionIndex);
        }

        resetAllVoices(isResetGates);       // we've always done this. It's a little weird to do it here,
                                            // but maybe belt and suspenders?
        isReset = false;
        isResetGates = false;
        isResetSectionIndex = false;
    }

     // keep processing events until we are caught up
    for (int i=0; i < MidiSong4::numTracks; ++i) {
        auto trackPlayer = trackPlayers[i];
        assert(trackPlayer);
        while (trackPlayer->playOnce(metricTime, quantizationInterval)) {
        }
    }
}

MidiTrackPlayerPtr MidiPlayer4::getTrackPlayer(int track)
{
    return trackPlayers[track];
}

void MidiPlayer4::reset(bool clearGates, bool resetSectionIndex)
{
    isReset = true;
    isResetGates = clearGates;
    isResetSectionIndex = resetSectionIndex;
}

int MidiPlayer4::getSection(int track) const
{
    return trackPlayers[track]->getSection();
}

int MidiPlayer4::getNextSectionRequest(int track) const
{
    return trackPlayers[track]->getNextSectionRequest();
}

void MidiPlayer4::setNextSectionRequest(int track, int section)
{
    trackPlayers[track]->setNextSectionRequest(section);
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