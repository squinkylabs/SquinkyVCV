#pragma once
#include <memory>

#include "MidiTrack.h"

class MidiSong4;
using MidiSong4Ptr = std::shared_ptr<MidiSong4>;

class MidiSong4
{
public:
    static const int numTracks = 4;
    static const int numSectionsPerTrack = 4;

    void assertValid() const;

    // the make UT
    void addTrack(int trackIndex, int sectionIndex, MidiTrackPtr track);
    MidiTrackPtr getTrack(int trackIndex, int sectionIndex);
    static MidiSong4Ptr makeTest(MidiTrack::TestContent, int trackIndex, int sectionIndex);

private:
    std::shared_ptr<MidiLock> lock = std::make_shared<MidiLock>();
    MidiTrackPtr tracks[numTracks][numSectionsPerTrack] = {nullptr};
};

