#include "MidiTrackPlayer.h"
#include "MidiSong4.h"
#include "MidiTrack4Options.h"
#include "TimeUtils.h"

#ifdef __PLUGIN
#include "engine/Param.hpp"
#include "engine/Port.hpp"
#endif

#include <assert.h>
#include <stdio.h>

MidiTrackPlayer::MidiTrackPlayer(
    std::shared_ptr<IMidiPlayerHost4> host,
    int trackIndex, 
    std::shared_ptr<MidiSong4> _song) : trackIndex(trackIndex),
                                        curSectionIndex(0),
                                        voiceAssigner(voices, 16),
                                        cv0Trigger(false),
                                        cv1Trigger(false) {
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

void MidiTrackPlayer::setNumVoices(int _numVoices) {
    this->numVoices = _numVoices;
    voiceAssigner.setNumVoices(numVoices);
}

void MidiTrackPlayer::setSong(std::shared_ptr<MidiSong4> newSong, int _trackIndex) {
    song = newSong;
    curTrack = song->getTrack(trackIndex);
    assert(_trackIndex == trackIndex);  // we don't expect to get re-assigned.

    findFirstTrackSection();
    auto options = song->getOptions(trackIndex, curSectionIndex);
    if (options) {
        sectionLoopCounter = options->repeatCount;
        // printf("in set song, get sectionLoopCounterfrom options %d\n", sectionLoopCounter);
    } else {
        sectionLoopCounter = 1;
        // printf("in set song, get sectionLoopCounter from default %d\n", sectionLoopCounter);
    }

#ifdef _MLOG
    if (!track) {
        printf("found nothing to play on track %d\n", trackIndex);
    }
#endif
}

MidiSong4Ptr MidiTrackPlayer::getSong() {
    return song;
}

void MidiTrackPlayer::findFirstTrackSection() {
    for (int i = 0; i < 4; ++i) {
        curTrack = song->getTrack(trackIndex, i);
        if (curTrack && curTrack->getLength()) {
            curSectionIndex = i;
            // printf("findFirstTrackSection found %d\n", curSectionIndex); fflush(stdout);
            return;
        }
    }
}

void MidiTrackPlayer::setupToPlayDifferentSection(int section) {
    curTrack = nullptr;
    int nextSection = validateSectionRequest(section);
    curSectionIndex = (nextSection == 0) ? 0 : nextSection - 1;
    // printf("setupToPlayDifferentSection next=%d\n", nextSection);
    // printf("setupToPlayDifferentSection set index to %d\n", curSectionIndex);
    setupToPlayCommon();
    // assert(false);
}

void MidiTrackPlayer::setupToPlayCommon() {
    curTrack = song->getTrack(trackIndex, curSectionIndex);
    if (curTrack) {
        auto opts = song->getOptions(trackIndex, curSectionIndex);
        assert(opts);
        if (opts) {
            sectionLoopCounter = opts->repeatCount;
            // printf("in setup common, get sectionLoopCounter from options %d (tk=%d, sec=%d)\n", sectionLoopCounter, trackIndex, curSectionIndex);
        } else {
            sectionLoopCounter = 1;
            // printf("in setup common, get sectionLoopCounter from defaults %d\n", sectionLoopCounter);
        }
    }
    totalRepeatCount = sectionLoopCounter;
}

void MidiTrackPlayer::setupToPlayNextSection() {
    curTrack = nullptr;
    MidiTrackPtr tk = nullptr;
    while (!tk) {
        if (++curSectionIndex > 3) {
            curSectionIndex = 0;
        }
        // printf("setupToPlayNExt set curSectionIndex to %d\n", curSectionIndex);
        tk = song->getTrack(trackIndex, curSectionIndex);
    }
    setupToPlayCommon();
}

int MidiTrackPlayer::validateSectionRequest(int section) const {
    int nextSection = section;
    if (nextSection == 0) {
        return 0;  // 0 means nothing selected
    }

    for (int tries = 0; tries < 4; ++tries) {
        auto tk = song->getTrack(trackIndex, nextSection - 1);
        if (tk && tk->getLength()) {
            return nextSection;
        }
        // keep it 1..4
        if (++nextSection > 4) {
            nextSection = 1;
        }
    }
    return 0;
}

void MidiTrackPlayer::setNextSection(int section) {
    // printf("called set next section with %d\n", section);
    eventQ.nextSectionIndex = validateSectionRequest(section);
    if (!isPlaying && eventQ.nextSectionIndex) {
        // if we aren't playing, set it in anticipation of starting.
        // If we are playing, the next end event will advance us
        // TODO: should we clear the one in the event Q now?
        curSectionIndex = eventQ.nextSectionIndex - 1;
        // printf("set next section just set curSection to %d\n", curSectionIndex);
    }
}

int MidiTrackPlayer::getNextSection() const {
    return eventQ.nextSectionIndex;
}

void MidiTrackPlayer::resetAllVoices(bool clearGates) {
    for (int i = 0; i < numVoices; ++i) {
        voices[i].reset(clearGates);
    }
}

bool MidiTrackPlayer::playOnce(double metricTime, float quantizeInterval) {
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

    const double eventStart = TimeUtils::quantize(eventStartUnQuantized, quantizeInterval, true);

#if defined(_MLOG) && 1
    printf("MidiTrackPlayer::playOnce index=%d eventStart=%.2f\n", trackIndex, eventStart);
#endif
    if (eventStart <= metricTime) {
        MidiEventPtr event = curEvent->second;
        switch (event->type) {
            case MidiEvent::Type::Note: {
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
            } break;
            case MidiEvent::Type::End:
                onEndOfTrack();
                break;
            default:
                assert(false);
        }
        didSomething = true;
    }
    return didSomething;
}

void MidiTrackPlayer::onEndOfTrack() {
    // printf(" MidiTrackPlayer::onEndOfTrack sectionLoopCounter=%d\n", sectionLoopCounter);
#if defined(_MLOG)
    printf("MidiTrackPlayer:playOnce index=%d type = end\n", trackIndex);
    printf("sectionLoopCounter = %d nextSectionIndex =%d\n", sectionLoopCounter, nextSectionIndex);
    fflush(stdout);
#endif
    // for now, should loop.
    currentLoopIterationStart += curEvent->first;

    // If there is a section change queued up, do it.
    if (eventQ.nextSectionIndex > 0) {
        printf("at end of track, found next section %d\n", eventQ.nextSectionIndex);

        setupToPlayDifferentSection(eventQ.nextSectionIndex);
        eventQ.nextSectionIndex = 0;
        printf("cleared next section cue\n");

        // we need to fold the above into
        // setupToPlayDifferentSection

    } else {
        // counter zero means loop forever
        bool keepLooping = true;
        if (sectionLoopCounter == 0) {
            keepLooping = true;
        } else {
            sectionLoopCounter--;
            keepLooping = (sectionLoopCounter > 0);
            // printf("sectionLoopCounter dec at end %d\n", sectionLoopCounter);
        }

        if (keepLooping) {
            // if still repeating this section..
            // Then I think all we need to do is reset the pointer.
            assert(curTrack);
            curEvent = curTrack->begin();
        } else {
            assert(sectionLoopCounter >= 0);

            // If we have reached the end of the repetitions of this section,
            // then go to the next one.
            setupToPlayNextSection();
            assert(curTrack);
        }
    }

    assert(curTrack);
    curEvent = curTrack->begin();
}

bool MidiTrackPlayer::pollForNoteOff(double metricTime) {
    bool didSomething = false;
    for (int i = 0; i < numVoices; ++i) {
        bool b = voices[i].updateToMetricTime(metricTime);
        if (b) {
            didSomething = true;
            ;
        }
    }
    return didSomething;
}

int MidiTrackPlayer::getSection() const {
    return curTrack ? curSectionIndex + 1 : 0;
}

void MidiTrackPlayer::reset(bool resetSectionIndex) {

    printf("for test ignoring new rest\n");
    resetSectionIndex = false;
    
    if (resetSectionIndex) {
        findFirstTrackSection();
    } else {
        // we don't want reset to erase nextSectionIndex, so
        // re-apply it after reset.
        const int saveSection = eventQ.nextSectionIndex;
        if (saveSection == 0) {
            findFirstTrackSection();
        } else {
            setNextSection(saveSection);
        }
    }

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
    totalRepeatCount = sectionLoopCounter; 
    //printf("sectionLoopCounter set in rest %d\n", sectionLoopCounter);
}

#if 0
double MidiTrackPlayer::getCurrentLoopIterationStart() const
{
    return currentLoopIterationStart;
}
#endif

void MidiTrackPlayer::setSampleCountForRetrigger(int numSamples) {
    for (int i = 0; i < maxVoices; ++i) {
        voices[i].setSampleCountForRetrigger(numSamples);
    }
}

void MidiTrackPlayer::updateSampleCount(int numElapsed) {
    for (int i = 0; i < numVoices; ++i) {
        voices[i].updateSampleCount(numElapsed);
    }
    pollForCVChange();
}

int MidiTrackPlayer::getCurrentRepetition() {
    // just return zero if not playing.
    if (!isPlaying) {
        return 0;
    }
    // printf("getCurrentRepetition clip#=%d, totalRep=%d, counter=%d\n",     this->curSectionIndex, totalRepeatCount, sectionLoopCounter);

    return totalRepeatCount + 1 - sectionLoopCounter;
}

void MidiTrackPlayer::pollForCVChange()
{
    // a lot of unit tests won't set this, so let's handle that
    if (input) {

        auto ch0 = input->getVoltage(0);
        cv0Trigger.go(ch0);
        if (cv0Trigger.trigger()) {
            setNextSection(curSectionIndex + 2);        // add one for next, another one for the command offset
        }

        auto ch1 = input->getVoltage(1);
        cv1Trigger.go(ch1);
        if (cv1Trigger.trigger()) {
            // I don't think this will work for section 0
            //assert(curSectionIndex != 0);
            int nextClip = curSectionIndex;     // because of the offset of 1, this will be prev
            if (nextClip == 0) {
                nextClip = 4;
                assert(false);      // untested?
            }
            setNextSection(nextClip);       
        }
    }
}
