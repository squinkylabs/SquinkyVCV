#include <assert.h>

#include "MidiSong.h"
#include "MidiTrack.h"


void MidiSong::createTrack(int index)
{
    if (index >= (int)tracks.size()) {
        tracks.resize(index + 1);
    }
    assert(!tracks[index]);         // can only create at empty loc

    tracks[index] = std::make_shared<MidiTrack>();
}

MidiTrackPtr MidiSong::getTrack(int index)
{
    assert(index < (int) tracks.size());
    assert(index >= 0);
    assert(tracks[index]);
    return tracks[index];
}


MidiTrackConstPtr MidiSong::getTrack(int index) const
{
    assert(index < (int) tracks.size());
    assert(index >= 0);
    assert(tracks[index]);
    return tracks[index];
}