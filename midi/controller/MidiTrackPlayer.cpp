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
    findFirstTrackSection();
    for (int i = 0; i < 16; ++i) {
        MidiVoice& vx = voices[i];
        vx.setHost(host.get());
        vx.setTrack(trackIndex);
        vx.setIndex(i);
    }
#if defined(_MLOG)
    printf("MidiTrackPlayer::ctor() track = %p index=%d\n", track.get(), trackIndex);
#endif
}

 void MidiTrackPlayer::setNumVoices(int _numVoices)
 {
     this->numVoices = _numVoices;
     voiceAssigner.setNumVoices(numVoices);
 }

void MidiTrackPlayer::setSong(std::shared_ptr<MidiSong4> newSong, int _trackIndex) 
{
    song = newSong;

    findFirstTrackSection();
#ifdef _MLOG
    if (!track) {
        printf("found nothing to play on track %d\n", trackIndex);
    }
#endif

    assert(trackIndex == _trackIndex);
    curSectionIndex = 0;
}

MidiSong4Ptr MidiTrackPlayer::getSong()
{
    return song;
}

void MidiTrackPlayer::findFirstTrackSection()
{
    for (int i = 0; i < 4; ++i) {
        track = song->getTrack(trackIndex, i);
        if (track && track->getLength()) {
            curSectionIndex = i;
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
  #if defined(_MLOG) && 1
    printf("MidiTrackPlayer::playOnce index=%d metrict=%.2f, quantizInt=%.2f track=%p\n", 
        trackIndex, metricTime, quantizeInterval, track.get());
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

#if defined(_MLOG) && 1
    printf("MidiTrackPlayer::playOnce index=%d eventStart=%.2f\n", trackIndex, eventStart);
#endif
    if (eventStart <= metricTime) {
        MidiEventPtr event = curEvent->second;
        switch (event->type) {
            case MidiEvent::Type::Note:
            {
#if defined(_MLOG)
                printf("MidiTrackPlayer:playOnce index=%d type = note\n", trackIndex);
#endif
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
#if defined(_MLOG)
                printf("MidiTrackPlayer:playOnce index=%d type = end\n", trackIndex);
#endif
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
#if 1 // new version
    findFirstTrackSection();
#else
    assert(curSectionIndex == 0);          // should we use cur section index? nothing?
    curSectionIndex = 0;
#endif
  
    track = song->getTrack(trackIndex, curSectionIndex);
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