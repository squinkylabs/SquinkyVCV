
class IMidiPlayerHost4;
class MidiSong4;
class MidiTrackPlayer;

#include <memory>
#include <vector>

using MidiTrackPlayerPtr = std::shared_ptr<MidiTrackPlayer>;
using MidiSong4Ptr = std::shared_ptr<MidiSong4>;

class MidiPlayer4
{
public:
     MidiPlayer4(std::shared_ptr<IMidiPlayerHost4> host, std::shared_ptr<MidiSong4> song);

         /**
     * Main "play something" function.
     * @param metricTime is the current time where 1 = quarter note.
     * @param quantizationInterval is the amount of metric time in a clock. 
     * So, if the click is a sixteenth note clock, quantizationInterval will be .25
     */
    void updateToMetricTime(double metricTime, float quantizationInterval, bool running);

    double getCurrentLoopIterationStart() const;

    /**
     * resets all internal playback state.
     * @param clearGate will set the host's gate low, if true
     */
    void reset(bool clearGates);
private:
    std::vector<MidiTrackPlayerPtr> trackPlayers;
    MidiSong4Ptr song;
    std::shared_ptr<IMidiPlayerHost4> host;

    void updateToMetricTimeInternal(double, float);

};