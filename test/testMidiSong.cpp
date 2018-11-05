
#include "MidiSong.h"
#include "MidiTrack.h"

#include "asserts.h"


static void test0()
{
    MidiSongPtr song = std::make_shared<MidiSong>();
    assertEQ(song->getHighestTrackNumber(), -1);
}

static void test1()
{
    MidiSongPtr song = std::make_shared<MidiSong>();
    song->createTrack(10);
    assertEQ(song->getHighestTrackNumber(), 10);
}

static void test2()
{
    MidiSongPtr song = std::make_shared<MidiSong>();
    song->createTrack(0);
    assertEQ(song->getHighestTrackNumber(), 0);

    auto track = song->getTrack(0);
    assert(track);

    for (auto ev : *track) {
        assert(false);              // there should be no events in the track
    }
    assert(track->isValid());
}

static void testDefSong()
{
    MidiSongPtr song = MidiSong::makeTest1();
    assertEQ(song->getHighestTrackNumber(), 0);         // there should be one track - 0
    auto track = song->getTrack(0);
    assert(track->isValid());

    int notes = 0;
    for (auto ev : *track) {
        ++notes;
    }
    assertEQ(notes, 4);
}

void testMidiSong()
{
    test0();
    test1();
    testDefSong();
}