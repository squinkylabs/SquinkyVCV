#include <assert.h>

#include "MidiSong.h"
#include "MidiTrack.h"


int MidiSong::getHighestTrackNumber() const
{
    int numTracks = int(tracks.size());
    return numTracks - 1;
}


void MidiSong::addTrack(int index, std::shared_ptr<MidiTrack> track)
{
    if (index >= (int) tracks.size()) {
        tracks.resize(index + 1);
    }
    assert(!tracks[index]);         // can only create at empty loc

    tracks[index] = track;
}

void MidiSong::createTrack(int index)
{
    addTrack(index, std::make_shared<MidiTrack>());
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

MidiSongPtr MidiSong::makeTest1()
{
    MidiSongPtr song = std::make_shared<MidiSong>();
    auto track = MidiTrack::makeTest1();
    song->addTrack(0, track);
    return song;
}

void MidiSong::assertValid() const
{
    for (auto track : tracks) {
        if (track) {
            track->assertValid();
        }
    }
}