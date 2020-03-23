
class IMidiPlayerHost4;
class MidiSong4;
class MidiTrackPlayer;

#include <memory>
#include <vector>

using MidiTrackPlayerPtr = std::shared_ptr<MidiTrackPlayer>;
using MidiSong4Ptr = std::shared_ptr<MidiSong4>;

#include "MidiTrackPlayer.h"

// #define _MLOG

class MidiPlayer4
{
public:
    using Input = MidiTrackPlayer::Input;
    using Param = MidiTrackPlayer::Param;
    MidiPlayer4(std::shared_ptr<IMidiPlayerHost4> host, std::shared_ptr<MidiSong4> song);

    void setSong(std::shared_ptr<MidiSong4> song);
    MidiSong4Ptr getSong();

    /**
     * Main "play something" function.
     * @param metricTime is the current time where 1 = quarter note.
     * @param quantizationInterval is the amount of metric time in a clock. 
     * So, if the click is a sixteenth note clock, quantizationInterval will be .25
     */
    void updateToMetricTime(double metricTime, float quantizationInterval, bool running);

    /** 
     * Called on the auto thread over and over.
     * Gives us a chance to do some work before updateToMetricTime gets called again.
     */
    void step();

    void setNumVoices(int track, int numVoices);
    void setSampleCountForRetrigger(int);
    void updateSampleCount(int numElapsed);

    /**
     * resets all internal playback state.
     * @param clearGate will set the host's gate low, if true
     */
    void reset(bool clearGates, bool resetSectionIndex);

    void setRunningStatus(bool running);

    int getSection(int track) const;
    void setNextSectionRequest(int track, int section);
    int getNextSectionRequest(int track) const;

    void setPorts(Input* cvInput, Param* triggerImmediate);
    
    /**
     * Provide direct access so we don't have to add a zillion
     * "pass thru" APIs.
     */
    MidiTrackPlayerPtr getTrackPlayer(int track);
private:
    std::vector<MidiTrackPlayerPtr> trackPlayers;
    MidiSong4Ptr song;
    std::shared_ptr<IMidiPlayerHost4> host;

    /**
     * when starting, or when reset by lock contention
     */
    bool isReset = true;
    bool isResetGates = false;
    bool isResetSectionIndex = false;

    void updateToMetricTimeInternal(double, float);
    void resetAllVoices(bool clearGates);

};