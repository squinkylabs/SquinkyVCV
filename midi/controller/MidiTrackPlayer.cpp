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

class PlayTracker
{
public:
    PlayTracker(bool& _flag) : flag(_flag) {
        assert(!flag);
        flag = true;
    }
    ~PlayTracker() {
        assert(flag);
        flag = false;
    }
private:
    bool& flag;
};


void MidiTrackPlayer::setSong(std::shared_ptr<MidiSong4> newSong, int _trackIndex) {
    assert(_trackIndex == constTrackIndex);     // we don't expect anyone to change this
    eventQ.newSong = newSong;                   // queue up the song to be moved on next play call
}

// TODO: move this all the playback
#if 0
void MidiTrackPlayer::setSong(std::shared_ptr<MidiSong4> newSong, int _trackIndex) {
    song = newSong;
    curTrack = song->getTrack(constTrackIndex);
    assert(_trackIndex == constTrackIndex);  // we don't expect to get re-assigned.

    setupToPlayFirstTrackSection();
    auto options = song->getOptions(constTrackIndex, playback.curSectionIndex);
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
#endif


// TODO: move to playback code section
int MidiTrackPlayer::validateSectionRequest(int section) const {
    int nextSection = section;
    if (nextSection == 0) {
        return 0;  // 0 means nothing selected
    }

    assert(playback.song);
    for (int tries = 0; tries < 4; ++tries) {
        auto tk = playback.song->getTrack(constTrackIndex, nextSection - 1);
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

void MidiTrackPlayer::resetAllVoices(bool clearGates) {
    for (int i = 0; i < numVoices; ++i) {
        voices[i].reset(clearGates);
    }
}

void MidiTrackPlayer::reset(bool resetSectionIndex) {
    eventQ.resetSections = resetSectionIndex;
    eventQ.reset = true;
}

// this probably won't even work until we put it in playback.
#if 0
void MidiTrackPlayer::reset(bool resetSectionIndex) {

    printf("for test ignoring new rest\n");
    resetSectionIndex = false;
    
    if (resetSectionIndex) {
        setupToPlayFirstTrackSection();
    } else {
        // we don't want reset to erase nextSectionIndex, so
        // re-apply it after reset.
        const int saveSection = eventQ.nextSectionIndex;
        if (saveSection == 0) {
            setupToPlayFirstTrackSection();
        } else {
            setNextSection(saveSection);
        }
    }


    curTrack = playback.song->getTrack(constTrackIndex, playback.curSectionIndex);
    if (curTrack) {
        // can we really handle not having a track?
        curEvent = curTrack->begin();
        //printf("reset put cur event back\n");
    }

    voiceAssigner.reset();
    currentLoopIterationStart = 0;
    auto options = playback.song->getOptions(constTrackIndex, playback.curSectionIndex);
    sectionLoopCounter = options ? options->repeatCount : 1;
    totalRepeatCount = sectionLoopCounter; 
    //printf("sectionLoopCounter set in rest %d\n", sectionLoopCounter);
}
#endif


/*********************************** code possibly called from UI thread ***********************/

MidiTrackPlayer::MidiTrackPlayer(
    std::shared_ptr<IMidiPlayerHost4> host,
    int trackIndex, 
    std::shared_ptr<MidiSong4> _song) : constTrackIndex(trackIndex),
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

void MidiTrackPlayer::setNextSection(int section) {
    // printf("called set next section with %d\n", section);
    eventQ.nextSectionIndex = validateSectionRequest(section);
    if (!isPlaying && eventQ.nextSectionIndex) {
        // if we aren't playing, set it in anticipation of starting.
        // If we are playing, the next end event will advance us
        // TODO: should we clear the one in the event Q now?
        playback.curSectionIndex = eventQ.nextSectionIndex - 1;
        // printf("set next section just set curSection to %d\n", curSectionIndex);
    }
}

int MidiTrackPlayer::getNextSection() const {
    return eventQ.nextSectionIndex;
}

int MidiTrackPlayer::getSection() const {
    return curTrack ? playback.curSectionIndex + 1 : 0;
}

int MidiTrackPlayer::getCurrentRepetition() {
    // just return zero if not playing.
    if (!isPlaying) {
        return 0;
    }
    // printf("getCurrentRepetition clip#=%d, totalRep=%d, counter=%d\n",     this->curSectionIndex, totalRepeatCount, sectionLoopCounter);

    return totalRepeatCount + 1 - sectionLoopCounter;
}

void MidiTrackPlayer::setSampleCountForRetrigger(int numSamples) {
    for (int i = 0; i < maxVoices; ++i) {
        voices[i].setSampleCountForRetrigger(numSamples);
    }
}

// maybe we should be rid of this accessor??
MidiSong4Ptr MidiTrackPlayer::getSong() {
    return playback.song;
}

/******************************* non-playback code typically called from proc ******************/

void MidiTrackPlayer::updateSampleCount(int numElapsed) {
    for (int i = 0; i < numVoices; ++i) {
        voices[i].updateSampleCount(numElapsed);
    }
    pollForCVChange();
}

void MidiTrackPlayer::pollForCVChange()
{
    // a lot of unit tests won't set this, so let's handle that
    if (input) {

        auto ch0 = input->getVoltage(0);
        cv0Trigger.go(ch0);
        if (cv0Trigger.trigger()) {
            setNextSection(playback.curSectionIndex + 2);        // add one for next, another one for the command offset
        }

        auto ch1 = input->getVoltage(1);
        cv1Trigger.go(ch1);
        if (cv1Trigger.trigger()) {
            // I don't think this will work for section 0
            //assert(curSectionIndex != 0);
            int nextClip = playback.curSectionIndex;     // because of the offset of 1, this will be prev
            if (nextClip == 0) {
                nextClip = 4;
                assert(false);      // untested?
            }
            setNextSection(nextClip);       
        }
    }
}

/****************************************** playback code ***********************************************/


bool MidiTrackPlayer::playOnce(double metricTime, float quantizeInterval) {
    PlayTracker tracker(playback.inPlayCode);
#if defined(_MLOG) && 1
    printf("MidiTrackPlayer::playOnce index=%d metrict=%.2f, quantizInt=%.2f track=%p\n",
           trackIndex, metricTime, quantizeInterval, track.get());
#endif

    // before other playback chores, see if there are any requests
    // we need to honor.
    serviceEventQueue();

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

void MidiTrackPlayer::serviceEventQueue()
{
    assert(playback.inPlayCode);
    if (eventQ.newSong) {
        setSongFromQueue(eventQ.newSong);
        eventQ.newSong.reset();
    } 
    if (eventQ.reset) {
        resetFromQueue(eventQ.resetSections);
        eventQ.reset = false;
        eventQ.resetSections = false;
    }  
    
}

void MidiTrackPlayer::resetFromQueue(bool resetSectionIndex) {
    assert(playback.inPlayCode);
    printf("for test ignoring new rest\n");
    resetSectionIndex = false;
    
    if (resetSectionIndex) {
        setupToPlayFirstTrackSection();
    } else {
        // we don't want reset to erase nextSectionIndex, so
        // re-apply it after reset.
        const int saveSection = eventQ.nextSectionIndex;
        if (saveSection == 0) {
            setupToPlayFirstTrackSection();
        } else {
            setNextSection(saveSection);
        }
    }


    curTrack = playback.song->getTrack(constTrackIndex, playback.curSectionIndex);
    if (curTrack) {
        // can we really handle not having a track?
        curEvent = curTrack->begin();
        //printf("reset put cur event back\n");
    }

    voiceAssigner.reset();
    currentLoopIterationStart = 0;
    auto options = playback.song->getOptions(constTrackIndex, playback.curSectionIndex);
    sectionLoopCounter = options ? options->repeatCount : 1;
    totalRepeatCount = sectionLoopCounter; 
    //printf("sectionLoopCounter set in rest %d\n", sectionLoopCounter);
}

void MidiTrackPlayer::setSongFromQueue(std::shared_ptr<MidiSong4> newSong)
{
    playback.song = newSong;

    curTrack = playback.song->getTrack(constTrackIndex);

    setupToPlayFirstTrackSection();
    auto options = playback.song->getOptions(constTrackIndex, playback.curSectionIndex);
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

void MidiTrackPlayer::onEndOfTrack() {
    assert(playback.inPlayCode);
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

void MidiTrackPlayer::setupToPlayFirstTrackSection() {
    assert(playback.inPlayCode);
    for (int i = 0; i < 4; ++i) {
        curTrack = playback.song->getTrack(constTrackIndex, i);
        if (curTrack && curTrack->getLength()) {
            playback.curSectionIndex = i;
            // printf("findFirstTrackSection found %d\n", curSectionIndex); fflush(stdout);
            return;
        }
    }
}

void MidiTrackPlayer::setupToPlayDifferentSection(int section) {
    assert(playback.inPlayCode);
    curTrack = nullptr;
    int nextSection = validateSectionRequest(section);
    playback.curSectionIndex = (nextSection == 0) ? 0 : nextSection - 1;
    // printf("setupToPlayDifferentSection next=%d\n", nextSection);
    // printf("setupToPlayDifferentSection set index to %d\n", curSectionIndex);
    setupToPlayCommon();
    // assert(false);
}

void MidiTrackPlayer::setupToPlayCommon() {
    assert(playback.inPlayCode);
    curTrack = playback.song->getTrack(constTrackIndex, playback.curSectionIndex);
    if (curTrack) {
        auto opts = playback.song->getOptions(constTrackIndex, playback.curSectionIndex);
        assert(opts);
        if (opts) {
            sectionLoopCounter = opts->repeatCount;
            // printf("in setup common, get sectionLoopCounter from options %d (tk=%d, sec=%d)\n", sectionLoopCounter, constTrackIndex, curSectionIndex);
        } else {
            sectionLoopCounter = 1;
            // printf("in setup common, get sectionLoopCounter from defaults %d\n", sectionLoopCounter);
        }
    }
    totalRepeatCount = sectionLoopCounter;
}

void MidiTrackPlayer::setupToPlayNextSection() {
    assert(playback.inPlayCode);
    curTrack = nullptr;
    MidiTrackPtr tk = nullptr;
    while (!tk) {
        if (++playback.curSectionIndex > 3) {
            playback.curSectionIndex = 0;
        }
        // printf("setupToPlayNExt set curSectionIndex to %d\n", curSectionIndex);
        tk = playback.song->getTrack(constTrackIndex, playback.curSectionIndex);
    }
    setupToPlayCommon();
}


bool MidiTrackPlayer::pollForNoteOff(double metricTime) {
    assert(playback.inPlayCode);
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

