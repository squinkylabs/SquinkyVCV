#include "MidiLock.h"
#include "MidiSong.h"
#include "MidiTrack.h"

#include <assert.h>

MidiSong::MidiSong() : lock(std::make_shared<MidiLock>())
{
    ++_mdb;
}
MidiSong::~MidiSong()
{
    --_mdb;
}

int MidiSong::getHighestTrackNumber() const
{
    int numTracks = int(tracks.size());
    return numTracks - 1;
}

bool MidiSong::trackExists(int tkNum) const
{
    if (tkNum >= int(tracks.size())) {
        return false;
    }

    return bool(tracks[tkNum]);
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
    assert(lock);
    addTrack(index, std::make_shared<MidiTrack>(lock));
}


MidiTrackPtr MidiSong::getTrack(int index)
{
    assert(index < (int) tracks.size());
    assert(index >= 0);
    assert(tracks[index]);
    return tracks[index];
}


MidiTrackConstPtr MidiSong::getTrackConst(int index) const
{
    assert(index < (int) tracks.size());
    assert(index >= 0);
    assert(tracks[index]);
    return tracks[index];
}


MidiSongPtr MidiSong::makeTest(MidiTrack::TestContent content, int trackNumber)
{
    MidiSongPtr song = std::make_shared<MidiSong>();
    MidiLocker l(song->lock);
    auto track = MidiTrack::makeTest(content, song->lock);
    song->addTrack(trackNumber, track);
    song->assertValid();
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