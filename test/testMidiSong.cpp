#include "MidiLock.h"
#include "MidiSong.h"
#include "MidiTrack.h"
#include "SqClipboard.h"

#include "asserts.h"


static void test0()
{
    MidiSongPtr song = std::make_shared<MidiSong>();
    assertEQ(song->getHighestTrackNumber(), -1);
    song->assertValid();
}

static void test1()
{
    MidiSongPtr song = std::make_shared<MidiSong>();
    MidiLocker l(song->lock);
    song->createTrack(10);
    song->getTrack(10)->insertEnd(0);
    assertEQ(song->getHighestTrackNumber(), 10);
    song->assertValid();
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
    track->assertValid();
    song->assertValid();
}

static void testDefSong()
{
    // TODO: move to song::assertValid()
    MidiSongPtr song = MidiSong::makeTest(MidiTrack::TestContent::eightQNotes, 0);
    assertEQ(song->getHighestTrackNumber(), 0);         // there should be one track - 0
    auto track = song->getTrack(0);
    track->assertValid();

    int notes = 0;
    int events = 0;
    for (auto ev : *track) {
        ++events;
        auto notep = safe_cast<MidiNoteEvent>(ev.second);
        if (notep) {
            ++notes;
        }
    }
    assertEQ(notes, 8);
    assertEQ(events, 9);

    song->assertValid();
}


static void testClip1()
{
    auto p = SqClipboard::getTrackData();
    assert(!p);

    std::shared_ptr<MidiLock> lock = std::make_shared<MidiLock>();

    std::shared_ptr<SqClipboard::Track> t = std::make_shared< SqClipboard::Track>();
    t->track = std::make_shared<MidiTrack>(lock);
    SqClipboard::putTrackData(t);

    p = SqClipboard::getTrackData();
    assert(p);
}

void testMidiSong()
{
    test0();
    test1();
    testDefSong();

    testClip1();
    assertNoMidi();     // check for leaks
}