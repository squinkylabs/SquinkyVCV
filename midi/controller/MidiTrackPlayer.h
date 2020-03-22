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

    /** 
     * Called on the auto thread over and over.
     * Gives us a chance to do some work before playOnce gets called again.
     */
    void step();
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
    void setNextSectionRequest(int section);


    int getNextSectionRequest() const;
    void setRunningStatus(bool running);
    bool _getRunningStatus() const;

    void setPorts(Input* cvInput, Param* triggerImmediate) {
        input = cvInput;
        immediateParam = triggerImmediate;
    }

    /**
     * Returns the count in counting up the repeats.
     */
    int getCurrentRepetition();

private:
    
   
    std::shared_ptr<IMidiPlayerHost4> host;

    /**
     * Which outer "track" we are assigned to, 0..3.
     * Unchanging (hence the name).
     */
    const int constTrackIndex = 0;


    /**
     * Variables around voice state
     */
    int numVoices = 1;  // up to 16
    static const int maxVoices = 16;
    MidiVoice voices[maxVoices];
    MidiVoiceAssigner voiceAssigner;

    /**
     * VCV Input port for the CV input for track
     */
    Input* input = nullptr;
    
    /**
     * Schmidt triggers for various CV input channels
     */
    GateTrigger nextSectionTrigger;
    GateTrigger prevSectionTrigger;

    Param* immediateParam = nullptr;        // not imp yet

    /**
     * This is the song UI sets directly and uses for UI purposes.
     * Often it is the same as playback.song.
     */
    std::shared_ptr<MidiSong4> uiSong;

    /**
     * This counter counts down. when if gets to zero
     * the section is done.
     */
    int sectionLoopCounter = 1;
    int totalRepeatCount = 1;  // what repeat was set to at the start of the section

    /**
     * Sometimes we need to know if we are playing.
     * This started as an experiment - is it still used?
     */
    bool isPlaying = false;    

    bool pollForNoteOff(double metricTime);
    void setupToPlayFirstTrackSection();

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
     * returns true if clock was reset
     */
    bool serviceEventQueue();
    void setSongFromQueue(std::shared_ptr<MidiSong4>);

    /**
     * Based on current song and section,
     * set curTrack, curEvent, loop Counter, and reset clock
     */
    void setPlaybackTrackFromSongAndSection();
    void resetFromQueue(bool sectionIndex);
    /**
     * @param section is a new requested section (0, 1..4)
     * @returns valid section request (0,1..4) 
     *      If section exists, will return section
     *      otherwise will search forward for one to play.
     *      Will return 0 if there are no playable sections.
     */
    static int validateSectionRequest(int section, std::shared_ptr<MidiSong4> song, int trackNumber);

    void dumpCurEvent(const char*);

    /**
     * variables only used by playback code.
     * Other code not allowed to touch it.
     */
    class Playback {
    public:
        /**
         * cur section index is 0..3, and is the direct index into the
         * song4 sections array.
         * This variable should not be directly manipulated by UI.
         * It is typically set by playback code when a measure changes.
         * It is also set when we set the song, etc... but that's probably a mistake. We should probably 
         * only queue a change when we set song.
         */
        int curSectionIndex = 0;

        /**
         * Flag to tell when we are running in the context of playback code.
         */
        bool inPlayCode = false;

        /**
         * The song we are currently playing
         */
        std::shared_ptr<MidiSong4> song;

        /**
         * abs metric time of start of current section's current loop.
         * Do we still uses this? we've changed how looping works..
         */
        double currentLoopIterationStart = 0;

        std::shared_ptr<MidiTrack> curTrack;

        /**
         * event iterator that playback uses. Advances each
         * time an event is played from the track.
         * We also set it on set song, but maybe that should be queued also?
         */
        MidiTrack::const_iterator curEvent;
    };

    /**
     * This is not an event queue at all.
     * It's a collection of flags and values that are queued up.
     * things come in mostly from other plugins proc() calls,
     * but could come in from UI thread (if we are being sloppy)
     * 
     * Will be service by playback code
     */
    class EventQ {
    public:
        /**
         * next section index is different. it is 1..4, where 
         * 0 means "no request". APIs to get and set this
         * use the save 1..4 offset index.
         */
        int nextSectionIndex = 0;
        bool nextSectionIndexSetWhileStopped = false;

        /**
         * If false, we wait for next loop iteration to apply queue.
         * If true, we do "immediately"; (is this used?)
         */
        bool eventsHappenImmediately = false;

        /** When UI wants to set a new song, it gets queued here.
         */
        std::shared_ptr<MidiSong4> newSong;

        bool reset = false;
        bool resetSections = false;
        bool startupTriggered = false;
    };

    EventQ eventQ;
    Playback playback;
    GateTrigger cv0Trigger;
    GateTrigger cv1Trigger;
};
