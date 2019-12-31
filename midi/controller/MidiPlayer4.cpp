#include "MidiPlayer4.h"
#include "MidiSong4.h"
#include "MidiTrackPlayer.h"

MidiPlayer4::MidiPlayer4(std::shared_ptr<IMidiPlayerHost4> host, std::shared_ptr<MidiSong4> song)
{
//MidiTrackPlayerPtr
    for (int i = 0; i<MidiSong4::numTracks; ++i) {
        trackPlayers.push_back( std::make_shared<MidiTrackPlayer>());
    }
}

void MidiPlayer4::updateToMetricTime(double metricTime, float quantizationInterval, bool running)
{

}

double MidiPlayer4::getCurrentLoopIterationStart() const
{
    return 0;
}