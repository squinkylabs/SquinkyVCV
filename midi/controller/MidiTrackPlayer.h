#pragma once

#include "GateTrigger.h"
#include "MidiTrack.h"
#include "MidiVoice.h"
#include "MidiVoiceAssigner.h"

#include <memory>

class IMidiPlayerHost4;
class MidiSong4;
class MidiTrack;

// #define _MLOG

/**
 * This awful hack is so that both the real plugin and
 * the unit tests can pass this "Output" struct around
 */
#ifdef __PLUGIN
namespace rack {
namespace engine {
struct Input;
struct Param;
}
}  // namespace rack
#else
#include "TestComposite.h"
#endif

/**
 * input port usage
 * 
 * 0 = gate: goto next section
 * 1 = gate: goto prev section
 * 2 = cv: set section number
 * 3 = transpose
 * 4 =play clip 2x/3x/4x... faster (CV)
 */

class MidiTrackPlayer {
public:
#ifdef __PLUGIN
    using Param = rack::engine::Param;
    using Input = rack::engine::Input;
#else
    using Input = ::Input;
    using Param = ::Param;
#endif

    MidiTrackPlayer(std::shared_ptr<IMidiPlayerHost4> host, int trackIndex, std::shared_ptr<MidiSong4> song);
    void setSong(std::shared_ptr<MidiSong4> newSong, int trackIndex);
    void resetAllVoices(bool clearGates);

    /**
     * play the next event, if possible.
     * return true if event played.
     */
    bool playOnce(double metricTime, float quantizeInterval);

    void reset(bool resetSectionIndex);
    void setNumVoices(int numVoices);
    void setSampleCountForRetrigger(int);
    void updateSampleCount(int numElapsed);
    std::shared_ptr<MidiSong4> getSong();

    /**
     * For all these API, the section numbers are 1..4
     * for "next section" that totally makes sense, as 0 means "no request".
     * for getSection() I don't know what it's that way...
     */
    int getSection() const;
    void setNextSection(int section);

    /**
     * @param section is a new requested section (0, 1..4)
     * @returns valid section request (0,1..4) 
     *      If section exists, will return section
     *      otherwise will search forward for one to play.
     *      Will return 0 if there are no playable sections.
     */
    int validateSectionRequest(int section) const;
    int getNextSection() const;
    void setRunningStatus(bool running) {
        isPlaying = running;
    }

    void setPorts(Input* cvInput, Param* triggerImmediate) {
        input = cvInput;
        immediateParam = triggerImmediate;
    }

    /**
     * Returns the count in counting up the repeats.
     */
    int getCurrentRepetition();

private:
    std::shared_ptr<MidiSong4> song;
    std::shared_ptr<MidiTrack> curTrack;  // need something like array for song4??
    const int trackIndex = 0;

    /**
     * cur section index is 0..3, and is the direct index into the
     * song4 sections array.
     */
    int curSectionIndex = 0;

    /**
     * Variables around voice state
     */
    int numVoices = 1;  // up to 16
    static const int maxVoices = 16;
    MidiVoice voices[maxVoices];
    MidiVoiceAssigner voiceAssigner;

    Input* input = nullptr;
    Param* immediateParam = nullptr;
    GateTrigger nextSectionTrigger;
    GateTrigger prevSectionTrigger;

    /***********************************************
     * variables for playing a track
     */

    /**
     * abs metric time of start of current section's current loop
     */
    double currentLoopIterationStart = 0;
    MidiTrack::const_iterator curEvent;

    /**
     * This counter counts down. when if gets to zero
     * the section is done.
     */
    int sectionLoopCounter = 1;
    int totalRepeatCount = 1;  // what repeat was set to at the start of the section
    bool isPlaying = false;    // somtimes we need to know if we are playing

    bool pollForNoteOff(double metricTime);
    void findFirstTrackSection();

    /**
     * will set curSectionIndex, and sectionLoopCounter
     * to play the next valid section after curSectionIndex
     */
    void setupToPlayNextSection();

    /**
     * As above, will set CurSelectionIndex, and sectionLoopCounter.
     * @param section is the section + 1 we wish to go to.
     */
    void setupToPlayDifferentSection(int section);
    void setupToPlayCommon();
    void onEndOfTrack();
    void pollForCVChange();

    /**
     * This is not an event queue at all.
     * It's a collection of flags and vaues that are queued up.
     */
    class EventQ {
    public:
        /**
         * next section index is different. it is 1..4, where 
         * 0 means "no request". APIs to get and set this
         * use the save 1..4 offset index.
         */
        int nextSectionIndex = 0;

        /**
         * If false, we wait for next loop iteration to apply queue.
         * If true, we do "immediately";
         */
        bool eventsHappenImmediately = false;
    };

    EventQ eventQ;
    GateTrigger cv0Trigger;
    GateTrigger cv1Trigger;
};
