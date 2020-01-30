#include "InteropClipboard.h"
#include "MidiLock.h"
#include "MidiSong.h"
#include "MidiSong4.h"
#include "MidiTrack.h"
#include "SqClipboard.h"
#include "TimeUtils.h"

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
#ifdef _OLDCLIP
    auto p = SqClipboard::getTrackData();
    assert(!p);

    std::shared_ptr<MidiLock> lock = std::make_shared<MidiLock>();

    std::shared_ptr<SqClipboard::Track> t = std::make_shared< SqClipboard::Track>();
    t->track = std::make_shared<MidiTrack>(lock);
    SqClipboard::putTrackData(t);

    p = SqClipboard::getTrackData();
    assert(p);
#else
    auto p = InteropClipboard::get();
    assert(!p);

    std::shared_ptr<MidiLock> lock = std::make_shared<MidiLock>();

   // std::shared_ptr<SqClipboard::Track> t = std::make_shared< SqClipboard::Track>();
   //t->track = std::make_shared<MidiTrack>(lock);

   MidiTrackPtr t = std::make_shared< MidiTrack>(lock);
    
    InteropClipboard::put(t);

    p = InteropClipboard::get();
    assert(p);
#endif
}

static void testSong4_1()
{
    MidiSong4Ptr song = std::make_shared<MidiSong4>();
    assertEQ(MidiSong4::numTracks, 4);
    assertEQ(MidiSong4::numSectionsPerTrack, 4);

    for (int tk = 0; tk < MidiSong4::numTracks; ++tk) {
        assertEQ(song->getTrackLength(tk), 0.f);
        for (int sect = 0; sect < MidiSong4::numSectionsPerTrack; ++sect) {
            assert(song->getTrack(tk, sect) == nullptr);
        }
    }
}

static void testSong4_2()
{
    MidiSong4Ptr song = MidiSong4::makeTest(MidiTrack::TestContent::empty, 2, 3);
    assertEQ(MidiSong4::numTracks, 4);
    assertEQ(MidiSong4::numSectionsPerTrack, 4);
    assert(song->getTrack(2, 3) != nullptr);
    assertEQ(song->getTrackLength(2), TimeUtils::quarterNote() * 8);
}

void testMidiSong()
{
    test0();
    test1();
    testDefSong();

    testClip1();

    testSong4_1();
    testSong4_2();
    assertNoMidi();     // check for leaks
}