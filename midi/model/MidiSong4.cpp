
#include "MidiLock.h"
#include "MidiSong4.h"
#include "MidiTrack4Options.h"

void MidiSong4::assertValid()
{
    for (int track=0; track<numTracks; ++track) {
        for (int sect=0; sect<numSectionsPerTrack; ++sect) {
            bool hasTrack = !!this->getTrack(track, sect);
            bool hasOpt = !!this->getOptions(track, sect);
            assert(hasTrack == hasOpt);
            if (hasTrack) {
                auto tk = this->getTrack(track, sect);
                tk->assertValid();
            }
        }
    }
}

void MidiSong4::addTrack(int trackIndex, int sectionIndex,  MidiTrackPtr track)
{
    if (trackIndex < 0 || trackIndex >= numTracks || sectionIndex < 0 || sectionIndex >= numSectionsPerTrack) {
        assert(false);
        return;
    }
    // This is pretty hacky - you need to set options after the track
    
    tracks[trackIndex][sectionIndex] = track;
    options[trackIndex][sectionIndex] = std::make_shared<MidiTrack4Options>();
}

void MidiSong4::addOptions(int trackIndex, int sectionIndex,  MidiTrack4OptionsPtr ops)
{
    if (trackIndex < 0 || trackIndex >= numTracks || sectionIndex < 0 || sectionIndex >= numSectionsPerTrack) {
        assert(false);
        return;
    }
   
    options[trackIndex][sectionIndex] = ops;
}

MidiTrackPtr MidiSong4::getTrack(int trackIndex, int sectionIndex)
{
     if (trackIndex < 0 || trackIndex >= numTracks || sectionIndex < 0 || sectionIndex >= numSectionsPerTrack) {
        assert(false);
        return nullptr;
    }
    return tracks[trackIndex][sectionIndex];
}

MidiTrack4OptionsPtr MidiSong4::getOptions(int trackIndex, int sectionIndex)
{
     if (trackIndex < 0 || trackIndex >= numTracks || sectionIndex < 0 || sectionIndex >= numSectionsPerTrack) {
        assert(false);
        return nullptr;
    }
    return options[trackIndex][sectionIndex];
}

 MidiSong4Ptr MidiSong4::makeTest(MidiTrack::TestContent content, int trackIndex, int sectionIndex)
 {
    MidiSong4Ptr song = std::make_shared<MidiSong4>();
    MidiLocker l(song->lock);
    auto track = MidiTrack::makeTest(content, song->lock);
    song->addTrack(trackIndex, sectionIndex, track);
    song->assertValid();
    return song;
 }

 float MidiSong4::getTrackLength(int trackIndex) const
 {
     if (trackIndex < 0 || trackIndex >= numTracks ) {
        assert(false);
        return 0;
    }
    float ret = 0;
    for (auto section : tracks[trackIndex]) {
        if (section) {
            ret += section->getLength();
        }
    }
    return ret;
 }

void MidiSong4::createTrack(int index, int sectionIndex)
{
    assert(lock);
    addTrack(index, sectionIndex, std::make_shared<MidiTrack>(lock));
}

void MidiSong4::_flipTracks()
{
    std::swap(tracks[0], tracks[1]);
    std::swap(options[0], options[1]);
    
}

void MidiSong4::_flipSections()
{
    std::swap(tracks[0][0], tracks[0][1]);
    std::swap(options[0][0], options[0][1]);
}

void MidiSong4::_dump()
{
    printf("song4:\n");
    for (int tk = 0; tk < numTracks; ++tk) {
        for (int sec=0; sec < numSectionsPerTrack; ++sec) {
            auto track = getTrack(tk, sec);
            if (track) {
                printf("track %d, section %d:\n", tk, sec);
                track->_dump();
            }
        }
    }
    fflush(stdout);
}