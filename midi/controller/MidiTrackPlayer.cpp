#include "MidiTrackPlayer.h"
#include "MidiTrack4Options.h"
#include "MidiSong4.h"
#include "TimeUtils.h"

#include <stdio.h>
#include <assert.h>

MidiTrackPlayer::MidiTrackPlayer(std::shared_ptr<IMidiPlayerHost4> host, int trackIndex, std::shared_ptr<MidiSong4> _song) :
    trackIndex(trackIndex),
    curSectionIndex(0),
    voiceAssigner(voices, 16)
{
    setSong(_song, trackIndex);
    for (int i = 0; i < 16; ++i) {
        MidiVoice& vx = voices[i];
        vx.setHost(host.get());
        vx.setTrack(trackIndex);
        vx.setIndex(i);
    }
#if defined(_MLOG)
    printf("MidiTrackPlayer::ctor() track = %p index=%d\n", track.get(), trackIndex);
#endif
    voiceAssigner.setNumVoices(numVoices);
}

 void MidiTrackPlayer::setNumVoices(int _numVoices)
 {
     this->numVoices = _numVoices;
     voiceAssigner.setNumVoices(numVoices);
 }

void MidiTrackPlayer::setSong(std::shared_ptr<MidiSong4> newSong, int _trackIndex) 
{
    song = newSong;
    curTrack = song->getTrack(trackIndex);
    assert(_trackIndex == trackIndex);      // we don't expect to get re-assigned.

    findFirstTrackSection();
    auto options = song->getOptions(trackIndex, curSectionIndex);
    if (options) {
        sectionLoopCounter = options->repeatCount;
    } else {
        sectionLoopCounter = 1;
    }

#ifdef _MLOG
    if (!track) {
        printf("found nothing to play on track %d\n", trackIndex);
    }
#endif
}

MidiSong4Ptr MidiTrackPlayer::getSong()
{
    return song;
}

void MidiTrackPlayer::findFirstTrackSection()
{
    for (int i = 0; i < 4; ++i) {
        curTrack = song->getTrack(trackIndex, i);
        if (curTrack && curTrack->getLength()) {
            curSectionIndex = i;
            // printf("findFirstTrackSection found %d\n", curSectionIndex); fflush(stdout);
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

    if (!curTrack) {
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
                // printfprintf("just inc curEvent 129\n");
            }
            break;
            case MidiEvent::Type::End:
#if defined(_MLOG)
                printf("MidiTrackPlayer:playOnce index=%d type = end\n", trackIndex);
                fflush(stdout);
#endif
                // for now, should loop.
                currentLoopIterationStart += curEvent->first;


                sectionLoopCounter--;
                if (sectionLoopCounter > 0) {
                    // if still repeating this section..
                    // Then I think all we need to do is reset the pointer.
                    assert(curTrack);
                    curEvent = curTrack->begin();
                } else {
                    if (sectionLoopCounter < 0) {
                        printf("inf not supported yet\n");
                        fflush(stdout);
                    }
                    // If we have reached the end of the repetitions of this section,
                    // then go to the next one.
                    curTrack = nullptr;
                    while (!curTrack) {

                        if (++curSectionIndex > 3) {
                            curSectionIndex = 0;
                        }
                        curTrack = song->getTrack(trackIndex, curSectionIndex);
                        if (curTrack) {
                            auto opts = song->getOptions(trackIndex, curSectionIndex);
                            assert(opts);
                            if (opts) {
                                sectionLoopCounter = opts->repeatCount;
                            } else {
                                sectionLoopCounter = 1;
                            }
                        }
                    }
                }
                assert(curTrack);
                curEvent = curTrack->begin();
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

int MidiTrackPlayer::getSection() const
{
    return curTrack ? curSectionIndex + 1 : 0;
}


void MidiTrackPlayer::setNextSection(int section)
{
    nextSectionIndex = section;
}

int MidiTrackPlayer::getNextSection() const
{
    return nextSectionIndex;
}

void MidiTrackPlayer::reset()
{
    findFirstTrackSection();
  
    curTrack = song->getTrack(trackIndex, curSectionIndex);
    if (curTrack) {
        // can we really handle not having a track?
        curEvent = curTrack->begin();
        //printf("reset put cur event back\n");
    }

    voiceAssigner.reset();
    currentLoopIterationStart = 0;
    auto options = song->getOptions(trackIndex, curSectionIndex);
    sectionLoopCounter = options ? options->repeatCount : 1;
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