
class IMidiPlayerHost4;
class MidiSong4;

#include <memory>

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

};