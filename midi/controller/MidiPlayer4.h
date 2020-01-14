
class IMidiPlayerHost4;
class MidiSong4;
class MidiTrackPlayer;

#include <memory>
#include <vector>

using MidiTrackPlayerPtr = std::shared_ptr<MidiTrackPlayer>;
using MidiSong4Ptr = std::shared_ptr<MidiSong4>;

// #define _MLOG

class MidiPlayer4
{
public:
    MidiPlayer4(std::shared_ptr<IMidiPlayerHost4> host, std::shared_ptr<MidiSong4> song);

    void setSong(std::shared_ptr<MidiSong4> song);

         /**
     * Main "play something" function.
     * @param metricTime is the current time where 1 = quarter note.
     * @param quantizationInterval is the amount of metric time in a clock. 
     * So, if the click is a sixteenth note clock, quantizationInterval will be .25
     */
    void updateToMetricTime(double metricTime, float quantizationInterval, bool running);

    /**
     * loops are independent for each track. Default parameter is only 
     * provided for compatibilty with old unit tests.
     */
    double getCurrentLoopIterationStart(int track = 0) const;

    void setNumVoices(int);
    void setSampleCountForRetrigger(int);
    void updateSampleCount(int numElapsed);

    /**
     * resets all internal playback state.
     * @param clearGate will set the host's gate low, if true
     */
    void reset(bool clearGates);
private:
    std::vector<MidiTrackPlayerPtr> trackPlayers;
    MidiSong4Ptr song;
    std::shared_ptr<IMidiPlayerHost4> host;

    /**
     * when starting, or when reset by lock contention
     */
    bool isReset = true;
    bool isResetGates = false;

    void updateToMetricTimeInternal(double, float);
    void resetAllVoices(bool clearGates);

};