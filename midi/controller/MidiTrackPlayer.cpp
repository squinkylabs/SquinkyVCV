#include "MidiTrackPlayer.h"
#include "MidiSong4.h"
#include "TimeUtils.h"

#include <stdio.h>
#include <assert.h>

MidiTrackPlayer::MidiTrackPlayer(std::shared_ptr<IMidiPlayerHost4> host, int trackIndex, std::shared_ptr<MidiSong4> _song) :
    song(_song),
    voiceAssigner(voices, 16),
    track(song->getTrack(trackIndex)),
    trackIndex(trackIndex),
    curSectionIndex(0)
{
    findFirstTrack();
    for (int i = 0; i < 16; ++i) {
        MidiVoice& vx = voices[i];
        vx.setHost(host.get());
        vx.setTrack(trackIndex);
        vx.setIndex(i);
    }
}

 void MidiTrackPlayer::setNumVoices(int _numVoices)
 {
     this->numVoices = _numVoices;
     voiceAssigner.setNumVoices(numVoices);
 }

void MidiTrackPlayer::setSong(std::shared_ptr<MidiSong4> newSong, int _trackIndex) 
{
    song = newSong;
    //track = song->getTrack(trackIndex);
    findFirstTrack();
    if (!track) {
        printf("found nothing to play on track %d\n", trackIndex);
    }
 //   assert(track);
    assert(trackIndex == _trackIndex);
    curSectionIndex = 0;
}

void MidiTrackPlayer::findFirstTrack()
{
    for (int i = 0; i < 4; ++i) {
        auto track = song->getTrack(trackIndex, i);
        if (track && track->getLength()) {
            return;
        }
    }
}

void MidiTrackPlayer::resetAllVoices(bool clearGates)
{
        for (int i = 0; i < numVoices; ++i) {
        voices[i].reset(clearGates);
    }
}

bool MidiTrackPlayer::playOnce(double metricTime, float quantizeInterval)
{
  #if defined(_MLOG) && 0
    printf("MidiPlayer::playOnce metrict=%.2f, quantizInt=%.2f\n", metricTime, quantizeInterval);
#endif
    bool didSomething = false;

    didSomething = pollForNoteOff(metricTime);
    if (didSomething) {
        return true;
    }

    if (!track) {
                            // should be possible if we keep int curPlaybackSection
        return false;
    }

    // push the start time up by loop start, so that event t==loop start happens at start of loop
    const double eventStartUnQuantized = (currentLoopIterationStart + curEvent->first);

    // Treat loop end just like track end. loop back around
    // when we pass then end.
#if 0   // we don't have subrange loop
    if (song->getSubrangeLoop().enabled) {
        auto loopEnd = song->getSubrangeLoop().endTime + currentLoopIterationStart;
        if (loopEnd <= metricTime) {
            currentLoopIterationStart += (song->getSubrangeLoop().endTime - song->getSubrangeLoop().startTime);
            curEvent = track->begin();
            return true;
        }
    }
#endif

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
                const double durationQuantized = TimeUtils::quantize(note->duration, quantizeInterval, false);  
                double quantizedNoteEnd = TimeUtils::quantize(durationQuantized + eventStart, quantizeInterval, false);
                voice->playNote(note->pitchCV, float(eventStart), float(quantizedNoteEnd));
                ++curEvent;
            }
            break;
            case MidiEvent::Type::End:
                // for now, should loop.
                currentLoopIterationStart += curEvent->first;
                track = nullptr;
                while (!track) {
                    if (++curSectionIndex > 3) {
                        curSectionIndex = 0;
                    }
                    track = song->getTrack(trackIndex, curSectionIndex);
                }
                curEvent = track->begin();
                break;
            default:
                assert(false);
        }
        didSomething = true;
    }
    return didSomething;
}

bool MidiTrackPlayer::pollForNoteOff(double metricTime)
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

void MidiTrackPlayer::reset()
{
    curSectionIndex = 0;
    track = song->getTrack(trackIndex, 0);
    if (track) {
        // can we really handle not having a track?
        curEvent = track->begin();
    }

    voiceAssigner.reset();
    currentLoopIterationStart = 0;
}

double MidiTrackPlayer::getCurrentLoopIterationStart() const
{
    return currentLoopIterationStart;
}

void MidiTrackPlayer::setSampleCountForRetrigger(int numSamples)
{
    for (int i = 0; i < maxVoices; ++i) {
        voices[i].setSampleCountForRetrigger(numSamples);
    }
}

void MidiTrackPlayer::updateSampleCount(int numElapsed)
{
    for (int i = 0; i < numVoices; ++i) {
        voices[i].updateSampleCount(numElapsed);
    }
}