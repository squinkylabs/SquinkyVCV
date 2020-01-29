#pragma once
#include <memory>

#include "MidiLock.h"
#include "MidiTrack.h"

class MidiSong4;
class MidiTrack4Options;
using MidiSong4Ptr = std::shared_ptr<MidiSong4>;
using MidiTrack4OptionsPtr = std::shared_ptr<MidiTrack4Options>;



class MidiSong4
{
public:
    static const int numTracks = 4;
    static const int numSectionsPerTrack = 4;

    void assertValid();
    float getTrackLength(int trackNum) const;

    /**
     * The last argument is optional for template compatibility with tests.
     * In the future we should get rid of this optional stuff.
     */
    void createTrack(int index, int sectionIndex = 0);
    void addTrack(int trackIndex, int sectionIndex, MidiTrackPtr track);
    MidiTrackPtr getTrack(int trackIndex, int sectionIndex = 0);
    MidiTrack4OptionsPtr getOptions(int trackIndex, int sectionIndex);
    void addOptions(int trackIndex, int sectionIndex, MidiTrack4OptionsPtr options);

    /**
     * The last argument is optional for template compatibility with tests/
     */
    static MidiSong4Ptr makeTest(MidiTrack::TestContent, int trackIndex, int sectionIndex = 0);

    std::shared_ptr<MidiLock> lock = std::make_shared<MidiLock>();

    void _flipTracks();
    void _flipSections();
    void _dump();
private:
    
    MidiTrackPtr tracks[numTracks][numSectionsPerTrack] = {{nullptr}};
    MidiTrack4OptionsPtr options[numTracks][numSectionsPerTrack] = {{nullptr}};
};

