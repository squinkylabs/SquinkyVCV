
#include "MidiPlayer4.h"
#include "MidiSong4.h"
#include "TestHost4.h"

#include "asserts.h"

static MidiSong4Ptr makeSong(int trackNum)
{
    MidiSong4Ptr song = std::make_shared<MidiSong4>();
    MidiLocker lock(song->lock);
    MidiTrackPtr clip0 = MidiTrack::makeTest(MidiTrack::TestContent::oneQ1, song->lock);
    MidiTrackPtr clip1 = MidiTrack::makeTest(MidiTrack::TestContent::FourAlmostTouchingQuarters, song->lock);
    song->addTrack(trackNum, 0, clip0);
    song->addTrack(trackNum, 1, clip1);
    return song;
}

static void testTwoSections(int trackNum)
{
    MidiSong4Ptr song = makeSong(trackNum);
    std::shared_ptr<TestHost4> host = std::make_shared<TestHost4>();
    MidiPlayer4 pl(host, song);

    // before note, nothing
    const float quantizationInterval = .1f;       
    pl.updateToMetricTime(.8, quantizationInterval, true);
   // host->assertOneActiveTrack(0);
    assertEQ(host->gateChangeCount, 0)

    // after note, 1
    pl.updateToMetricTime(1.1, quantizationInterval, true);
    assertEQ(host->gateChangeCount, 1);

    assert(false);  //now next section
}

void testMidiPlayer4()
{
    testTwoSections(0);
}